#ifndef QWEBKITWEBVIEW_H
#define QWEBKITWEBVIEW_H
#define WEBVIEW_IMPL QWebKitWebView

#include "iqwebview.h"

#include <QWidget>
#include <QUrl>
#include <QProcess>

class QWebKitWebView : public QWidget, public IQWebView
{
    Q_OBJECT
public:
    explicit QWebKitWebView(QWidget *parent = nullptr, QUrl url = QUrl());
    ~QWebKitWebView();
    QUrl url() const override;
    QString title() const override;
private:
    QProcess *_process;
    QUrl _url;
    QString _title;
signals:
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
};

#endif // QWEBKITWEBVIEW_H
