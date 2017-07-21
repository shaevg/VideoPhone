#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <videoplayerform.h>
#include <message.h>
#include <QByteArray>
#include <QPixmap>
#include <QImage>
#include "audioplayer.h"

class VideoPlayer : public QObject
{
	Q_OBJECT
public:
	explicit VideoPlayer(bool income, QObject *parent = 0);
	~VideoPlayer();
	void kill();
	void addMessage(Message* message);
	void setForm(VideoPlayerForm* form);
	void setConnected();
signals:
	void finished();
	void stopSignal();
	void receiveMessage(Message* message);
public slots:
	void onRun();
	void stop();
	void onTimer();
	void pushMessage(Message* message);
	void setVolume(int volume);
	void startTalk();
private:
	bool _alive;
	QMutex _aliveMutex;
	QTimer* _timer;
	VideoPlayerForm* _form;
	AudioPlayer* _audioPlayer;

	bool _isIncome;
	bool _connected;

	void pushVideoFrame(QByteArray* data);
	void pushAudioFrame(QByteArray* data);
	QPixmap* getPixmapFromArray(QByteArray* data, int width, int height);
};

#endif // VIDEOPLAYER_H
