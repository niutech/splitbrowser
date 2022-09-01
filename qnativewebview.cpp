#include "qnativewebview.h"
#include "mainwindow.h"
#include "webview.h"

#include <QWindow>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QMessageBox>
#include <QApplication>

#define WEBVIEW ((webview::webview *) _wv)

using namespace std;

QNativeWebView::QNativeWebView(QWidget *parent, QUrl url) : QWidget(parent)
{
    _wv = new webview::webview(false, nullptr);
    if (!WEBVIEW->window()) {
        QMessageBox::critical(this, QApplication::applicationName(), "Cannot initialize native webview!"
#ifdef _WIN32
           "\nInstall Edge WebView2 from https://developer.microsoft.com/en-us/microsoft-edge/webview2/consumer/"
#endif
        );
        exit(1);
    }

    WEBVIEW->bind("qtInit", [=](string log) -> string {
        _url = WEBVIEW->get_url();
        _title = WEBVIEW->get_title();
        emit urlChanged(_url);
        emit titleChanged(_title);
        emit loadProgress(100);
        emit loadFinished(true);
        return log;
    });
    WEBVIEW->init("addEventListener('DOMContentLoaded', qtInit)");
    QWidget *wvWidget = createWindowContainer(QWindow::fromWinId((unsigned long long) WEBVIEW->window()));

    //Toolbar
    QToolBar *toolbar = new QToolBar;
    toolbar->setIconSize(QSize(24, 24));

    QAction *backAction = new QAction(QIcon(":/icons/back.svg"), tr("&Back"), this);
    backAction->setShortcut(QKeySequence("Alt+Left"));
    backAction->setToolTip("Go back");
    connect(backAction, &QAction::triggered, this, &QNativeWebView::back);
    toolbar->addAction(backAction);

    QAction *forwardAction = new QAction(QIcon(":/icons/forward.svg"), tr("&Forward"), this);
    forwardAction->setShortcut(QKeySequence("Alt+Right"));
    forwardAction->setToolTip("Go forward");
    connect(forwardAction, &QAction::triggered, this, &QNativeWebView::forward);
    toolbar->addAction(forwardAction);

    QAction *refreshAction = new QAction(QIcon(":/icons/refresh.svg"), tr("&Refresh"), this);
    refreshAction->setShortcuts(QKeySequence::Refresh);
    refreshAction->setToolTip("Refresh");
    connect(refreshAction, &QAction::triggered, this, &QNativeWebView::reload);
    toolbar->addAction(refreshAction);

    QAction *stopAction = new QAction(QIcon(":/icons/close.svg"), tr("&Stop"), this);
    stopAction->setShortcuts(QKeySequence::Cancel);
    stopAction->setToolTip("Stop");
    refreshAction->setVisible(false);
    connect(stopAction, &QAction::triggered, this, &QNativeWebView::stop);
    toolbar->addAction(stopAction);

    _addressBar = new QLineEdit;
    _addressBar->setDragEnabled(true);
    _addressBar->installEventFilter(this);
    connect(_addressBar, &QLineEdit::returnPressed, this, [=]() {
        load(MainWindow::sanitizeUrl(_addressBar->text()));
    });
    connect(this, &QNativeWebView::urlChanged, [=](QUrl url) {
        _addressBar->setText(url.toString());
    });
    connect(this, &QNativeWebView::loadStarted, [=]() {
        refreshAction->setVisible(false);
        stopAction->setVisible(true);
    });
    connect(this, &QNativeWebView::loadFinished, [=]() {
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

    load(url);
}

QNativeWebView::~QNativeWebView()
{
    delete WEBVIEW;
}

void QNativeWebView::load(const QUrl &url)
{
    if (!url.isValid())
        return;
    _url = url;
    emit loadStarted();
    emit loadProgress(0);
    WEBVIEW->navigate(_url.toString().toStdString());
    WEBVIEW->step(0);
}

void QNativeWebView::setUrl(const QUrl &url)
{
    load(url);
}

QUrl QNativeWebView::url() const
{
    return _url;
}

QString QNativeWebView::title() const
{
    return _title;
}

bool QNativeWebView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == _addressBar && event->type() == QEvent::FocusIn)
        QTimer::singleShot(0, _addressBar, &QLineEdit::selectAll);
    return QWidget::eventFilter(obj, event);
}

void QNativeWebView::back()
{
    emit loadStarted();
    emit loadProgress(0);
    WEBVIEW->go_back();
}

void QNativeWebView::forward()
{
    emit loadStarted();
    emit loadProgress(0);
    WEBVIEW->go_forward();
}

void QNativeWebView::reload()
{
    emit loadStarted();
    emit loadProgress(0);
    WEBVIEW->reload();
}

void QNativeWebView::stop()
{
    WEBVIEW->stop();
    emit loadProgress(100);
    emit loadFinished(true);
}
