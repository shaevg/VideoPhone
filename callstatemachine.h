#ifndef CALLSTATEMACHINE_H
#define CALLSTATEMACHINE_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include "message.h"
#include "server.h"
#include "audioplayer.h"
#include "videoplayer.h"
#include "videoplayerform.h"

class CallStateMachine : public QObject
{
	Q_OBJECT
public:
	enum CallState {START = 0, IN = 1, OUT = 2, TALK = 3, FINISH = 4, CANCELS = 5, ECHO = 6};
	explicit CallStateMachine(const QHostAddress& host, const quint16& port, CallState state = START, QObject *parent = 0);
	~CallStateMachine();
	void pushMessage(Message* message);
	void sendAudioData(QByteArray* data);
	void sendVideoData(QByteArray* data);
	CallState getState();
	QHostAddress getHost() const;
	quint16 getPort() const;
	bool isConnected() const;
signals:
	void userOnline(const bool& isOnline,const QHostAddress& host);
	void statusChanged();
	void startTalk();
public slots:
	void acceptCall();
	void rejectCall();
	void onTimer();
	void videoFormClosed();
private:
	CallState _state;

	QHostAddress _host;
	quint16 _port;

	QTimer _timer;

	bool _connection;
	int _timerShots;
	VideoPlayer* _videoPlayer;
	VideoPlayerForm* _videoPlayerForm;
	QThread _videoThread;

	void goToState(CallState state);
	void receiveControlMessage(Message* message);
	void receiveMediaMessage(Message* message);
};

#endif // CALLSTATEMACHINE_H
