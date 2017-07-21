#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QIODevice>
#include <QAudioOutput>
#include <QByteArray>

class AudioPlayer : public QIODevice
{
	Q_OBJECT
private:
	/*Необходимые для воспроизведения звука вещи*/
	QAudioOutput* _qAudioOutput;
	QAudioDeviceInfo _qAudioDeviceInfo;
	QAudioFormat _qAudioFormat;

	/*Буффер _buffer и его настройки*/
	QByteArray _buffer;        /*Буфер для хранения записи перед отправкой*/
	quint16 _bufferSize; /*предельный размер буффера _buffer*/
	quint16 _emitEdge;   /*объём буфера при котором происходит событие QIODevice::ReadyRead*/

public:
	explicit AudioPlayer(QObject *parent = 0);
	qint64 readData(char *data, qint64 maxlen);
	qint64 writeData(const char *data, qint64 len); /*Не используется*/

	void addData(QByteArray &data);

	int start();
	void stop();
	void init();
	void setVolume(qreal vol);
	void settingsChanged();
	QAudio::State getState();
	QAudioDeviceInfo getOutputDevice();
	int getBufferLoad() const;
signals:

public slots:
	void handleStateChanged(QAudio::State newState);
};

#endif // AUDIOPLAYER_H
