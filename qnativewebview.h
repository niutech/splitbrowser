#ifndef QNATIVEWEBVIEW_H
#define QNATIVEWEBVIEW_H
#define WEBVIEW_IMPL QNativeWebView

#include "iqwebview.h"

#include <QWidget>
#include <QUrl>
#include <QLineEdit>

class QNativeWebView : public QWidget, public IQWebView
{
    Q_OBJECT
public:
    explicit QNativeWebView(QWidget *parent = nullptr, QUrl url = QUrl());
    ~QNativeWebView();
    void load(const QUrl &url) override;
    void setUrl(const QUrl &url);
    QUrl url() const override;
    QString title() const override;
protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
private:
    void *_wv;
    QUrl _url;
    QString _title;
    QLineEdit *_addressBar;
public slots:
    void back();
    void forward();
    void reload();
    void stop();
signals:
    void loadFinished(bool ok);
    void loadProgress(int progress);
    void loadStarted();
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
};

#endif // QNATIVEWEBVIEW_H
