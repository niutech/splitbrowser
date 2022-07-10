#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QMainWindow>
#include <QThread>
#include "DockManager.h"

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
    QSize sizeHint() const override;
protected:
    virtual void closeEvent(QCloseEvent*) override;
private:
    Ui::MainWindow *ui;
    ads::CDockManager *dm;
#if USE_ULTRALIGHT
    ultralight::RefPtr<ultralight::App> ulApp;
#endif
    ads::DockWidgetArea area = ads::RightDockWidgetArea;
    inline static QUrl homePage = QUrl("https://start.duckduckgo.com/");
    inline static QUrl searchPage = QUrl("https://duckduckgo.com/?q=[query]");
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
};
#endif // MAINWINDOW_H
