#include "audioplayer.h"
#include <QDebug>
#include "audiosettings.h"

AudioPlayer::AudioPlayer(QObject *parent) :
	QIODevice(parent),
	_qAudioOutput(0),
	_bufferSize(19000),
	_emitEdge(2500)
{
	init();
}

qint64 AudioPlayer::readData(char *data, qint64 maxlen)
{
	qint64 len = maxlen;
	if (maxlen > _buffer.size())
		len = _buffer.size();
	memcpy(data,_buffer.data(),len);
	_buffer.remove(0,len);
	return len;
}

qint64 AudioPlayer::writeData(const char *data, qint64 len)
{
	Q_UNUSED(data)
	Q_UNUSED(len)
	return 0;
}

void AudioPlayer::addData(QByteArray& data)
{
	_buffer.append(qUncompress(data));
	int k = _buffer.size()-_bufferSize;
	if (k>0)
		_buffer.remove(0,k);

	if (_buffer.size() >= _emitEdge && getState()){
		/*начать или возобновить проигывание*/
		if (this->isOpen())
			_qAudioOutput->start(this);
	}
	//qDebug()<<"OUT Buffer: "<< _buffer.size();
}

int AudioPlayer::start()
{
	this->open(QIODevice::ReadWrite);
	_qAudioOutput->start(this);
	return _qAudioOutput->state();
}

void AudioPlayer::stop()
{
	this->close();
	_qAudioOutput->stop();
}

void AudioPlayer::init()
{
	_buffer.clear();
	_qAudioFormat.setSampleRate(AudioSettings::get()->sampleRate());
	_qAudioFormat.setChannelCount(1);
	_qAudioFormat.setSampleSize(AudioSettings::get()->sampleSize());
	_qAudioFormat.setSampleType(QAudioFormat::SignedInt);
	_qAudioFormat.setByteOrder(QAudioFormat::LittleEndian);
	_qAudioFormat.setCodec("audio/pcm");

	_qAudioDeviceInfo = AudioSettings::get()->outputDevice();
	QAudioDeviceInfo info(_qAudioDeviceInfo);
	if (!info.isFormatSupported(_qAudioFormat)) {
		qDebug() << "Default format not supported - trying to use nearest";
		_qAudioFormat = info.nearestFormat(_qAudioFormat);
	}
	_qAudioOutput = new QAudioOutput(_qAudioFormat, this);
	connect(_qAudioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
}

void AudioPlayer::setVolume(qreal vol)
{
	qDebug() << "AudioPlayer::volume 1";
	try {
		if (_qAudioOutput && vol >= 0 && vol <= 1) {
			qDebug() << "AudioPlayer::volume 2";
			_qAudioOutput->setVolume(vol);
			qDebug() << "AudioPlayer::volume 3";
		}
	} catch (...) {
	}
	qDebug() << "AudioPlayer::volume 4";
}

void AudioPlayer::settingsChanged()
{
	if (getState() == QAudio::ActiveState) {
		stop();
		init();
		start();
	} else {
		init();
	}
}



QAudio::State AudioPlayer::getState()
{
	if (_qAudioOutput){
		return _qAudioOutput->state();
	}
	return QAudio::State::StoppedState;
}

QAudioDeviceInfo AudioPlayer::getOutputDevice()
{
	return _qAudioDeviceInfo;
}

int AudioPlayer::getBufferLoad() const
{
	return (int)(((double)_buffer.size()*100)/_bufferSize);
}

void AudioPlayer::handleStateChanged(QAudio::State newState)
{
	switch (newState) {
	case QAudio::IdleState:
				// Finished playing (no more data)
		qDebug() << "AudioOutput: Idle state";
		break;
	case QAudio::StoppedState:
				// Stopped for other reasons
		if (_qAudioOutput->error() != QAudio::NoError) {
					// Error handling
			qDebug() << "AudioOutput: Stopped and error";
		}
		break;
	default:
				// ... other cases as appropriate
		break;
	}
}
