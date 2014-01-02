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

#include "SyntroBSDCamConsole.h"
#include "SyntroBSDCam.h"

#include <termios.h>
#include <unistd.h>
#include <signal.h>

#include "VideoDriver.h"

#define FRAME_RATE_TIMER_INTERVAL 3

volatile bool SyntroBSDCamConsole::sigIntReceived = false;

SyntroBSDCamConsole::SyntroBSDCamConsole(bool daemonMode, QObject *parent)
	: QThread(parent)
{
	m_daemonMode = daemonMode;

	m_computedFrameRate = 0.0;
	m_frameCount = 0;
	m_frameRateTimer = 0;
	m_camera = NULL;

	m_client = NULL;

	m_width = 0;
	m_height = 0;
	m_framerate = 0;

	if (m_daemonMode) {
		registerSigHandler();
		
		if (daemon(1, 1)) {
			perror("daemon");
			return;
		}
	}

	connect((QCoreApplication *)parent, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));

	SyntroUtils::syntroAppInit();

	m_client = new CamClient(this);
	m_client->resumeThread();

	if (!m_daemonMode) {
		m_frameCount = 0;
		m_frameRateTimer = startTimer(FRAME_RATE_TIMER_INTERVAL * 1000);
	}

	start();
}

void SyntroBSDCamConsole::aboutToQuit()
{
	if (m_frameRateTimer) {
		killTimer(m_frameRateTimer);
		m_frameRateTimer = 0;
	}

	for (int i = 0; i < 5; i++) {
		if (wait(1000))
			break;

		if (!m_daemonMode)
			printf("Waiting for console thread to finish...\n");
	}
}

bool SyntroBSDCamConsole::createCamera()
{
	if (m_camera) {
		delete m_camera;
		m_camera = NULL;
	}

	m_camera = new VideoDriver();

	if (!m_camera)
		return false;

	return true;
}

bool SyntroBSDCamConsole::startVideo()
{
	if (!m_camera) {
		if (!createCamera()) {
			appLogError("Error allocating camera");
			return false;
		}
	}

	if (!m_daemonMode) {
		connect(m_camera, SIGNAL(cameraState(QString)), this, SLOT(cameraState(QString)), Qt::DirectConnection);
		connect(m_camera, SIGNAL(newFrame()), this, SLOT(newFrame()), Qt::DirectConnection);
	}

	connect(m_camera, SIGNAL(videoFormat(int,int,int)), this, SLOT(videoFormat(int,int,int)));
	connect(m_camera, SIGNAL(videoFormat(int,int,int)), m_client, SLOT(videoFormat(int,int,int)));

	connect(m_camera, SIGNAL(newJPEG(QByteArray)), m_client, SLOT(newJPEG(QByteArray)), Qt::DirectConnection);

	m_camera->resumeThread();

	return true;
}

void SyntroBSDCamConsole::stopVideo()
{
    if (m_camera) {
        disconnect(m_camera, SIGNAL(newJPEG(QByteArray)), m_client, SLOT(newJPEG(QByteArray)));

        disconnect(m_camera, SIGNAL(cameraState(QString)), this, SLOT(cameraState(QString)));
        disconnect(m_camera, SIGNAL(videoFormat(int,int,int)), this, SLOT(videoFormat(int,int,int)));
        disconnect(m_camera, SIGNAL(videoFormat(int,int,int)), m_client, SLOT(videoFormat(int,int,int)));

        m_camera->exitThread();
        m_camera = NULL;
    }
}

void SyntroBSDCamConsole::videoFormat(int width, int height, int framerate)
{
	m_width = width;
	m_height = height;
	m_framerate = framerate;
}

void SyntroBSDCamConsole::cameraState(QString state)
{
	m_cameraState = state;
}

void SyntroBSDCamConsole::newFrame()
{
	m_frameCount++;
}

void SyntroBSDCamConsole::timerEvent(QTimerEvent *)
{
	m_computedFrameRate =  (double)m_frameCount / (double)FRAME_RATE_TIMER_INTERVAL;
	m_frameCount = 0;
}

void SyntroBSDCamConsole::showHelp()
{
	printf("\nOptions are:\n\n");
	printf("  h - Show help\n");
	printf("  s - Show status\n");
	printf("  x - Exit\n");
}

void SyntroBSDCamConsole::showStatus()
{    
	printf("\nStatus: %s\n", qPrintable(m_client->getLinkState()));

	if (m_cameraState == "Running")
	        printf("Measured frame rate is    : %f fps\n", m_computedFrameRate);
	else
		printf("Camera state: %s\n", qPrintable(m_cameraState));

	printf("Frame format is  : %s\n", qPrintable(m_videoFormat));
	printf("Frame size is    : %d x %d\n", m_width, m_height);
	printf("Frame rate is    : %d\n", m_framerate);
}

void SyntroBSDCamConsole::run()
{
	if (m_daemonMode)
		runDaemon();
	else
		runConsole();

	stopVideo();

	m_client->exitThread();

	SyntroUtils::syntroAppExit();

	QCoreApplication::exit();
}

void SyntroBSDCamConsole::runConsole()
{
	struct termios	ctty;

	tcgetattr(fileno(stdout), &ctty);
	ctty.c_lflag &= ~(ICANON);
	tcsetattr(fileno(stdout), TCSANOW, &ctty);

	bool grabbing = startVideo();

	while (grabbing) {
		printf("\nEnter option: ");

	        switch (tolower(getchar()))
		{
		case 'h':
			showHelp();
			break;

		case 's':
			showStatus();
			break;

		case 'x':
			printf("\nExiting\n");
			grabbing = false;		
			break;

		case '\n':
			continue;
		}
	}
}

void SyntroBSDCamConsole::runDaemon()
{
	startVideo();

	while (!SyntroBSDCamConsole::sigIntReceived)
		msleep(100); 
}

void SyntroBSDCamConsole::registerSigHandler()
{
	struct sigaction sia;

	bzero(&sia, sizeof sia);
	sia.sa_handler = SyntroBSDCamConsole::sigHandler;

	if (sigaction(SIGINT, &sia, NULL) < 0)
		perror("sigaction(SIGINT)");
}

void SyntroBSDCamConsole::sigHandler(int)
{
	SyntroBSDCamConsole::sigIntReceived = true;
}

