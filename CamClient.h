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

#ifndef CAMCLIENT_H
#define CAMCLIENT_H

#include "ChangeDetector.h"

#include <qimage.h>
#include <qmutex.h>

#include "SyntroLib.h"


//  Settings keys


#define CAMERA_GROUP       "Camera"

#define	CAMERA_CAMERA      "Device"
#define	CAMERA_WIDTH       "Width"
#define	CAMERA_HEIGHT      "Height"
#define	CAMERA_FRAMERATE   "FrameRate"
#define CAMERA_FORMAT      "Format"

#define CAMERA_DEFAULT_DEVICE "<default device>"


#define	CAMCLIENT_STREAM_GROUP                "Stream"

#define	CAMCLIENT_HIGHRATEVIDEO_MININTERVAL   "HighRateMinInterval"
#define	CAMCLIENT_HIGHRATEVIDEO_MAXINTERVAL   "HighRateMaxInterval"
#define	CAMCLIENT_HIGHRATEVIDEO_NULLINTERVAL  "HighRateNullInterval"
#define CAMCLIENT_GENERATE_LOWRATE            "GenerateLowRate"
#define CAMCLIENT_LOWRATE_HALFRES             "LowRateHalfRes"
#define CAMCLIENT_LOWRATEVIDEO_MININTERVAL    "LowRateMinInterval"
#define CAMCLIENT_LOWRATEVIDEO_MAXINTERVAL    "LowRateMaxInterval"
#define CAMCLIENT_LOWRATEVIDEO_NULLINTERVAL   "LowRateNullInterval"


#define	CAMCLIENT_MOTION_GROUP           "Motion"

#define CAMCLIENT_MOTION_TILESTOSKIP     "MotionTilesToSkip"
#define CAMCLIENT_MOTION_INTERVALSTOSKIP "MotionIntervalsToSkip"
#define	CAMCLIENT_MOTION_MIN_DELTA       "MotionMinDelta"
#define	CAMCLIENT_MOTION_MIN_NOISE       "MotionMinNoise"

// interval between frames checked for deltas in mS. 0 means never check - always send image

#define CAMCLIENT_MOTION_DELTA_INTERVAL  "MotionDeltaInterval"

//  length of preroll. 0 turns off the feature

#define CAMCLIENT_MOTION_PREROLL         "MotionPreroll"

// length of postroll. 0 turns off the feature

#define CAMCLIENT_MOTION_POSTROLL        "MotionPostroll"

// maximum rate - 120 per second (allows for 4x rate during preroll send)

#define	CAMERA_IMAGE_INTERVAL	((qint64)SYNTRO_CLOCKS_PER_SEC/120)

#define CAMCLIENT_AV_TYPE_MJPPCM     0               // MJPEG + PCM


typedef struct
{
	QByteArray data;
	qint64 timestamp;
	int param;
} PREROLL;

typedef struct
{
	QByteArray data;
	qint64 timestamp;
} CLIENT_QUEUEDATA;


// These defines are for the motion sequence state machine

#define CAMCLIENT_STATE_IDLE         0               // waiting for a motion event
#define CAMCLIENT_STATE_PREROLL      1               // sending preroll saved frames from the queue
#define CAMCLIENT_STATE_INSEQUENCE   2               // sending frames as normal during motion sequence
#define CAMCLIENT_STATE_POSTROLL     3               // sending the postroll frames
#define CAMCLIENT_STATE_CONTINUOUS   4               // no motion detect so continuous sending

class CamClient : public Endpoint
{
	Q_OBJECT

public:
	CamClient(QObject *parent);
	int getFrameCount();

public slots:
	void newStream();
	void newJPEG(QByteArray);
	void videoFormat(int width, int height, int framerate);

protected:
	void appClientInit();
	void appClientExit();
	void appClientConnected();
	void appClientBackground();
	int m_avmuxPortHighRate;
	int m_avmuxPortLowRate;

private:
	void processAVQueueMJPPCM();
	void sendHeartbeatFrameMJPPCM(qint64 now, const QByteArray& jpeg);
	bool sendAVMJPPCM(qint64 now, int param, bool checkMotion);
	void sendNullFrameMJPPCM(qint64 now, bool highRate);
	void sendPrerollMJPPCM(bool highRate);
	void halfRes(QByteArray& jpeg);

	bool m_generateLowRate;
	bool m_lowRateHalfRes;

	void checkForMotion(qint64 now, QByteArray& jpeg);
	bool dequeueVideoFrame(QByteArray& videoData, qint64& timestamp);
	void clearVideoQueue();
	void clearQueues();
	void ageOutPrerollQueues(qint64 now);

	qint64 m_lastChangeTime;
	bool m_imageChanged; 

	qint64 m_lastFrameTime;                  // last time any frame was sent - null or full
	qint64 m_lastLowRateFrameTime;           // last time any frame was sent - null or full - on low rate
	qint64 m_lastPrerollFrameTime;           // last time a frame was added to the preroll
	qint64 m_lastLowRatePrerollFrameTime;    // last time a low rate frame was added to the preroll
	qint64 m_lastFullFrameTime;              // last time a full frame was sent
	qint64 m_lastLowRateFullFrameTime;       // last time a full frame was sent on low rate
 
	qint64 m_highRateMinInterval;            // min interval between high rate frames
	qint64 m_highRateMaxInterval;            // max interval between full frame refreshes
	qint64 m_highRateNullInterval;           // max interval between null or real frames

	qint64 m_lowRateMinInterval;             // min interval between high rate frames
	qint64 m_lowRateMaxInterval;             // max interval between full frame refreshes
	qint64 m_lowRateNullInterval;            // max interval between null or real frames

	int m_minDelta;
	int m_minNoise;
	qint64 m_deltaInterval;                                 // interval between frames checked for motion
	qint64 m_preroll;                                       // length in mS of preroll
	qint64 m_postroll;                                      // length in mS of postroll

	int m_tilesToSkip;                                      // number of tiles in an interval to skip
	int m_intervalsToSkip;                                  // number of intervals to skip

	int m_sequenceState;                                    // the state of the motion sequence state machine
	qint64 m_postrollStart;                                 // time that the postroll started

	QQueue<PREROLL *> m_videoPrerollQueue;                  // queue of PREROLL structures for the video preroll queue
	QQueue<PREROLL *> m_videoLowRatePrerollQueue;           // queue of PREROLL structures for the low rate video preroll queue

	qint64 m_lastDeltaTime;                                 // when the delta was last checked

	QQueue <CLIENT_QUEUEDATA *> m_videoFrameQ;
	QMutex m_videoQMutex;

	ChangeDetector m_cd;                                    // the change detector instance

	int m_frameCount;
	QMutex m_frameCountLock;

	int m_recordIndex;                                      // increments for every avmux record constructed

	SYNTRO_AVPARAMS m_avParams;                             // used to hold stream parameters

	bool m_gotVideoFormat;
};

#endif // CAMCLIENT_H

