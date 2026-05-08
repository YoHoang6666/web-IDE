#pragma once

#include <QHash>
#include <QWidget>

class QPlainTextEdit;
class QSplitter;
class QTabWidget;

namespace webide {
class EditorAreaWidget : public QWidget {
    Q_OBJECT

public:
    explicit EditorAreaWidget(QWidget* parent = nullptr);

    bool openFile(const QString& filePath);
    bool saveCurrent();
    bool saveCurrentAs();
    void closeCurrentTab();
    void setSplitEditorVisible(bool visible);
    bool isSplitEditorVisible() const;

    QString currentFilePath() const;

public slots:
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();

signals:
    void currentFileChanged(const QString& filePath, const QString& content, bool isHtml);
    void documentContentChanged(const QString& filePath, const QString& content, bool isHtml);
    void statusMessage(const QString& message);

private:
    struct EditorTabInfo {
        QString filePath;
        bool untitled = false;
    };

    void setupUi();
    QPlainTextEdit* createEditor();
    QTabWidget* tabWidgetForNewFile() const;
    QPlainTextEdit* currentEditor() const;
    void setActiveTabWidget(QTabWidget* tabs);
    void updateTabTitle(QPlainTextEdit* editor);
    QString tabTitleFor(QPlainTextEdit* editor) const;
    bool saveEditor(QPlainTextEdit* editor, bool forceSaveAs);
    int indexOfEditor(QTabWidget* tabs, QPlainTextEdit* editor) const;
    bool closeEditorAt(QTabWidget* tabs, int index);
    bool emitCurrentFileState();

    QSplitter* splitter_;
    QTabWidget* primaryTabs_;
    QTabWidget* secondaryTabs_;
    QTabWidget* activeTabs_;

    QHash<QPlainTextEdit*, EditorTabInfo> tabInfo_;
    int untitledCounter_ = 1;
};
}  // namespace webide
