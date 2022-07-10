#include "mainwindow.h"
#include "ui_mainwindow.h"
#if USE_WEBKIT
#include "qwebkitwebview.h"
#elif USE_ULTRALIGHT
#include "qultralightwebview.h"
#else
#include "qnativewebview.h"
#endif
#include "DockAreaWidget.h"
#include "DockAreaTitleBar.h"
#include "DockAreaTabBar.h"

#include <QSettings>
#include <QInputDialog>
#include <QWidgetAction>
#include <QMessageBox>
#include <QToolButton>
#include <QDesktopWidget>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

#ifndef ULTRALIGHT_VERSION
#define ULTRALIGHT_VERSION ""
#endif

using namespace std;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#if USE_ULTRALIGHT
    ultralight::Settings settings;
    settings.developer_name = DEVELOPER_NAME;
    settings.app_name = APP_NAME " Ultralight";
    ultralight::Config config;
//    config.memory_cache_size = 128 * 1024 * 1024;
//    config.page_cache_size = 2;
    ulApp = ultralight::App::Create(settings, config);
#endif

    // Dock
    ads::CDockManager::setConfigFlag(ads::CDockManager::AllTabsHaveCloseButton);
    ads::CDockManager::setConfigFlag(ads::CDockManager::EqualSplitOnInsertion);
    //ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting);
    dm = new ads::CDockManager(this);
    connect(dm, &ads::CDockManager::dockAreaCreated, [=](ads::CDockAreaWidget *da) {
        ads::CDockAreaTitleBar *titleBar = da->titleBar();
        QToolButton *addPaneButton = new QToolButton;
        addPaneButton->setDefaultAction(ui->actionAdd);
        addPaneButton->setAutoRaise(true);
        titleBar->insertWidget(titleBar->indexOf(titleBar->tabBar()) + 1, addPaneButton);
    });

    restoreSettings();
    restoreWorkspaces();

    if (!dm->openedDockWidgets().size())
        addPane(homePage);

    dm->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(dm, &ads::CDockManager::customContextMenuRequested, [=](const QPoint &pos) {
        QMenu *menu = new QMenu(this);
        menu->addAction(ui->actionMenubar);
        menu->addAction(ui->actionAdd);
        menu->exec(mapToGlobal(pos));
    });
}

MainWindow::~MainWindow()
{
    delete dm;
    delete ui;
    if (clearDataOnClose)
        clearData();
}

QUrl MainWindow::sanitizeUrl(const QUrl &url)
{
    if (!url.isValid())
        return QUrl("about:blank");
    if (url.isRelative()) {
        if (url.toString().endsWith(".com") ||
            url.toString().endsWith(".net") ||
            url.toString().endsWith(".org") ||
            url.toString().endsWith(".info") ||
            url.toString().endsWith(".co") ||
            url.toString().endsWith(".uk") ||
            url.toString().endsWith(".de") ||
            url.toString().endsWith(".fr") ||
            url.toString().endsWith(".pl") ||
            url.toString().endsWith(".io")) {
            return QUrl("http://" + url.toEncoded());
        }
        return QUrl(searchPage.toString().replace("[query]", url.toEncoded()));
    }
    return url;
}

QSize MainWindow::sizeHint() const
{
    return QDesktopWidget().availableGeometry(this).size() * 0.9;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveSettings();
#if USE_ULTRALIGHT
    ulApp->Quit();
#endif
    QMainWindow::closeEvent(event);
}

void MainWindow::addPane(QUrl url)
{
    ads::CDockWidget* dw = new ads::CDockWidget("Loading...");
    WEBVIEW_IMPL *wv = new WEBVIEW_IMPL(dw, url);
    dw->setWidget(wv);
    dw->setObjectName(QString::number(dm->dockWidgetsMap().size()));
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    dm->addDockWidget(area, dw, dm->dockAreaAt(QCursor::pos()));

    connect(wv, &WEBVIEW_IMPL::titleChanged, [=](const QString &title) {
        QString shortTitle(title);
        if (shortTitle.size() > 80) {
            shortTitle.truncate(80);
            shortTitle.append("…");
        }
        dw->setWindowTitle(shortTitle);
    });
}

void MainWindow::addWorkspace()
{
    QString workspaceName = QInputDialog::getText(this, "Add new workspace", "Type unique name:");
    if (workspaceName.isEmpty())
        return;

    dm->addPerspective(workspaceName);
    QAction *a = new QAction(workspaceName);
    a->setActionGroup(ui->workspaces);
    a->setCheckable(true);
    a->setChecked(true);
    ui->menuWorkspaces->addAction(a);
    ui->actionRemoveWorkspace->setVisible(true);
    saveWorkspaces();
}

void MainWindow::removeWorkspace()
{
    if (ui->workspaces->actions().size() <= 1)
        return;

    QAction *a = ui->workspaces->checkedAction();

    QMessageBox *confirmBox = new QMessageBox(this);
    confirmBox->setText("Do you want to remove the current workspace " + a->text() + "?");
    confirmBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox->setDefaultButton(QMessageBox::No);
    if(confirmBox->exec() != QMessageBox::Yes)
        return;

    dm->removePerspective(a->text());
    ui->workspaces->removeAction(a);
    ui->menuWorkspaces->removeAction(a);
    ui->menuWorkspaces->actions().first()->setChecked(true);
    saveWorkspaces();
}

void MainWindow::saveWorkspaces()
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    dm->savePerspectives(settings);
}

void MainWindow::restoreWorkspaces()
{
    QSettings settings("Settings.ini", QSettings::IniFormat);
    dm->loadPerspectives(settings);
    ui->menuWorkspaces->clear();
    foreach (const QString &name, dm->perspectiveNames()) {
        QAction *a = new QAction(name);
        a->setCheckable(true);
        a->setActionGroup(ui->workspaces);
        ui->menuWorkspaces->addAction(a);
    }
    ui->actionRemoveWorkspace->setVisible(ui->workspaces->actions().size() > 1);
    if (ui->workspaces->actions().size() > 0)
        ui->menuWorkspaces->actions().first()->setChecked(true);
}

void MainWindow::saveSettings()
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    settings.setValue("browser/area", area);
    settings.setValue("browser/homePage", homePage);
    settings.setValue("browser/searchPage", searchPage);
    settings.setValue("browser/saveLayout", saveLayoutOnClose);
    settings.setValue("browser/geometry", saveGeometry());
    settings.setValue("browser/state", saveState());
    settings.setValue("browser/dockingState", dm->saveState());

    if (saveLayoutOnClose) {
        settings.beginGroup("tabs");
        settings.remove("");
        settings.endGroup();
        settings.beginWriteArray("tabs");
        QList<ads::CDockWidget*> tabs = dm->openedDockWidgets();
        for(int i = 0; i < tabs.size(); i++) {
            settings.setArrayIndex(i);
            settings.setValue("URL", ((WEBVIEW_IMPL*) tabs[i]->widget())->url());
        }
        settings.endArray();
    }

    settings.sync();
}

void MainWindow::restoreSettings()
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    restoreGeometry(settings.value("browser/geometry").toByteArray());
    restoreState(settings.value("browser/state").toByteArray());
    area = (ads::DockWidgetArea) settings.value("browser/area", area).toInt();
    homePage = settings.value("browser/homePage", homePage).toString();
    searchPage = settings.value("browser/searchPage", searchPage).toString();
    saveLayoutOnClose = settings.value("browser/saveLayout", saveLayoutOnClose).toBool();

    ui->actionSave->setChecked(saveLayoutOnClose);
    if (area == ads::RightDockWidgetArea)
        ui->actionRight->setChecked(true);
    else if (area == ads::LeftDockWidgetArea)
        ui->actionLeft->setChecked(true);
    else if (area == ads::TopDockWidgetArea)
        ui->actionTop->setChecked(true);
    else if (area == ads::BottomDockWidgetArea)
        ui->actionBottom->setChecked(true);
    else
        ui->actionCenter->setChecked(true);

    if (saveLayoutOnClose) {
        int tabs = settings.beginReadArray("tabs");
        for(int i = 0; i < tabs; i++) {
            settings.setArrayIndex(i);
            addPane(settings.value("URL").toUrl());
        }
        settings.endArray();
        dm->restoreState(settings.value("browser/dockingState").toByteArray());
    }
}

bool MainWindow::clearData()
{
#if USE_WEBKIT
    QString path(QProcessEnvironment::systemEnvironment().value("LOCALAPPDATA") + "\\Apple Computer\\WebKit");
#elif USE_ULTRALIGHT
    QString path(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
#elif defined(_WIN32)
    QString path(QProcessEnvironment::systemEnvironment().value("APPDATA") + "\\" + QFileInfo(QApplication::applicationFilePath()).fileName() + "\\EBWebView");
#elif defined(__unix__)
    QString path(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.cache/" + QFileInfo(QApplication::applicationFilePath()).fileName() + "/WebKitCache");
#endif
    qDebug() << "Removing" << path;
    QThread::msleep(500);
    return QDir(path).removeRecursively();
}

void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::on_actionSave_toggled(bool checked)
{
    saveLayoutOnClose = checked;
}

void MainWindow::on_actionAdd_triggered()
{
    addPane(homePage);
}

void MainWindow::on_actionAddWorkspace_triggered()
{
    addWorkspace();
}

void MainWindow::on_actionRemoveWorkspace_triggered()
{
    removeWorkspace();
}

void MainWindow::on_tabAreas_triggered()
{
    QString name = ui->tabAreas->checkedAction()->objectName();
    if (name == "actionRight")
        area = ads::RightDockWidgetArea;
    else if (name == "actionLeft")
        area = ads::LeftDockWidgetArea;
    else if (name == "actionTop")
        area = ads::TopDockWidgetArea;
    else if (name == "actionBottom")
        area = ads::BottomDockWidgetArea;
    else
        area = ads::CenterDockWidgetArea;
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About " + QApplication::applicationName(),
                       QApplication::applicationName() + " version " + QApplication::applicationVersion() +
#if USE_WEBKIT
                       "\nPlaywright WebKit revision " + QString::number(WEBKIT_VERSION) +
#elif USE_ULTRALIGHT
                       "\nUltralight version " + QString::fromStdString(ULTRALIGHT_VERSION) +
#endif
                       "\n© 2022 Jerzy Głowacki\nMIT License");
}

void MainWindow::on_actionHomepage_triggered()
{
    bool ok;
    QUrl url = QUrl(QInputDialog::getText(this, "Set home page", "URL:", QLineEdit::Normal, homePage.toString(), &ok));
    if (ok && url.isValid() && !url.isRelative())
        homePage = url;
    else if (ok)
        QMessageBox::warning(this, "Set home page", "Invalid URL");
}

void MainWindow::on_actionSearchpage_triggered()
{
    bool ok;
    QUrl url = QUrl(QInputDialog::getText(this, "Set search page", "URL with [query]:", QLineEdit::Normal, searchPage.toString(), &ok));
    if (ok && url.isValid() && !url.isRelative() && url.toString().contains("[query]"))
        searchPage = url;
    else if (ok)
        QMessageBox::warning(this, "Set search page", "Invalid URL or does not contain [query]");
}

void MainWindow::on_actionClear_toggled(bool checked)
{
    clearDataOnClose = checked;
}

void MainWindow::on_actionMenubar_toggled(bool checked)
{
    ui->menubar->setVisible(checked);
}
