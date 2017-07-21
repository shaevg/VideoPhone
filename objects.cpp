#include "objects.h"
#include <QFile>
#include <QDebug>

Objects *Objects::get()
{
	static Objects obj;
	return &obj;
}

Objects::Objects(QObject *parent) :
	QObject(parent),
	_videoRecorder(0),
	_audioRecorder(0),
	_callController(0),
	_server(0),
	_ring(0),
	_beep(0),
	_serverPort(3636)
{
	//_beep = new QByteArray();
	QFile file(":/sound/sound/ringraw.wav");

	if (file.exists()) {
		file.open(QIODevice::ReadOnly);
		_ring = new QByteArray(qCompress(file.readAll(),9));
		file.close();
	}

	file.setFileName(":/sound/sound/beepraw.wav");

	if (file.exists()) {
		file.open(QIODevice::ReadOnly);
		_beep = new QByteArray(qCompress(file.readAll(),9));
		file.close();
	}
}
