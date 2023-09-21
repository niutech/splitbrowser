#include "qultralightwebview.h"
#include "mainwindow.h"

#include <QWindow>
#include <QDebug>
#include <QKeyEvent>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidgetAction>

QUltralightWebView::QUltralightWebView(QWidget *parent, QUrl url) : QWidget(parent)
{
    ultralight::App *app = ultralight::App::instance();
    _window = ultralight::Window::Create(app->main_monitor(), 0, 0, false, 0);
    _overlay = ultralight::Overlay::Create(_window, _window->width(), _window->height(), 0, 0);
    _view = _overlay->view();
    _view->set_load_listener(this);
    _view->set_view_listener(this);
    _window->set_listener(this);

    QWidget *wvWidget = createWindowContainer(QWindow::fromWinId((WId) _window->native_handle()), this, Qt::FramelessWindowHint);

    //Toolbar
    QToolBar *toolbar = new QToolBar;
    toolbar->setIconSize(QSize(24, 24));

    QAction *backAction = new QAction(QIcon(":/icons/back.svg"), tr("&Back"), this);
    backAction->setShortcut(QKeySequence("Alt+Left"));
    backAction->setToolTip("Go back");
    connect(backAction, &QAction::triggered, this, &QUltralightWebView::back);
    toolbar->addAction(backAction);

    QAction *forwardAction = new QAction(QIcon(":/icons/forward.svg"), tr("&Forward"), this);
    forwardAction->setShortcut(QKeySequence("Alt+Right"));
    forwardAction->setToolTip("Go forward");
    connect(forwardAction, &QAction::triggered, this, &QUltralightWebView::forward);
    toolbar->addAction(forwardAction);

    QAction *refreshAction = new QAction(QIcon(":/icons/refresh.svg"), tr("&Refresh"), this);
    refreshAction->setShortcuts(QKeySequence::Refresh);
    refreshAction->setToolTip("Refresh");
    connect(refreshAction, &QAction::triggered, this, &QUltralightWebView::reload);
    toolbar->addAction(refreshAction);

    QAction *stopAction = new QAction(QIcon(":/icons/close.svg"), tr("&Stop"), this);
    stopAction->setShortcuts(QKeySequence::Cancel);
    stopAction->setToolTip("Stop");
    refreshAction->setVisible(false);
    connect(stopAction, &QAction::triggered, this, &QUltralightWebView::stop);
    toolbar->addAction(stopAction);

    _addressBar = new QLineEdit;
    _addressBar->setDragEnabled(true);
    _addressBar->installEventFilter(this);
    connect(_addressBar, &QLineEdit::returnPressed, this, [=]() {
        QTimer::singleShot(10, [=]() {
            load(MainWindow::sanitizeUrl(_addressBar->text()));
        });
    });
    connect(this, &QUltralightWebView::urlChanged, [=](QUrl url) {
        _addressBar->setText(url.toString());
    });
    connect(this, &QUltralightWebView::loadStarted, [=]() {
        refreshAction->setVisible(false);
        stopAction->setVisible(true);
    });
    connect(this, &QUltralightWebView::loadFinished, [=]() {
        refreshAction->setVisible(true);
        stopAction->setVisible(false);
    });
    QWidgetAction *addressBarAction = new QWidgetAction(this);
    addressBarAction->setDefaultWidget(_addressBar);
    toolbar->addAction(addressBarAction);

    QFrame *hLine = new QFrame();
    hLine->setFrameShape(QFrame::HLine);
    hLine->setFrameShadow(QFrame::Sunken);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(toolbar);
    layout->addWidget(hLine);
    layout->addWidget(wvWidget);
    setLayout(layout);

    setFocusPolicy(Qt::WheelFocus);

    load(url);

    QTimer::singleShot(0, [=]() {
        if(!app->is_running())
            app->Run();
        setFocus();
    });
}

QUltralightWebView::~QUltralightWebView()
{
    _view->set_load_listener(nullptr);
    _view->set_view_listener(nullptr);
    _window->set_listener(nullptr);
    _window->Close();
}

void QUltralightWebView::load(const QUrl &url)
{
    if (!url.isValid())
        return;
    qDebug() << "Loading" << url;
    _view->LoadURL(QUrlToUlString(url));
}


void QUltralightWebView::setUrl(const QUrl &url)
{
    load(url);
}

QUrl QUltralightWebView::url() const
{
    return _url;
}

QString QUltralightWebView::title() const
{
    return _title;
}

bool QUltralightWebView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == _addressBar && event->type() == QEvent::FocusIn)
        QTimer::singleShot(0, _addressBar, &QLineEdit::selectAll);
    return QWidget::eventFilter(obj, event);
}

void QUltralightWebView::keyPressEvent(QKeyEvent* ev)
{
    sendKey(ev);
    QWidget::keyPressEvent(ev);
}

void QUltralightWebView::back()
{
    _view->GoBack();
}

void QUltralightWebView::forward()
{
    _view->GoForward();
}

void QUltralightWebView::reload()
{
    _view->Reload();
}

void QUltralightWebView::stop()
{
    _view->Stop();
}

int QUltralightWebView::QtModsToUlMods(int mods)
{
    int result = 0;
    if (mods & Qt::AltModifier)
      result |= ultralight::KeyEvent::kMod_AltKey;
    if (mods & Qt::ControlModifier)
      result |= ultralight::KeyEvent::kMod_CtrlKey;
    if (mods & Qt::MetaModifier)
      result |= ultralight::KeyEvent::kMod_MetaKey;
    if (mods & Qt::ShiftModifier)
      result |= ultralight::KeyEvent::kMod_ShiftKey;
    return result;
}

QString QUltralightWebView::ulStringToQString(const ultralight::String string)
{
    return QString::fromUtf8(string.utf8().data());
}

QUrl QUltralightWebView::ulStringToQUrl(const ultralight::String string)
{
    return QUrl(ulStringToQString(string));
}

ultralight::String QUltralightWebView::QStringToUlString(const QString string)
{
    return (ultralight::String8) string.toUtf8().constData();
}

ultralight::String QUltralightWebView::QUrlToUlString(const QUrl url)
{
    return QStringToUlString(url.toString());
}

void QUltralightWebView::OnResize(ultralight::Window *window, uint32_t width_px, uint32_t height_px)
{
    _overlay->Resize(width_px, height_px);
}

//void QUltralightWebView::OnClose(ultralight::Window *window)
//{
//    qDebug() << "Not implemented: OnClose" << window;
//}

void QUltralightWebView::OnChangeCursor(ultralight::View* caller, ultralight::Cursor cursor) {
    _window->SetCursor(cursor);
}

void QUltralightWebView::OnChangeTitle(ultralight::View *caller, const ultralight::String &title)
{
    _title = ulStringToQString(title);
    emit titleChanged(_title);
}

void QUltralightWebView::OnChangeURL(ultralight::View *caller, const ultralight::String &url)
{
    _url = ulStringToQUrl(url);
    emit urlChanged(_url);
}

//void QUltralightWebView::OnChangeTooltip(ultralight::View *caller, const ultralight::String &tooltip)
//{
//    qDebug() << "Not implemented: OnChangeTooltip" << tooltip.utf8().data();
//}

//void QUltralightWebView::OnAddConsoleMessage(ultralight::View *caller, ultralight::MessageSource source, ultralight::MessageLevel level, const ultralight::String &message, uint32_t line_number, uint32_t column_number, const ultralight::String &source_id)
//{
//    if (level == ultralight::MessageLevel::kMessageLevel_Log || level == ultralight::MessageLevel::kMessageLevel_Info)
//        qInfo() << ">>" << ulStringToQString(message);
//    else if (level == ultralight::MessageLevel::kMessageLevel_Debug)
//        qDebug() << ">>" << ulStringToQString(message);
//    else if (level == ultralight::MessageLevel::kMessageLevel_Warning)
//        qWarning() << ">>" << ulStringToQString(message);
//    else if (level == ultralight::MessageLevel::kMessageLevel_Error)
//        qCritical() << ">>" << ulStringToQString(message);
//}

//ultralight::RefPtr<ultralight::View> QUltralightWebView::OnCreateChildView(ultralight::View *caller, const ultralight::String &opener_url, const ultralight::String &target_url, bool is_popup, const ultralight::IntRect &popup_rect)
//{
//    qDebug() << "Not implemented: OnCreateChildView";
//    return nullptr;
//}

void QUltralightWebView::OnBeginLoading(ultralight::View *caller, uint64_t frame_id, bool is_main_frame, const ultralight::String &url)
{
    if (is_main_frame) {
        emit loadStarted();
        emit loadProgress(0);
    }
}

void QUltralightWebView::OnFinishLoading(ultralight::View *caller, uint64_t frame_id, bool is_main_frame, const ultralight::String &url)
{
    if (is_main_frame) {
        emit loadProgress(100);
        emit loadFinished(true);
    }
}

void QUltralightWebView::OnFailLoading(ultralight::View *caller, uint64_t frame_id, bool is_main_frame, const ultralight::String &url, const ultralight::String &description, const ultralight::String &error_domain, int error_code)
{
    qCritical() << "ERROR" << error_code << "(" << ulStringToQString(error_domain) << ")" << ulStringToQString(description) << "while loading" << ulStringToQUrl(url).toString();
    if (is_main_frame) {
        emit loadProgress(100);
        emit loadFinished(false);
    }
}

//void QUltralightWebView::OnWindowObjectReady(ultralight::View *caller, uint64_t frame_id, bool is_main_frame, const ultralight::String &url)
//{
//    qDebug() << "Not implemented: OnWindowObjectReady";
//}

//void QUltralightWebView::OnDOMReady(ultralight::View *caller, uint64_t frame_id, bool is_main_frame, const ultralight::String &url)
//{
//    qDebug() << "Not implemented: OnDOMReady";
//}

//void QUltralightWebView::OnUpdateHistory(ultralight::View *caller)
//{
//    qDebug() << "Not implemented: OnUpdateHistory";
//}

void QUltralightWebView::sendKey(QKeyEvent *event)
{
    ultralight::KeyEvent evt;
    evt.type = ultralight::KeyEvent::kType_RawKeyDown;
    evt.virtual_key_code = event->nativeVirtualKey();
    GetKeyIdentifierFromVirtualKeyCode(event->nativeVirtualKey(), evt.key_identifier);
    evt.modifiers = QtModsToUlMods(event->modifiers());
    _overlay->view()->FireKeyEvent(evt);
    // Support typing chars
    evt.type = ultralight::KeyEvent::kType_Char;
    evt.text = QStringToUlString(event->text());
    evt.unmodified_text = evt.text;
    _overlay->view()->FireKeyEvent(evt);
}
