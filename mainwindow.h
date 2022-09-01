#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QMainWindow>
#include <QThread>

#if USE_WEBKIT
#include "qwebkitwebview.h"
#elif USE_ULTRALIGHT
#include "qultralightwebview.h"
#else
#include "qnativewebview.h"
#endif

#include "DockManager.h"
#include "autosaver.h"
#include "bookmarks.h"
#include "history.h"
#include "iqwebview.h"

#if USE_ULTRALIGHT
#include <AppCore/AppCore.h>
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static QUrl sanitizeUrl(const QUrl &url);
    static HistoryManager *historyManager();
    static BookmarksManager *bookmarksManager();
    QSize sizeHint() const override;
    WEBVIEW_IMPL *currentWebView() const;
protected:
    virtual void closeEvent(QCloseEvent*) override;
private:
    Ui::MainWindow *_ui;
    ads::CDockManager *_dm;
    BookmarksToolBar *_bookmarksToolbar;
    AutoSaver *_autoSaver;
#if USE_ULTRALIGHT
    ultralight::RefPtr<ultralight::App> _ulApp;
#endif
    ads::DockWidgetArea _area = ads::RightDockWidgetArea;
    static HistoryManager *_historyManager;
    static BookmarksManager *_bookmarksManager;
    inline static QUrl _homePage = QUrl("https://start.duckduckgo.com/");
    inline static QUrl _searchPage = QUrl("https://duckduckgo.com/?q=[query]");
    bool saveLayoutOnClose = true;
    bool clearDataOnClose = false;
    void addPane(QUrl url);
    void addWorkspace();
    void removeWorkspace();
    void saveWorkspaces();
    void restoreWorkspaces();
    void saveSettings();
    void restoreSettings();
    bool clearData();
private slots:
    void on_actionQuit_triggered();
    void on_actionSave_toggled(bool checked);
    void on_actionAdd_triggered();
    void on_actionAddWorkspace_triggered();
    void on_actionRemoveWorkspace_triggered();
    void on_tabAreas_triggered();
    void on_actionAbout_triggered();
    void on_actionHomepage_triggered();
    void on_actionSearchpage_triggered();
    void on_actionClear_toggled(bool checked);
    void on_actionMenubar_toggled(bool checked);
    void on_actionBookmarksbar_toggled(bool checked);
    void on_actionFullscreen_toggled(bool checked);
    void onShowBookmarksDialog();
    void onAddBookmark();
    void save();
};
#endif // MAINWINDOW_H
