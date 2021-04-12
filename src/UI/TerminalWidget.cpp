/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <QtMath>
#include <QScrollBar>
#include <QApplication>
#include <Serial/Console.h>
#include <UI/TerminalWidget.h>

using namespace UI;

/*
 * NOTE: most of the Doxygen documentation comments where heavily based from the following
 *       URL https://doc.qt.io/qt-5/qplaintextedit.html. In some cases the comments are a
 *       simple copy-paste job. I am lazy and the Qt documentation is very good IMO.
 *
 *       This class works by initializing a QPlainTextEdit widget, rendering it into the
 *       the painter of a QML item and handling keyboard/mouse events.
 *
 *       The rest of the functions are just a wrapper around the functions of the
 *       QPlainTextEdit widget for increased QML-friendliness.
 *
 *       For more info, check this blog post:
 *       https://www.alex-spataru.com/blog/using-qt-widgets-in-your-qmlqtquick-applications
 */

/**
 * Constructor function
 */
TerminalWidget::TerminalWidget(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_autoscroll(true)
    , m_emulateVt100(false)
    , m_copyAvailable(false)
    , m_textEdit(new QPlainTextEdit)
    , m_terminalState(VT100_Text)
{
    // Set item flags
    setFlag(ItemHasContents, true);
    setFlag(ItemAcceptsInputMethod, true);
    setFlag(ItemIsFocusScope, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    // Configure text edit widget
    setScrollbarWidth(14);
    textEdit()->installEventFilter(this);

    // Set the QML item's implicit size
    auto hint = textEdit()->sizeHint();
    setImplicitSize(hint.width(), hint.height());

    // Setup default options
    textEdit()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit()->setSizeAdjustPolicy(QPlainTextEdit::AdjustToContents);

    // Get default text color
    m_color = qApp->palette().color(QPalette::Text);

    // Resize QPlainTextEdit to fit QML item
    connect(this, &QQuickPaintedItem::widthChanged, this,
            &TerminalWidget::updateWidgetSize);
    connect(this, &QQuickPaintedItem::heightChanged, this,
            &TerminalWidget::updateWidgetSize);

    // Connect console signals (doing this on QML uses about 50% of UI thread time)
    auto console = Serial::Console::getInstance();
    connect(console, &Serial::Console::stringReceived, this, &TerminalWidget::insertText);

    // React to widget events
    connect(textEdit(), SIGNAL(copyAvailable(bool)), this, SLOT(setCopyAvailable(bool)));
}

/**
 * Destructor function
 */
TerminalWidget::~TerminalWidget()
{
    m_textEdit->deleteLater();
}

/**
 * Handle application events manually
 */
bool TerminalWidget::event(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::FocusIn:
            forceActiveFocus();
            return QQuickPaintedItem::event(event);
            break;
        case QEvent::Wheel:
            processWheelEvents(static_cast<QWheelEvent *>(event));
            return true;
            break;
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
            processMouseEvents(static_cast<QMouseEvent *>(event));
            return true;
            break;
        default:
            break;
    }

    return QApplication::sendEvent(textEdit(), event);
}

/**
 * Render the text edit on the given @a painter
 */
void TerminalWidget::paint(QPainter *painter)
{
    if (m_textEdit && painter)
        textEdit()->render(painter);
}

/**
 * Custom event filter to manage redraw requests
 */
bool TerminalWidget::eventFilter(QObject *watched, QEvent *event)
{
    Q_ASSERT(m_textEdit);

    if (watched == textEdit())
    {
        switch (event->type())
        {
            case QEvent::Paint:
            case QEvent::UpdateRequest:
                update();
                break;
            default:
                break;
        }
    }

    return QQuickPaintedItem::eventFilter(watched, event);
}

/**
 * Returns the font used by the QPlainTextEdit widget
 */
QFont TerminalWidget::font() const
{
    return textEdit()->font();
}

/**
 * Returns the text color used by the QPlainTextEdit widget
 */
QColor TerminalWidget::color() const
{
    return m_color;
}

/**
 * Returns the plain text of the QPlainTextEdit widget
 */
QString TerminalWidget::text() const
{
    return textEdit()->toPlainText();
}

/**
 * Returns @c true if the text document is empty
 */
bool TerminalWidget::empty() const
{
    return textEdit()->document()->isEmpty();
}

/**
 * Returns @c true if the widget is set to read-only
 */
bool TerminalWidget::readOnly() const
{
    return textEdit()->isReadOnly();
}

/**
 * Returns @c true if the widget shall scroll automatically to the bottom when new
 * text is appended to the widget.
 */
bool TerminalWidget::autoscroll() const
{
    return m_autoscroll;
}

/**
 * Returns the palette used by the QPlainTextEdit widget
 */
QPalette TerminalWidget::palette() const
{
    return textEdit()->palette();
}

/**
 * Returns the wrap mode of the QPlainTextEdit casted to an integer type (so that it
 * can be used from the QML interface).
 */
int TerminalWidget::wordWrapMode() const
{
    return static_cast<int>(textEdit()->wordWrapMode());
}

/**
 * Returns the width of the vertical scrollbar
 */
int TerminalWidget::scrollbarWidth() const
{
    return textEdit()->verticalScrollBar()->width();
}

/**
 * Returns @c true if the user is able to copy any text from the document. This value is
 * updated through the copyAvailable() signal sent by the QPlainTextEdit.
 */
bool TerminalWidget::copyAvailable() const
{
    return m_copyAvailable;
}

/**
 * Returns @c true if the QPlainTextEdit widget is enabled
 */
bool TerminalWidget::widgetEnabled() const
{
    return textEdit()->isEnabled();
}

/**
 * If set to true, the plain text edit scrolls the document vertically to make the cursor
 * visible at the center of the viewport. This also allows the text edit to scroll below
 * the end of the document. Otherwise, if set to false, the plain text edit scrolls the
 * smallest amount possible to ensure the cursor is visible.
 */
bool TerminalWidget::centerOnScroll() const
{
    return textEdit()->centerOnScroll();
}

/**
 * Returns true if the control shall parse basic VT-100 escape secuences. This can be
 * useful if you need to interface with a shell/CLI from Serial Studio.
 */
bool TerminalWidget::vt100emulation() const
{
    return m_emulateVt100;
}

/**
 * This property holds whether undo and redo are enabled.
 * Users are only able to undo or redo actions if this property is true, and if there is
 * an action that can be undone (or redone).
 */
bool TerminalWidget::undoRedoEnabled() const
{
    return textEdit()->isUndoRedoEnabled();
}

/**
 * This property holds the limit for blocks in the document.
 *
 * Specifies the maximum number of blocks the document may have. If there are more blocks
 * in the document that specified with this property blocks are removed from the beginning
 * of the document.
 *
 * A negative or zero value specifies that the document may contain an unlimited amount
 * of blocks.
 */
int TerminalWidget::maximumBlockCount() const
{
    return textEdit()->maximumBlockCount();
}

/**
 * This property holds the editor placeholder text.
 *
 * Setting this property makes the editor display a grayed-out placeholder text as long as
 * the document is empty.
 */
QString TerminalWidget::placeholderText() const
{
    return textEdit()->placeholderText();
}

/**
 * Returns the pointer to the text document
 */
QTextDocument *TerminalWidget::document() const
{
    return textEdit()->document();
}

/**
 * Returns the pointer to the text edit object
 */
QPlainTextEdit *TerminalWidget::textEdit() const
{
    return m_textEdit;
}

/**
 * Copies any selected text to the clipboard.
 */
void TerminalWidget::copy()
{
    textEdit()->copy();
}

/**
 * Deletes all the text in the text edit.
 */
void TerminalWidget::clear()
{
    textEdit()->clear();
    updateScrollbarVisibility();
    update();

    emit textChanged();
}

/**
 * Selects all the text of the text edit.
 */
void TerminalWidget::selectAll()
{
    textEdit()->selectAll();
    update();
}

/**
 * Clears the text selection
 */
void TerminalWidget::clearSelection()
{
    auto cursor = QTextCursor(textEdit()->document());
    cursor.clearSelection();
    textEdit()->setTextCursor(cursor);
    updateScrollbarVisibility();
    update();
}

/**
 * Changes the read-only state of the text edit.
 *
 * In a read-only text edit the user can only navigate through the text and select text;
 * modifying the text is not possible.
 */
void TerminalWidget::setReadOnly(const bool ro)
{
    textEdit()->setReadOnly(ro);
    update();

    emit readOnlyChanged();
}

/**
 * Changes the font used to display the text of the text edit.
 */
void TerminalWidget::setFont(const QFont &font)
{
    textEdit()->setFont(font);
    updateScrollbarVisibility();
    update();

    emit fontChanged();
}

/**
 * Appends a new paragraph with text to the end of the text edit.
 *
 * If @c autoscroll() is enabled, this function shall also update the scrollbar position
 * to scroll to the bottom of the text.
 */
void TerminalWidget::append(const QString &text)
{
    textEdit()->appendPlainText(text);
    updateScrollbarVisibility();

    if (autoscroll())
        scrollToBottom();

    update();
    emit textChanged();
}

/**
 * Replaces the text of the text editor with @c text.
 *
 * If @c autoscroll() is enabled, this function shall also update the scrollbar position
 * to scroll to the bottom of the text.
 */
void TerminalWidget::setText(const QString &text)
{
    textEdit()->setPlainText(text);
    updateScrollbarVisibility();

    if (autoscroll())
        scrollToBottom();

    update();
    emit textChanged();
}

/**
 * Changes the text color of the text editor.
 */
void TerminalWidget::setColor(const QColor &color)
{
    m_color = color;
    auto qss = QString("QPlainTextEdit{color: %1;}").arg(color.name());
    textEdit()->setStyleSheet(qss);
    update();

    emit colorChanged();
}

/**
 * Changes the width of the vertical scrollbar
 */
void TerminalWidget::setScrollbarWidth(const int width)
{
    auto bar = textEdit()->verticalScrollBar();
    bar->setFixedWidth(width);
    update();

    emit scrollbarWidthChanged();
}

/**
 * Changes the @c QPalette of the text editor widget and its children.
 */
void TerminalWidget::setPalette(const QPalette &palette)
{
    textEdit()->setPalette(palette);
    update();

    emit paletteChanged();
}

/**
 * Enables or disables the text editor widget.
 */
void TerminalWidget::setWidgetEnabled(const bool enabled)
{
    textEdit()->setEnabled(enabled);
    update();

    emit widgetEnabledChanged();
}

/**
 * Enables/disable automatic scrolling. If automatic scrolling is enabled, then the
 * vertical scrollbar shall automatically scroll to the end of the document when the
 * text of the text editor is changed.
 */
void TerminalWidget::setAutoscroll(const bool enabled)
{
    // Change internal variables
    m_autoscroll = enabled;
    updateScrollbarVisibility();

    // Scroll to bottom if autoscroll is enabled
    if (enabled)
        scrollToBottom(true);

    // Update console configuration
    Serial::Console::getInstance()->setAutoscroll(enabled);

    // Update UI
    update();
    emit autoscrollChanged();
}

/**
 * Inserts the given @a text directly, no additional line breaks added.
 */
void TerminalWidget::insertText(const QString &text)
{
    addText(text, vt100emulation());
}

/**
 * Changes the word wrap mode of the text editor.
 *
 * This property holds the mode QPlainTextEdit will use when wrapping text by words.
 */
void TerminalWidget::setWordWrapMode(const int mode)
{
    textEdit()->setWordWrapMode(static_cast<QTextOption::WrapMode>(mode));
    updateScrollbarVisibility();
    update();

    emit wordWrapModeChanged();
}

/**
 * If set to true, the plain text edit scrolls the document vertically to make the cursor
 * visible at the center of the viewport. This also allows the text edit to scroll below
 * the end of the document. Otherwise, if set to false, the plain text edit scrolls the
 * smallest amount possible to ensure the cursor is visible.
 */
void TerminalWidget::setCenterOnScroll(const bool enabled)
{
    textEdit()->setCenterOnScroll(enabled);
    update();

    emit centerOnScrollChanged();
}

/**
 * Enables/disables interpretation of VT-100 escape secuences. This can be useful when
 * interfacing through network ports or interfacing with a MCU that implements some
 * kind of shell.
 */
void TerminalWidget::setVt100Emulation(const bool enabled)
{
    m_emulateVt100 = enabled;
    emit vt100EmulationChanged();
}

/**
 * Enables/disables undo/redo history support.
 */
void TerminalWidget::setUndoRedoEnabled(const bool enabled)
{
    textEdit()->setUndoRedoEnabled(enabled);
    update();

    emit undoRedoEnabledChanged();
}

/**
 * Changes the placeholder text of the text editor. The placeholder text is only displayed
 * when the document is empty.
 */
void TerminalWidget::setPlaceholderText(const QString &text)
{
    textEdit()->setPlaceholderText(text);
    update();

    emit placeholderTextChanged();
}

/**
 * Moves the position of the vertical scrollbar to the end of the document. However, this
 * function also ensures that the last line of the document is shown at the bottom of
 * the widget to mimic a terminal.
 */
void TerminalWidget::scrollToBottom(const bool repaint)
{
    // Get scrollbar pointer, calculate line count & visible text lines
    auto *bar = textEdit()->verticalScrollBar();
    auto lineCount = textEdit()->document()->blockCount();
    auto visibleLines = qFloor(height() / textEdit()->fontMetrics().height());

    // Abort operation if control is not visible
    if (visibleLines <= 0)
        return;

    // Update scrolling range
    bar->setMinimum(0);
    bar->setMaximum(lineCount + 1);

    // Do not scroll to bottom if all text fits in current window
    if (lineCount > visibleLines)
        bar->setValue(lineCount - visibleLines + 1);
    else
        bar->setValue(0);

    // Trigger UI repaint
    if (repaint)
        update();
}

/**
 * Specifies the maximum number of blocks the document may have. If there are more blocks
 * in the document that specified with this property blocks are removed from the beginning
 * of the document.
 *
 * A negative or zero value specifies that the document may contain an unlimited amount of
 * blocks.
 */
void TerminalWidget::setMaximumBlockCount(const int maxBlockCount)
{
    textEdit()->setMaximumBlockCount(maxBlockCount);
    update();

    emit maximumBlockCountChanged();
}

/**
 * Resizes the text editor widget to fit inside the QML item.
 */
void TerminalWidget::updateWidgetSize()
{
    textEdit()->setFixedSize(width(), height());
    updateScrollbarVisibility();
    update();
}

/**
 * Hides or shows the scrollbar
 */
void TerminalWidget::updateScrollbarVisibility()
{
    // Get current line count & visible lines
    auto lineCount = textEdit()->document()->blockCount();
    auto visibleLines = qCeil(height() / textEdit()->fontMetrics().height());

    // Autoscroll enabled or text content is less than widget height
    if (autoscroll() || visibleLines >= lineCount)
        textEdit()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Show the scrollbar if the text content is greater than the widget height
    else
        textEdit()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

/**
 * Updates the value of copy-available. This function is automatically called by the text
 * editor widget when the user makes any text selection/deselection.
 */
void TerminalWidget::setCopyAvailable(const bool yes)
{
    m_copyAvailable = yes;
    emit copyAvailableChanged();
}

/**
 * Inserts the given @a text directly, no additional line breaks added.
 */
void TerminalWidget::addText(const QString &text, const bool enableVt100)
{
    // Get text to insert
    QString textToInsert = text;
    if (enableVt100)
        textToInsert = vt100Processing(text);

    // Add text at the end of the text document
    QTextCursor cursor(textEdit()->document());
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(textToInsert);
    cursor.endEditBlock();

    // Autoscroll to bottom (if needed)
    updateScrollbarVisibility();
    if (autoscroll())
        scrollToBottom();

    // Redraw the control
    update();
    emit textChanged();
}

/**
 * Hack: call the appropiate protected mouse event handler function of the QPlainTextEdit
 *       item depending on event type
 */
void TerminalWidget::processMouseEvents(QMouseEvent *event)
{
    // Subclass QPlainTextEdit so that we can call protected functions
    class Hack : public QPlainTextEdit
    {
    public:
        using QPlainTextEdit::mouseDoubleClickEvent;
        using QPlainTextEdit::mouseMoveEvent;
        using QPlainTextEdit::mousePressEvent;
        using QPlainTextEdit::mouseReleaseEvent;
    };

    // Call appropiate function
    auto hack = static_cast<Hack *>(textEdit());
    switch (event->type())
    {
        case QEvent::MouseButtonPress:
            hack->mousePressEvent(event);
            break;
        case QEvent::MouseMove:
            hack->mouseMoveEvent(event);
            break;
        case QEvent::MouseButtonRelease:
            hack->mouseReleaseEvent(event);
            break;
        case QEvent::MouseButtonDblClick:
            hack->mouseDoubleClickEvent(event);
            break;
        default:
            break;
    }
}

/**
 * Hack: call the protected wheel event handler function of the QPlainTextEdit item
 */
void TerminalWidget::processWheelEvents(QWheelEvent *event)
{
    // Subclass QPlainTextEdit so that we can call protected functions
    class Hack : public QPlainTextEdit
    {
    public:
        using QPlainTextEdit::wheelEvent;
    };

    // Call wheel event function only if linecount is larget than text height
    auto lineCount = textEdit()->document()->blockCount();
    auto visibleLines = qCeil(height() / textEdit()->fontMetrics().height());
    if (lineCount > visibleLines)
    {
        // Call event
        static_cast<Hack *>(textEdit())->wheelEvent(event);

        // Disable autoscroll if we are scrolling upwards
        if (autoscroll())
        {
            auto delta = event->angleDelta();
            auto deltaY = delta.y();

            if (deltaY > 0)
            {
                setAutoscroll(false);
                update();
            }
        }

        // Enable autoscroll if scrolling to bottom
        else
        {
            auto bar = textEdit()->verticalScrollBar();
            if (bar->value() >= bar->maximum())
            {
                setAutoscroll(true);
                update();
            }
        }
    }
}

/**
 * Processes the given @a data to remove the escape sequences from the text. Colors and
 * text format is not processed.
 *
 * Implementation based on https://github.com/sebcaux/QVTerminal
 * List of commands: https://espterm.github.io/docs/VT100%20escape%20codes.html
 *
 * I did the necessary stuff to be able to watch ASCII Star Wars from Serial Studio.
 * If you want/need to do more stuff, please make a PR.
 */
QString TerminalWidget::vt100Processing(const QString &data)
{
    QString text;
    QString command;
    bool hasNumbers = false;
    bool hasCommand = false;

    for (int i = 0; i < data.length(); ++i)
    {
        QChar c = data.at(i);
        switch (m_terminalState)
        {
            case VT100_Text:
                if (c == 0x1B)
                {
                    addText(text, false);
                    text.clear();
                    m_terminalState = VT100_Escape;
                }

                else if (c == "\n")
                {
                    addText(text + "\n", false);
                    text.clear();
                }

                else
                    text.append(c);

                break;
            case VT100_Escape:
                command.clear();
                if (c == "[")
                    m_terminalState = VT100_Command;
                else if (c == "(")
                    m_terminalState = VT100_ResetFont;
                break;
            case VT100_Command:
                // Go to escape sequence
                if (c == 0x1B)
                {
                    m_terminalState = VT100_Escape;
                    break;
                }

                // Escape from command mode
                hasNumbers = (c >= '0' && c <= '9');
                hasCommand = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
                if (!hasNumbers && !hasCommand)
                {
                    m_terminalState = VT100_Text;
                    break;
                }

                // Construct command
                command.append(c);

                // Clear screen
                if (command == "2J")
                {
                    textEdit()->clear();
                    m_terminalState = VT100_Text;
                }

                // Move cursor to upper left corner (ugly implementation)
                else if (command == "H")
                {
                    textEdit()->clear();
                    m_terminalState = VT100_Text;
                }

                // Clear line
                else if (command == "2K")
                {
                    textEdit()->setFocus();
                    auto storedCursor = textEdit()->textCursor();
                    textEdit()->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
                    textEdit()->moveCursor(QTextCursor::StartOfLine,
                                           QTextCursor::MoveAnchor);
                    textEdit()->moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
                    textEdit()->textCursor().removeSelectedText();
                    textEdit()->textCursor().deletePreviousChar();
                    textEdit()->setTextCursor(storedCursor);
                    m_terminalState = VT100_Text;
                }

                // Escape to normal text if command length >= 3 chars
                else if (command.length() >= 3)
                    m_terminalState = VT100_Text;

                break;
            case VT100_ResetFont:
                m_terminalState = VT100_Text;
                break;
        }
    }

    // Return VT-100 processed text
    return text;
}
