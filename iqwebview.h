#ifndef IQWEBVIEW_H
#define IQWEBVIEW_H

#include <QUrl>

class IQWebView
{
public:
    virtual QUrl url() const = 0;
    virtual QString title() const = 0;
    virtual void load(const QUrl &url) = 0;
signals:
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
};

#endif // IQWEBVIEW_H
