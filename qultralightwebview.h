#ifndef QULTRALIGHTWEBVIEW_H
#define QULTRALIGHTWEBVIEW_H
#define WEBVIEW_IMPL QUltralightWebView

#include "iqwebview.h"

#include <QLineEdit>
#include <QWidget>

#include <AppCore/AppCore.h>

class QUltralightWebView : public QWidget,
        public IQWebView,
        public ultralight::WindowListener,
        public ultralight::LoadListener,
        public ultralight::ViewListener
{
    Q_OBJECT
public:
    explicit QUltralightWebView(QWidget *parent = nullptr, QUrl url = QUrl());
    ~QUltralightWebView();
    void load(const QUrl &url) override;
    void setUrl(const QUrl &url);
    QUrl url() const override;
    QString title() const override;
protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void keyPressEvent(QKeyEvent*) override;
private:
    ultralight::RefPtr<ultralight::Window> _window;
    ultralight::RefPtr<ultralight::Overlay> _overlay;
    ultralight::RefPtr<ultralight::View> _view;
    QUrl _url;
    QString _title;
    QLineEdit *_addressBar;
    static int QtModsToUlMods(int mods);
    static QString ulStringToQString(const ultralight::String string);
    static QUrl ulStringToQUrl(const ultralight::String string);
    static ultralight::String QStringToUlString(const QString string);
    static ultralight::String QUrlToUlString(const QUrl url);
    void OnResize(ultralight::Window* window, uint32_t width_px, uint32_t height_px) override;
//    void OnClose(ultralight::Window* window) override;
    void OnChangeCursor(ultralight::View* caller, ultralight::Cursor cursor) override;
    void OnChangeTitle(ultralight::View* caller, const ultralight::String& title) override;
    void OnChangeURL(ultralight::View* caller, const ultralight::String& url) override;
//    void OnChangeTooltip(ultralight::View* caller, const ultralight::String& tooltip) override;
//    void OnAddConsoleMessage(ultralight::View* caller, ultralight::MessageSource source, ultralight::MessageLevel level,  const ultralight::String& message, uint32_t line_number, uint32_t column_number, const ultralight::String& source_id) override;
//    ultralight::RefPtr<ultralight::View> OnCreateChildView(ultralight::View* caller, const ultralight::String& opener_url, const ultralight::String& target_url, bool is_popup, const ultralight::IntRect& popup_rect) override;
    void OnBeginLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url) override;
    void OnFinishLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url) override;
    void OnFailLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url, const ultralight::String& description, const ultralight::String& error_domain, int error_code) override;
//    void OnWindowObjectReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url) override;
//    void OnDOMReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url) override;
//    void OnUpdateHistory(ultralight::View* caller) override;
public slots:
    void back();
    void forward();
    void reload();
    void stop();
    void sendKey(QKeyEvent *event);
signals:
    void loadFinished(bool ok);
    void loadProgress(int progress);
    void loadStarted();
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
};

#endif // QULTRALIGHTWEBVIEW_H
