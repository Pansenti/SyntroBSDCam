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

#ifndef SYNTROBSDCAMCONSOLE_H
#define SYNTROBSDCAMCONSOLE_H

#include <qthread.h>

class CamClient;
class VideoDriver;

class SyntroBSDCamConsole : public QThread
{
	Q_OBJECT

public:
	SyntroBSDCamConsole(bool daemonMode, QObject *parent);

public slots:
	void cameraState(QString);
	void newFrame();
	void aboutToQuit();
	void videoFormat(int width, int height, int frameRate);

protected:
	void run();
	void timerEvent(QTimerEvent *event);

private:
	bool createCamera();
	bool startVideo();
	void stopVideo();
	void showHelp();
	void showStatus();

	void runConsole();
	void runDaemon();

	void registerSigHandler();
	static void sigHandler(int sig);

	CamClient *m_client;
	VideoDriver *m_camera;

	QString m_cameraState;
	int m_frameCount;
	int m_frameRateTimer;
	double m_computedFrameRate;
	bool m_daemonMode;
	static volatile bool sigIntReceived;

	QString m_videoFormat;
	int m_width;
	int m_height;
	int m_framerate;
};

#endif // SYNTROBSDCAMCONSOLE_H

