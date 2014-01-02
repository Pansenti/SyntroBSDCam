//
//  Copyright (c) 2014 Pansenti, LLC.
//
//  This file is part of Syntro
//
//  Syntro is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Syntro is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Syntro.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef SYNTROBSDCAM_H
#define SYNTROBSDCAM_H

#include <QtGui>
#include "ui_syntrobsdcam.h"

#include "VideoDriver.h"
#include "CamClient.h"


#define PRODUCT_TYPE "SyntroBSDCam"


class SyntroBSDCam : public QMainWindow
{
	Q_OBJECT

public:
	SyntroBSDCam();
	~SyntroBSDCam();

signals:
	void newStream();
	void newCamera();

public slots:
	void onAbout();
	void onBasicSetup();
	void onConfigureCamera();
	void onConfigureStreams();
	void onConfigureMotion();

	void cameraState(QString state);
	void videoFormat(int width, int height, int frameRate);
	void newJPEG(QByteArray);

protected:
	void timerEvent(QTimerEvent *event);
	void closeEvent(QCloseEvent *event);

private:
	void startVideo();
	void stopVideo();
	void processFrameQueue();
	void showJPEG(QByteArray frame);
	void showImage(QImage img);
	bool createCamera();
	void clearQueue();
	void layoutStatusBar();
	void saveWindowState();
	void restoreWindowState();

	Ui::SyntroBSDCamClass ui;

	QLabel *m_cameraView;
	QLabel *m_frameRateStatus;
	QLabel *m_controlStatus;

	CamClient *m_client;
 	VideoDriver *m_camera;
	QString m_cameraState;
	QMutex m_frameQMutex;
	QQueue <QByteArray> m_jpegFrameQ;

	int m_frameRateTimer;
	int m_frameRefreshTimer;
	int m_frameCount;
	QSize m_imgSize;
};

#endif // SYNTROBSDCAM_H

