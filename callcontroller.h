#ifndef CALLCONTROLLER_H
#define CALLCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QThread>
#include <QMutex>
#include "server.h"
#include "message.h"
#include "callstatemachine.h"
#include "audiorecorder.h"
#include "audioplayer.h"
#include "videorecorder.h"

class CallController : public QObject
{
	Q_OBJECT
public:
	CallController(QObject *parent = 0);
	~CallController();

	static CallController* getInstance();
	void checkUserAsync(const QHostAddress& host);
	bool createCall(const QHostAddress& host);
	void rejectCall(const QHostAddress& host);
	void acceptCall(const QHostAddress& host);
	void killApplication();
	void setVideoRecorderEnable(bool videoRecorderEnable);
	void setAudioRecorderEnable(bool audioRecorderEnable);

signals:
	void userOnline(bool isOnline, QHostAddress host);
public slots:
	void newMessage(Message* message);
	void newUnknownMessage(Message* message);
private slots:
	void userOnlineFromMachine(const bool& isOnline,const QHostAddress& host);
	void readAudioRecord();
	void readVideoFrame();
	void checkConnections();
private:
	typedef QMap<quint32, CallStateMachine*> CallMap;
	CallMap _connections;
	QMutex _connectionsMutex;
	void newcomerControlMessage(Message* message);
	void newcomerMediaMessage(Message* message);
	AudioRecorder* _audioRecorder;
	VideoRecorder* _videoRecorder;
	Server* _server;
	QThread* _videoThread;
	QThread* _audioThread;
	QThread* _serverThread;
	bool _videoRecorderEnable;
	bool _audioRecorderEnable;
	QMutex _pushMutex;
};

#endif // CALLCONTROLLER_H
