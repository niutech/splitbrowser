#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName(DEVELOPER_NAME);
    a.setApplicationName(QString(APP_NAME) + (USE_WEBKIT ? " WebKit" : USE_ULTRALIGHT ? " Ultralight" : " Native"));
    a.setApplicationVersion(QString::number(APP_VERSION));
    MainWindow w;
    w.show();
    return a.exec();
}
