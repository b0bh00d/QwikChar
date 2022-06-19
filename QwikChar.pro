QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

RESOURCES += ./QwikChar.qrc

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

mac {
    DEFINES += QT_OSX
}

unix:!mac {
    DEFINES += QT_LINUX
}

win32 {
    DEFINES += QT_WIN
    SOURCES += win32.cpp
}


SOURCES += \
    clickablelabel.cpp \
    main.cpp \
    mainwindow.cpp \
    selectablechar.cpp \
    settingsdialog.cpp

HEADERS += \
    clickablelabel.h \
    mainwindow.h \
    selectablechar.h \
    settingsdialog.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui

win32:RC_FILE = QwikChar.rc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    glyphs.xml
