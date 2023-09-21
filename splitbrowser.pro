DEVELOPER_NAME='Jerzy Glowacki'
APP_NAME='Split Browser'
APP_VERSION=0.2
WEBKIT_VERSION = 1728
WEB_ENGINE = native # webkit/native/ultralight

QT += core gui widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
    DEVELOPER_NAME="\"\\\"$$DEVELOPER_NAME\\\"\"" APP_NAME="\"\\\"$$APP_NAME\\\"\"" \
    APP_VERSION=$$APP_VERSION WEBKIT_VERSION=$$WEBKIT_VERSION

equals(WEB_ENGINE, 'webkit'): DEFINES += USE_WEBKIT=1 USE_ULTRALIGHT=0
else: equals(WEB_ENGINE, 'ultralight'): DEFINES += USE_WEBKIT=0 USE_ULTRALIGHT=1
else: DEFINES += USE_WEBKIT=0 USE_ULTRALIGHT=0

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    autosaver.cpp \
    bookmarks.cpp \
    edittreeview.cpp \
    history.cpp \
    main.cpp \
    mainwindow.cpp \
    modelmenu.cpp \
    searchlineedit.cpp \
    xbel.cpp

equals(WEB_ENGINE, 'webkit'): SOURCES += qwebkitwebview.cpp
else: equals(WEB_ENGINE, 'ultralight'): SOURCES += qultralightwebview.cpp
else: SOURCES += qnativewebview.cpp

HEADERS += \
    autosaver.h \
    bookmarks.h \
    edittreeview.h \
    history.h \
    iqwebview.h \
    mainwindow.h \
    modelmenu.h \
    searchlineedit.h \
    xbel.h

equals(WEB_ENGINE, 'webkit'): HEADERS += qwebkitwebview.h
else: equals(WEB_ENGINE, 'ultralight'): HEADERS += qultralightwebview.h
else: HEADERS += qnativewebview.h webview.h webview2.h

FORMS += \
    addbookmarkdialog.ui \
    bookmarks.ui \
    history.ui \
    mainwindow.ui

CONFIG(debug, debug|release):win32: LIBS += -L$$PWD/lib/ -lqtadvanceddockingd
else: LIBS += -L$$PWD/lib/ -lqtadvanceddocking

win32:equals(WEB_ENGINE, 'native'): LIBS += -lWebView2Loader
unix:equals(WEB_ENGINE, 'native'): LIBS += -lwebkit2gtk-4.0 -lgtk-3 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lsoup-2.4 -lgio-2.0 -ljavascriptcoregtk-4.0 -lgobject-2.0 -lglib-2.0
win32:equals(WEB_ENGINE, 'webkit'): LIBS += -luser32
equals(WEB_ENGINE, 'ultralight'): LIBS += -L$$PWD/ultralight/ -lUltralight -lUltralightCore -lAppCore -lWebCore

INCLUDEPATH += $$PWD/ads
equals(WEB_ENGINE, 'ultralight'): INCLUDEPATH += $$PWD/ultralight/include
unix:equals(WEB_ENGINE, 'native'): INCLUDEPATH += /usr/include/webkitgtk-4.0 /usr/include/gtk-3.0 /usr/include/at-spi2-atk/2.0 /usr/include/at-spi-2.0 /usr/include/dbus-1.0 /usr/lib/x86_64-linux-gnu/dbus-1.0/include /usr/include/gtk-3.0 /usr/include/gio-unix-2.0/ /usr/include/cairo /usr/include/pango-1.0 /usr/include/harfbuzz /usr/include/pango-1.0 /usr/include/atk-1.0 /usr/include/cairo /usr/include/pixman-1 /usr/include/freetype2 /usr/include/libpng16 /usr/include/freetype2 /usr/include/libpng16 /usr/include/gdk-pixbuf-2.0 /usr/include/libpng16 /usr/include/libsoup-2.4 /usr/include/libxml2 /usr/include/webkitgtk-4.0 /usr/include/glib-2.0 /usr/lib/x86_64-linux-gnu/glib-2.0/include

DEPENDPATH += $$PWD/ads
equals(WEB_ENGINE, 'ultralight'): DEPENDPATH += $$PWD/ultralight/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \

RC_ICONS = $$PWD/icons/splitbrowser.ico

DESTDIR = $$OUT_PWD/$$WEB_ENGINE
OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui


ultralightBin.files = $$files($$PWD/../ultralight/bin/*.dll)
ultralightBin.path = $$DESTDIR

ultralightRes.files = $$files($$PWD/../ultralight/bin/resources/*)
ultralightRes.path = $$DESTDIR/assets/resources

equals(WEB_ENGINE, 'ultralight'): COPIES += ultralightBin ultralightRes
