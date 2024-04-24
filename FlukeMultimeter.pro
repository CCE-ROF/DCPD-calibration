QT       += core gui xml serialport network sql printsupport charts qml #xlsx

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

DESTDIR = $$PWD/bin

include(../VFTCommon/CF_VFTCommon.pri)

INCLUDEPATH += $$PWD/../VFTCommon/

SOURCES += \
    CalCheckInfo.cpp \
    CommunicationModule.cpp \
    FlukeHelper.cpp \
    PageAbout.cpp \
    PageFactoryCalibrationSV.cpp \
    PageSetupApp.cpp \
    PageCheck.cpp \
    PageReport.cpp \
    PageRun.cpp \
    PageSetupDV.cpp \
    PageSetupSV.cpp \
    Reports.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    CalCheckInfo.h \
    CommunicationModule.h \
    FlukeHelper.h \
    MainWindow.h \
    PageAbout.h \
    PageFactoryCalibrationSV.h \
    PageSetupApp.h \
    PageCheck.h \
    PageReport.h \
    PageRun.h \
    PageSetupDV.h \
    PageSetupSV.h \
    Reports.h

FORMS += \
    CalCheckInfo.ui \
    MainWindow.ui \
    PageAbout.ui \
    PageFactoryCalibrationSV.ui \
    PageSetupApp.ui \
    PageCheck.ui \
    PageReport.ui \
    PageRun.ui \
    PageSetupDV.ui \
    PageSetupSV.ui

RC_FILE = FlukeMultimeter.rc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32 {
    MOC_DIR = tmp-win32
    UI_DIR = tmp-win32
    UI_HEADERS_DIR = tmp-win32
    UI_SOURCES_DIR = tmp-win32
    OBJECTS_DIR = tmp-win32
    RCC_DIR = tmp-win32
}

unix:!macx {
    MOC_DIR = tmp-lin64
    UI_DIR = tmp-lin64
    UI_HEADERS_DIR = tmp-lin64
    UI_SOURCES_DIR = tmp-lin64
    OBJECTS_DIR = tmp-lin64
    RCC_DIR = tmp-lin64
}

#----------------------------------------------------------------------------

DEFINES += QTRPT_LIBRARY

unix:!macx|win32: LIBS += -L$$PWD/bin/ -lQtRPT

INCLUDEPATH += $$PWD/../ThirdParty/QtRPT/QtRPT
DEPENDPATH += $$PWD/../ThirdParty/QtRPT/QtRPT

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/bin/lib/QtRPT.lib
else:unix:!macx|win32-g++: PRE_TARGETDEPS += $$PWD/bin/lib/libQtRPT.a

#----------------------------------------------------------------------------

unix:!macx|win32: LIBS += -L$$PWD/bin/ -lQtXlsx

INCLUDEPATH += $$PWD/../ThirdParty/QtRPT/3rdparty/QtXlsx
DEPENDPATH += $$PWD/../ThirdParty/QtRPT/3rdparty/QtXlsx

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/bin/lib/QtXlsx.lib
else:unix:!macx|win32-g++: PRE_TARGETDEPS += $$PWD/bin/lib/libQtXlsx.a

RESOURCES += \
    images.qrc
