#ifndef OBJECTS_H
#define OBJECTS_H

#include <QObject>
#include <callcontroller.h>
#include <videorecorder.h>
#include <server.h>

class Objects : public QObject
{
	Q_OBJECT
public:
	static Objects* get();
	VideoRecorder* _videoRecorder;
	AudioRecorder* _audioRecorder;
	CallController* _callController;
	Server* _server;
	QByteArray* _ring;
	QByteArray* _beep;
	int _serverPort;
signals:

public slots:
private:
	explicit Objects(QObject *parent = 0);
};

#endif // OBJECTS_H
