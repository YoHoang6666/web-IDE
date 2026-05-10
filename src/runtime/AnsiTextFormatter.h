#pragma once

#include <QColor>
#include <QFont>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextEdit>

namespace webide {
class AnsiTextFormatter {
public:
    void append(QTextEdit* output, const QString& text, bool isError) {
        if (!output || text.isEmpty()) {
            return;
        }

        AnsiState& state = isError ? stderrState_ : stdoutState_;
        AnsiState current = state;
        QString buffer;

        auto flushBuffer = [&](const QString& segment) {
            if (segment.isEmpty()) {
                return;
            }
            QTextCursor cursor = output->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.setCharFormat(formatFor(current));
            cursor.insertText(segment);
            output->setTextCursor(cursor);
        };

        for (int i = 0; i < text.size(); ++i) {
            if (text.at(i) == QLatin1Char('\x1b') && i + 1 < text.size() && text.at(i + 1) == QLatin1Char('[')) {
                flushBuffer(buffer);
                buffer.clear();
                const int end = text.indexOf(QLatin1Char('m'), i + 2);
                if (end == -1) {
                    continue;
                }
                const QString codesSection = text.mid(i + 2, end - (i + 2));
                const QStringList codes = codesSection.split(QLatin1Char(';'), Qt::SkipEmptyParts);
                if (codes.isEmpty()) {
                    applyCode(current, 0);
                } else {
                    for (const QString& code : codes) {
                        applyCode(current, code.toInt());
                    }
                }
                i = end;
            } else {
                buffer.append(text.at(i));
            }
        }
        flushBuffer(buffer);
        state = current;
        output->ensureCursorVisible();
    }

private:
    struct AnsiState {
        QColor foreground;
        QColor background;
        bool bold = false;
        bool hasForeground = false;
        bool hasBackground = false;
    };

    static QColor colorForCode(int code) {
        static const QColor baseColors[] = {
            QColor(Qt::black),
            QColor(Qt::red),
            QColor(Qt::green),
            QColor(Qt::yellow),
            QColor(Qt::blue),
            QColor(Qt::magenta),
            QColor(Qt::cyan),
            QColor(Qt::lightGray),
        };

        static const QColor brightColors[] = {
            QColor(Qt::darkGray),
            QColor(255, 85, 85),
            QColor(85, 255, 85),
            QColor(255, 255, 85),
            QColor(85, 85, 255),
            QColor(255, 85, 255),
            QColor(85, 255, 255),
            QColor(Qt::white),
        };

        if (code >= 30 && code <= 37) {
            return baseColors[code - 30];
        }
        if (code >= 90 && code <= 97) {
            return brightColors[code - 90];
        }
        if (code >= 40 && code <= 47) {
            return baseColors[code - 40];
        }
        if (code >= 100 && code <= 107) {
            return brightColors[code - 100];
        }
        return QColor();
    }

    static QTextCharFormat formatFor(const AnsiState& state) {
        QTextCharFormat format;
        if (state.hasForeground) {
            format.setForeground(state.foreground);
        }
        if (state.hasBackground) {
            format.setBackground(state.background);
        }
        if (state.bold) {
            format.setFontWeight(QFont::Bold);
        }
        return format;
    }

    static void applyCode(AnsiState& state, int code) {
        if (code == 0) {
            state = {};
            return;
        }
        if (code == 1) {
            state.bold = true;
            return;
        }
        if (code == 39) {
            state.hasForeground = false;
            return;
        }
        if (code == 49) {
            state.hasBackground = false;
            return;
        }

        if ((code >= 30 && code <= 37) || (code >= 90 && code <= 97)) {
            state.foreground = colorForCode(code);
            state.hasForeground = true;
            return;
        }
        if ((code >= 40 && code <= 47) || (code >= 100 && code <= 107)) {
            state.background = colorForCode(code);
            state.hasBackground = true;
        }
    }

    AnsiState stdoutState_;
    AnsiState stderrState_;
};
}  // namespace webide
