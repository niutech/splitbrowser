#include "mainwindow.h"
#include "ui_mainwindow.h"
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

HistoryManager *MainWindow::_historyManager = 0;
BookmarksManager *MainWindow::_bookmarksManager = 0;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), _ui(new Ui::MainWindow), _autoSaver(new AutoSaver(this))
{
    _ui->setupUi(this);

#if USE_ULTRALIGHT
    ultralight::Settings settings;
    settings.developer_name = DEVELOPER_NAME;
    settings.app_name = APP_NAME " Ultralight";
    ultralight::Config config;
//    config.memory_cache_size = 128 * 1024 * 1024;
//    config.page_cache_size = 2;
    _ulApp = ultralight::App::Create(settings, config);
#endif

    // Dock
    ads::CDockManager::setConfigFlag(ads::CDockManager::AllTabsHaveCloseButton);
    ads::CDockManager::setConfigFlag(ads::CDockManager::EqualSplitOnInsertion);
    //ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting);
    _dm = new ads::CDockManager(this);
    connect(_dm, &ads::CDockManager::dockAreaCreated, [=](ads::CDockAreaWidget *da) {
        ads::CDockAreaTitleBar *titleBar = da->titleBar();
        QToolButton *addPaneButton = new QToolButton;
        addPaneButton->setDefaultAction(_ui->actionAdd);
        addPaneButton->setAutoRaise(true);
        titleBar->insertWidget(titleBar->indexOf(titleBar->tabBar()) + 1, addPaneButton);
    });


    // History
    historyManager();
    HistoryMenu *historyMenu = new HistoryMenu(this);
    connect(historyMenu, &HistoryMenu::openUrl, [=](QUrl url) {
        if (currentWebView()) currentWebView()->load(url);
        else addPane(QUrl(url));
    });
    historyMenu->setObjectName("historyMenu");
    historyMenu->setTitle(tr("Hi&story"));
    menuBar()->insertMenu(_ui->menuSettings->menuAction(), historyMenu);

//    QList<QAction*> historyActions;
//    historyActions.append(m_tabWidget->recentlyClosedTabsAction());
//    historyMenu->setInitialActions(historyActions);

    // Bookmarks
    BookmarksMenu *bookmarksMenu = new BookmarksMenu(this);
    connect(bookmarksMenu, &BookmarksMenu::openUrl, [=](QUrl url) {
        if (currentWebView()) currentWebView()->load(url);
        else addPane(QUrl(url));
    });
    bookmarksMenu->setObjectName("bookmarksMenu");
    bookmarksMenu->setTitle(tr("&Bookmarks"));
    menuBar()->insertMenu(_ui->menuSettings->menuAction(), bookmarksMenu);

    QAction *showAllBookmarksAction = new QAction(tr("Show All Bookmarks"), this);
    connect(showAllBookmarksAction, SIGNAL(triggered()), this, SLOT(onShowBookmarksDialog()));

    QAction *addBookmark = new QAction(tr("Add Bookmark..."), this);
    connect(addBookmark, SIGNAL(triggered()), this, SLOT(onAddBookmark()));
    addBookmark->setShortcut(QKeySequence("Ctrl+D"));

    QAction *importBookmarks = new QAction(tr("&Import Bookmarks..."), this);
    connect(importBookmarks, SIGNAL(triggered()), bookmarksManager(), SLOT(importBookmarks()));

    QAction *exportBookmarks = new QAction(tr("&Export Bookmarks..."), this);
    connect(exportBookmarks, SIGNAL(triggered()), bookmarksManager(), SLOT(exportBookmarks()));

    QList<QAction*> bookmarksActions;
    bookmarksActions.append(showAllBookmarksAction);
    bookmarksActions.append(addBookmark);
    bookmarksActions.append(importBookmarks);
    bookmarksActions.append(exportBookmarks);
    bookmarksMenu->setInitialActions(bookmarksActions);

    BookmarksModel *boomarksModel = bookmarksManager()->bookmarksModel();
    _bookmarksToolbar = new BookmarksToolBar(boomarksModel, this);
    _bookmarksToolbar->setObjectName("bookmarksbar");
    connect(_bookmarksToolbar, &BookmarksToolBar::openUrl, [=](QUrl url) {
        if (currentWebView()) currentWebView()->load(url);
        else addPane(QUrl(url));
    });
    connect(_bookmarksToolbar->toggleViewAction(), SIGNAL(toggled(bool)),
            this, SLOT(on_actionBookmarksbar_toggled(bool)));
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
#if defined(Q_OS_OSX)
    layout->addWidget(_bookmarksToolbar);
    layout->addWidget(new QWidget); // OS X tab widget style bug
#else
    addToolBarBreak();
    addToolBar(_bookmarksToolbar);
#endif

    // Context menu
    _dm->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_dm, &ads::CDockManager::customContextMenuRequested, [=](const QPoint &pos) {
        QMenu *menu = new QMenu(this);
        menu->addAction(_ui->actionMenubar);
        menu->addAction(_ui->actionBookmarksbar);
        menu->addAction(_ui->actionAdd);
        menu->exec(mapToGlobal(pos));
    });

    restoreSettings();
    restoreWorkspaces();

    if (!_dm->openedDockWidgets().size())
        addPane(_homePage);

//    for (ads::CDockWidget *dw : _dm->dockWidgetsMap())
//	{
//		_this->connect(dw, SIGNAL(viewToggled(bool)), SLOT(onViewToggled(bool)));
//		_this->connect(dw, SIGNAL(visibilityChanged(bool)), SLOT(onViewVisibilityChanged(bool)));
//	}
}

MainWindow::~MainWindow()
{
    _autoSaver->changeOccurred();
    _autoSaver->saveIfNeccessary();
    delete _dm;
    delete _ui;
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
        return QUrl(_searchPage.toString().replace("[query]", url.toEncoded()));
    }
    return url;
}

HistoryManager *MainWindow::historyManager()
{
    if (!_historyManager) {
        _historyManager = new HistoryManager();
    }
    return _historyManager;
}

BookmarksManager *MainWindow::bookmarksManager()
{
    if (!_bookmarksManager) {
        _bookmarksManager = new BookmarksManager();
    }
    return _bookmarksManager;
}

QSize MainWindow::sizeHint() const
{
    return QDesktopWidget().availableGeometry(this).size() * 0.9;
}

WEBVIEW_IMPL *MainWindow::currentWebView() const
{
    if (_dm->dockArea(0) && _dm->dockArea(0)->currentDockWidget())
        return (WEBVIEW_IMPL*) _dm->dockArea(0)->currentDockWidget()->widget();
    return nullptr;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveSettings();
#if USE_ULTRALIGHT
    _ulApp->Quit();
#endif
    QMainWindow::closeEvent(event);
}

void MainWindow::addPane(QUrl url)
{
    ads::CDockWidget* dw = new ads::CDockWidget(tr("Loading..."));
    WEBVIEW_IMPL *wv = new WEBVIEW_IMPL(dw, url);
    dw->setWidget(wv);
    dw->setObjectName(QString::number(_dm->dockWidgetsMap().size()));
    dw->setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);
    _dm->addDockWidget(_area, dw, _dm->dockAreaAt(QCursor::pos()));

    connect(wv, &WEBVIEW_IMPL::titleChanged, [=](const QString &title) {
        qDebug() << "Title changed to:" << title;
        QString shortTitle(title);
        if (shortTitle.size() > 80) {
            shortTitle.truncate(80);
            shortTitle.append("…");
        } else if (shortTitle == "")
            shortTitle = tr("Untitled");
        dw->setWindowTitle(shortTitle);
        historyManager()->updateHistoryItem(wv->url(), title);
    });
    connect(wv, &WEBVIEW_IMPL::urlChanged, [=](const QUrl &url) {
        qDebug() << "URL changed to:" << url;
        if (url.isValid())
            historyManager()->addHistoryEntry(url.toString());
    });
}

void MainWindow::addWorkspace()
{
    QString workspaceName = QInputDialog::getText(this, tr("Add new workspace"), tr("Type unique name:"));
    if (workspaceName.isEmpty())
        return;

    _dm->addPerspective(workspaceName);
    QAction *a = new QAction(workspaceName);
    a->setActionGroup(_ui->workspaces);
    a->setCheckable(true);
    a->setChecked(true);
    _ui->menuWorkspaces->addAction(a);
    _ui->actionRemoveWorkspace->setVisible(true);
    saveWorkspaces();
}

void MainWindow::removeWorkspace()
{
    if (_ui->workspaces->actions().size() <= 1)
        return;

    QAction *a = _ui->workspaces->checkedAction();

    QMessageBox *confirmBox = new QMessageBox(this);
    confirmBox->setText("Do you want to remove the current workspace " + a->text() + "?");
    confirmBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox->setDefaultButton(QMessageBox::No);
    if(confirmBox->exec() != QMessageBox::Yes)
        return;

    _dm->removePerspective(a->text());
    _ui->workspaces->removeAction(a);
    _ui->menuWorkspaces->removeAction(a);
    _ui->menuWorkspaces->actions().first()->setChecked(true);
    saveWorkspaces();
}

void MainWindow::saveWorkspaces()
{
    QSettings settings;
    _dm->savePerspectives(settings);
}

void MainWindow::restoreWorkspaces()
{
    QSettings settings;
    _dm->loadPerspectives(settings);
    _ui->menuWorkspaces->clear();
    foreach (const QString &name, _dm->perspectiveNames()) {
        QAction *a = new QAction(name);
        a->setCheckable(true);
        a->setActionGroup(_ui->workspaces);
        _ui->menuWorkspaces->addAction(a);
    }
    _ui->actionRemoveWorkspace->setVisible(_ui->workspaces->actions().size() > 1);
    if (_ui->workspaces->actions().size() > 0)
        _ui->menuWorkspaces->actions().first()->setChecked(true);
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("browser/area", _area);
    settings.setValue("browser/homePage", _homePage);
    settings.setValue("browser/searchPage", _searchPage);
    settings.setValue("browser/saveLayout", saveLayoutOnClose);
    settings.setValue("browser/geometry", saveGeometry());
    settings.setValue("browser/state", saveState());
    settings.setValue("browser/dockingState", _dm->saveState());
    settings.setValue("browser/bookmarksBar", _bookmarksToolbar->isVisible());

    if (saveLayoutOnClose) {
        settings.beginGroup("tabs");
        settings.remove("");
        settings.endGroup();
        settings.beginWriteArray("tabs");
        QList<ads::CDockWidget*> tabs = _dm->openedDockWidgets();
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
    QSettings settings;
    restoreGeometry(settings.value("browser/geometry").toByteArray());
    restoreState(settings.value("browser/state").toByteArray());
    _area = (ads::DockWidgetArea) settings.value("browser/area", _area).toInt();
    _homePage = settings.value("browser/homePage", _homePage).toString();
    _searchPage = settings.value("browser/searchPage", _searchPage).toString();
    saveLayoutOnClose = settings.value("browser/saveLayout", saveLayoutOnClose).toBool();
    _bookmarksToolbar->setVisible(settings.value("browser/bookmarksBar", false).toBool());

    _ui->actionSave->setChecked(saveLayoutOnClose);
    if (_area == ads::RightDockWidgetArea)
        _ui->actionRight->setChecked(true);
    else if (_area == ads::LeftDockWidgetArea)
        _ui->actionLeft->setChecked(true);
    else if (_area == ads::TopDockWidgetArea)
        _ui->actionTop->setChecked(true);
    else if (_area == ads::BottomDockWidgetArea)
        _ui->actionBottom->setChecked(true);
    else
        _ui->actionCenter->setChecked(true);

    if (saveLayoutOnClose) {
        int tabs = settings.beginReadArray("tabs");
        for(int i = 0; i < tabs; i++) {
            settings.setArrayIndex(i);
            addPane(settings.value("URL").toUrl());
        }
        settings.endArray();
        _dm->restoreState(settings.value("browser/dockingState").toByteArray());
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
    addPane(_homePage);
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
    QString name = _ui->tabAreas->checkedAction()->objectName();
    if (name == "actionRight")
        _area = ads::RightDockWidgetArea;
    else if (name == "actionLeft")
        _area = ads::LeftDockWidgetArea;
    else if (name == "actionTop")
        _area = ads::TopDockWidgetArea;
    else if (name == "actionBottom")
        _area = ads::BottomDockWidgetArea;
    else
        _area = ads::CenterDockWidgetArea;
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
    QUrl url = QUrl(QInputDialog::getText(this, tr("Set home page"), tr("URL:"), QLineEdit::Normal, _homePage.toString(), &ok));
    if (ok && url.isValid() && !url.isRelative())
        _homePage = url;
    else if (ok)
        QMessageBox::warning(this, tr("Set home page"), tr("Invalid URL"));
}

void MainWindow::on_actionSearchpage_triggered()
{
    bool ok;
    QUrl url = QUrl(QInputDialog::getText(this, tr("Set search page"), tr("URL with [query]:"), QLineEdit::Normal, _searchPage.toString(), &ok));
    if (ok && url.isValid() && !url.isRelative() && url.toString().contains("[query]"))
        _searchPage = url;
    else if (ok)
        QMessageBox::warning(this, tr("Set search page"), tr("Invalid URL or does not contain [query]"));
}

void MainWindow::on_actionClear_toggled(bool checked)
{
    clearDataOnClose = checked;
}

void MainWindow::on_actionMenubar_toggled(bool checked)
{
    _ui->menubar->setVisible(checked);
}

void MainWindow::on_actionBookmarksbar_toggled(bool checked)
{
    qDebug() << "Toggled bookmarksbar" << checked;
    _ui->actionBookmarksbar->setChecked(checked);
    _bookmarksToolbar->setVisible(checked);
    _autoSaver->changeOccurred();
}

void MainWindow::on_actionFullscreen_toggled(bool checked)
{
    if (checked) {
        showFullScreen();
#if USE_NATIVE
        currentWebView()->findChild<QToolBar*>("wvtoolbar")->hide();
#endif
    } else {
        if (isMinimized())
            showMinimized();
        else if (isMaximized())
            showMaximized();
        else showNormal();
#if USE_NATIVE
        currentWebView()->findChild<QToolBar*>("wvtoolbar")->show();
#endif
    }
}

void MainWindow::onShowBookmarksDialog()
{
    BookmarksDialog *dialog = new BookmarksDialog(this);
    connect(dialog, SIGNAL(openUrl(QUrl)),
            currentWebView(), SLOT(load(QUrl)));
    dialog->show();
}

void MainWindow::onAddBookmark()
{
    IQWebView *webView = currentWebView();
    QString url = webView->url().toString();
    QString title = webView->title();
    AddBookmarkDialog dialog(url, title);
    dialog.exec();
}

void MainWindow::save()
{
    // TODO
}
