#
#  Copyright (c) 2014 Pansenti, LLC.
#
#  This file is part of Syntro
#

TEMPLATE = app

TARGET = SyntroBSDCam

DESTDIR = Output

QT += core gui network

CONFIG += release

LIBS += -lSyntroLib -lSyntroGUI
INCLUDEPATH += /usr/include/syntro /usr/include/syntro/SyntroAV

target.path = /usr/local/bin

INSTALLS += target

DEFINES += QT_NETWORK_LIB

INCLUDEPATH += GeneratedFiles

MOC_DIR += GeneratedFiles/release

OBJECTS_DIR += release  

UI_DIR += GeneratedFiles

RCC_DIR += GeneratedFiles

include(SyntroBSDCam.pri)

