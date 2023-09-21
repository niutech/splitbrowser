#include "qwebkitwebview.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include <QGridLayout>
#include <QTimer>
#include <QWindow>
#include <QThread>

using namespace std;

QWebKitWebView::QWebKitWebView(QWidget *parent, QUrl url) : QWidget(parent)
{
#ifndef Q_OS_WIN
    QMessageBox::critical(this, QApplication::applicationName(), "WebKit webview is unavailable. Use the native WebViewGTK engine instead.");
    exit(1);
#else
    _process = new QProcess(this);
    _process->start("webkit\\Playwright.exe", QStringList(url.toString()));
    _process->waitForStarted();
    HWND winId = NULL;
    while (!winId) {
        QThread::msleep(100);
        winId = FindWindow(L"Playwright", NULL);
    }
    HWND toolbarId = FindWindowEx(winId, NULL, L"ToolbarWindow32", NULL);
    HWND addressBarId = FindWindowEx(toolbarId, NULL, L"Edit", NULL);
    QWindow *window = QWindow::fromWinId((WId) winId);
    QWidget *wvWidget = createWindowContainer(window, this, Qt::FramelessWindowHint);
    QGridLayout *layout = new QGridLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(wvWidget);
    setLayout(layout);

    QTimer *timer = new QTimer(this);
    timer->setTimerType(Qt::VeryCoarseTimer);
    connect(timer, &QTimer::timeout, this, [=]() {
        wstring wtitle(GetWindowTextLength(winId), L'\0');
        GetWindowText(winId, &wtitle[0], (int) wtitle.capacity());
        QString title = QString::fromStdWString(wtitle).replace(" [WebKit]", "");
        if (title == "Playwright")
            title = "";
        if (_title != title) {
            _title = title;
            emit titleChanged(_title);
        }
        wstring wurl((int) SendMessage(addressBarId, WM_GETTEXTLENGTH, NULL, NULL), L'\0');
        SendMessage(addressBarId, WM_GETTEXT, wurl.capacity(), (LPARAM) &wurl[0]);
        QUrl url = QUrl(QString::fromStdWString(wurl));
        if (_url != url) {
            _url = url;
            emit urlChanged(_url);
        }
    });
    timer->start(1000);
#endif
}

QWebKitWebView::~QWebKitWebView()
{
    _process->terminate();
    _process->kill();
    _process->deleteLater();
}

QUrl QWebKitWebView::url() const
{
    return _url;
}

QString QWebKitWebView::title() const
{
    return _title;
}

void QWebKitWebView::load(const QUrl &url)
{
    //TODO
}
