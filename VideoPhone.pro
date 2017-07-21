#-------------------------------------------------
#
# Project created by QtCreator 2016-11-14T13:54:09
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VideoPhone
TEMPLATE = app


SOURCES += main.cpp\
	mainwindow.cpp \
    addcontact.cpp \
    audioplayer.cpp \
    audiorecorder.cpp \
    callcontroller.cpp \
    callstatemachine.cpp \
    message.cpp \
    server.cpp \
    audiosettings.cpp \
    videorecorder.cpp \
    videosettings.cpp \
    objects.cpp \
    videoplayer.cpp \
    videoplayerform.cpp

HEADERS  += mainwindow.h \
    addcontact.h \
    defines.h \
    audioplayer.h \
    audiorecorder.h \
    callcontroller.h \
    callstatemachine.h \
    message.h \
    server.h \
    audiosettings.h \
    videorecorder.h \
    videosettings.h \
    objects.h \
    videoplayer.h \
    videoplayerform.h

FORMS    += mainwindow.ui \
    addcontact.ui \
    videoplayerform.ui

INCLUDEPATH += "D:\WorkStage\OpenCV\build\install\include" \

LIBS += -L"D:\\WorkStage\\OpenCV\\build\\install\\x86\\mingw\\lib" \
-lopencv_core320 \
-lopencv_imgproc320 \
-lopencv_imgcodecs320 \
-lopencv_video320 \
-lopencv_videoio320

RESOURCES += \
    resources.qrc

win32:RC_FILE = resource.rc
