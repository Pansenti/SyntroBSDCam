#
#  Copyright (c) 2014 Pansenti, LLC.
#
#  This file is part of Syntro
#

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += SyntroBSDCam.h \
        SyntroBSDCamConsole.h \
	VideoDriver.h \
	CamClient.h \	
	StreamsDlg.h \
        CameraDlg.h \
        MotionDlg.h

SOURCES += main.cpp \
        SyntroBSDCam.cpp \
        SyntroBSDCamConsole.cpp \
	VideoDriver.cpp \
	CamClient.cpp \
	StreamsDlg.cpp \
        CameraDlg.cpp \
        MotionDlg.cpp

FORMS += syntrobsdcam.ui

