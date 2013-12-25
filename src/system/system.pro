#-------------------------------------------------
#
# Project created by QtCreator 2013-10-20T11:38:08
#
#-------------------------------------------------

#QT       -= gui

TEMPLATE = lib
TARGET = paysages_system

DEFINES += SYSTEM_LIBRARY

include(../common.pri)

SOURCES += \
    Thread.cpp \
    Mutex.cpp \
    System.cpp \
    PackStream.cpp \
    RandomGenerator.cpp \
    Memory.cpp \
    ParallelWork.cpp \
    ParallelQueue.cpp \
    CacheFile.cpp \
    PictureWriter.cpp \
    Logs.cpp \
    ParallelPool.cpp

HEADERS += \
    system_global.h \
    Thread.h \
    Mutex.h \
    System.h \
    PackStream.h \
    RandomGenerator.h \
    Memory.h \
    ParallelWork.h \
    ParallelQueue.h \
    CacheFile.h \
    PictureWriter.h \
    Logs.h \
    ParallelPool.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
