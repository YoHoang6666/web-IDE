#include "EditorAreaWidget.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QTabWidget>
#include <QTextDocument>
#include <QTextStream>
#include <QVBoxLayout>

namespace webide {
bool EditorAreaWidget::isHtmlFile(const QString& filePath) {
    return filePath.endsWith(".html", Qt::CaseInsensitive) || filePath.endsWith(".htm", Qt::CaseInsensitive);
}

EditorAreaWidget::EditorAreaWidget(QWidget* parent)
    : QWidget(parent),
      splitter_(new QSplitter(Qt::Horizontal, this)),
      primaryTabs_(new QTabWidget(splitter_)),
      secondaryTabs_(new QTabWidget(splitter_)),
      activeTabs_(primaryTabs_) {
    setupUi();
}

void EditorAreaWidget::setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    for (QTabWidget* tabs : {primaryTabs_, secondaryTabs_}) {
        tabs->setDocumentMode(true);
        tabs->setTabsClosable(true);
        tabs->setMovable(true);

        connect(tabs, &QTabWidget::currentChanged, this, [this, tabs](int) {
            setActiveTabWidget(tabs);
            emitCurrentFileState();
        });

        connect(tabs, &QTabWidget::tabCloseRequested, this, [this, tabs](int index) {
            closeEditorAt(tabs, index);
            emitCurrentFileState();
        });

        connect(tabs, &QTabWidget::tabBarClicked, this, [this, tabs](int) {
            setActiveTabWidget(tabs);
        });
    }

    splitter_->addWidget(primaryTabs_);
    splitter_->addWidget(secondaryTabs_);
    splitter_->setStretchFactor(0, 1);
    splitter_->setStretchFactor(1, 1);

    layout->addWidget(splitter_);
}

QPlainTextEdit* EditorAreaWidget::createEditor() {
    auto* editor = new QPlainTextEdit(this);
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);

    QFont font;
    font.setStyleHint(QFont::Monospace);
    font.setFamilies({QStringLiteral("Consolas"), QStringLiteral("Monospace"), QStringLiteral("JetBrains Mono")});
    font.setPointSize(11);
    editor->setFont(font);

    connect(editor->document(), &QTextDocument::modificationChanged, this, [this, editor](bool) {
        updateTabTitle(editor);
    });

    connect(editor, &QPlainTextEdit::textChanged, this, [this, editor]() {
        updateTabTitle(editor);
        const auto info = tabInfo_.value(editor);
        const bool isHtml = isHtmlFile(info.filePath);
        emit documentContentChanged(info.filePath, editor->toPlainText(), isHtml);
    });

    return editor;
}

QTabWidget* EditorAreaWidget::tabWidgetForNewFile() const {
    return (activeTabs_ && activeTabs_->isVisible()) ? activeTabs_ : primaryTabs_;
}

bool EditorAreaWidget::openFile(const QString& filePath) {
    if (filePath.isEmpty()) {
        return false;
    }

    for (QTabWidget* tabs : {primaryTabs_, secondaryTabs_}) {
        for (int i = 0; i < tabs->count(); ++i) {
            auto* editor = qobject_cast<QPlainTextEdit*>(tabs->widget(i));
            if (editor && tabInfo_.value(editor).filePath == filePath) {
                tabs->setCurrentIndex(i);
                setActiveTabWidget(tabs);
                emitCurrentFileState();
                return true;
            }
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit statusMessage(tr("Failed to open file: %1").arg(filePath));
        return false;
    }

    QTextStream stream(&file);
    const QString content = stream.readAll();

    auto* editor = createEditor();
    editor->setPlainText(content);
    editor->document()->setModified(false);

    EditorTabInfo info;
    info.filePath = filePath;
    info.untitled = false;
    tabInfo_.insert(editor, info);

    QTabWidget* tabs = tabWidgetForNewFile();
    const int index = tabs->addTab(editor, QFileInfo(filePath).fileName());
    tabs->setCurrentIndex(index);
    setActiveTabWidget(tabs);

    emit statusMessage(tr("Opened: %1").arg(filePath));
    emitCurrentFileState();
    return true;
}

bool EditorAreaWidget::saveCurrent() { return saveEditor(currentEditor(), false); }

bool EditorAreaWidget::saveCurrentAs() { return saveEditor(currentEditor(), true); }

bool EditorAreaWidget::saveEditor(QPlainTextEdit* editor, bool forceSaveAs) {
    if (!editor) {
        return false;
    }

    auto info = tabInfo_.value(editor);
    QString filePath = info.filePath;

    if (forceSaveAs || filePath.isEmpty()) {
        filePath = QFileDialog::getSaveFileName(this, tr("Save File As"), filePath);
        if (filePath.isEmpty()) {
            return false;
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Save"), tr("Failed to save file: %1").arg(filePath));
        return false;
    }

    QTextStream stream(&file);
    stream << editor->toPlainText();
    file.close();

    info.filePath = filePath;
    info.untitled = false;
    tabInfo_[editor] = info;
    editor->document()->setModified(false);

    updateTabTitle(editor);
    emit statusMessage(tr("Saved: %1").arg(filePath));
    emitCurrentFileState();
    return true;
}

void EditorAreaWidget::closeCurrentTab() {
    QTabWidget* tabs = activeTabs_ ? activeTabs_ : primaryTabs_;
    closeEditorAt(tabs, tabs->currentIndex());
    emitCurrentFileState();
}

bool EditorAreaWidget::closeEditorAt(QTabWidget* tabs, int index) {
    if (!tabs || index < 0 || index >= tabs->count()) {
        return false;
    }

    auto* editor = qobject_cast<QPlainTextEdit*>(tabs->widget(index));
    if (!editor) {
        return false;
    }

    if (editor->document()->isModified()) {
        const auto answer = QMessageBox::question(this,
                                                  tr("Unsaved Changes"),
                                                  tr("Save changes to %1?").arg(tabTitleFor(editor)),
                                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                                  QMessageBox::Yes);
        if (answer == QMessageBox::Cancel) {
            return false;
        }
        if (answer == QMessageBox::Yes && !saveEditor(editor, false)) {
            return false;
        }
    }

    tabInfo_.remove(editor);
    tabs->removeTab(index);
    editor->deleteLater();
    return true;
}

void EditorAreaWidget::setSplitEditorVisible(bool visible) {
    secondaryTabs_->setVisible(visible);
    splitter_->setHandleWidth(visible ? 5 : 1);
}

bool EditorAreaWidget::isSplitEditorVisible() const { return secondaryTabs_->isVisible(); }

QString EditorAreaWidget::currentFilePath() const {
    auto* editor = currentEditor();
    if (!editor) {
        return {};
    }
    return tabInfo_.value(editor).filePath;
}

QPlainTextEdit* EditorAreaWidget::currentEditor() const {
    QTabWidget* tabs = activeTabs_ ? activeTabs_ : primaryTabs_;
    return tabs ? qobject_cast<QPlainTextEdit*>(tabs->currentWidget()) : nullptr;
}

void EditorAreaWidget::setActiveTabWidget(QTabWidget* tabs) {
    if (tabs) {
        activeTabs_ = tabs;
    }
}

void EditorAreaWidget::updateTabTitle(QPlainTextEdit* editor) {
    if (!editor) {
        return;
    }

    for (QTabWidget* tabs : {primaryTabs_, secondaryTabs_}) {
        const int index = indexOfEditor(tabs, editor);
        if (index >= 0) {
            tabs->setTabText(index, tabTitleFor(editor));
            break;
        }
    }
}

QString EditorAreaWidget::tabTitleFor(QPlainTextEdit* editor) const {
    const auto info = tabInfo_.value(editor);
    QString title = info.filePath.isEmpty() ? tr("Untitled") : QFileInfo(info.filePath).fileName();
    if (editor && editor->document()->isModified()) {
        title.prepend('*');
    }
    return title;
}

int EditorAreaWidget::indexOfEditor(QTabWidget* tabs, QPlainTextEdit* editor) const {
    if (!tabs || !editor) {
        return -1;
    }
    for (int i = 0; i < tabs->count(); ++i) {
        if (tabs->widget(i) == editor) {
            return i;
        }
    }
    return -1;
}

bool EditorAreaWidget::emitCurrentFileState() {
    auto* editor = currentEditor();
    if (!editor) {
        emit currentFileChanged({}, {}, false);
        return false;
    }

    const auto info = tabInfo_.value(editor);
    const bool isHtml = isHtmlFile(info.filePath);
    emit currentFileChanged(info.filePath, editor->toPlainText(), isHtml);
    return true;
}

void EditorAreaWidget::undo() {
    if (auto* editor = currentEditor()) {
        editor->undo();
    }
}

void EditorAreaWidget::redo() {
    if (auto* editor = currentEditor()) {
        editor->redo();
    }
}

void EditorAreaWidget::cut() {
    if (auto* editor = currentEditor()) {
        editor->cut();
    }
}

void EditorAreaWidget::copy() {
    if (auto* editor = currentEditor()) {
        editor->copy();
    }
}

void EditorAreaWidget::paste() {
    if (auto* editor = currentEditor()) {
        editor->paste();
    }
}
}  // namespace webide
