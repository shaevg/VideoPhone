#include "audiorecorder.h"
#include <QDebug>
#include "audiosettings.h"
#include "objects.h"

AudioRecorder::AudioRecorder(QObject *parent):
	QIODevice(parent),
	_qAudioInput(0),
	_bufferSize(10240),
	_emitEdge(640),
	_alive(true),
	_running(false)
{

}

AudioRecorder::~AudioRecorder(){
	if(_qAudioInput){
		if (_qAudioInput->state() == QAudio::ActiveState)
			_qAudioInput->stop();
		delete _qAudioInput;
	}
}

AudioRecorder *AudioRecorder::getInstance()
{
	return Objects::get()->_audioRecorder;
}

void AudioRecorder::startAudioRecorder()
{
	_buffer.clear();
	this->open(QIODevice::ReadWrite);
	_qAudioInput->start(this);
}

void AudioRecorder::stopAudioRecorder()
{
	this->close();
	_qAudioInput->stop();
}

void AudioRecorder::settingsChangedSlot()
{
	if (getState() == QAudio::ActiveState) {
		stopAudioRecorder();
		init();
		startAudioRecorder();
	} else {
		init();
	}
}

void AudioRecorder::settingsChanged()
{
	emit settingsChangedSignal();
}

QAudioDeviceInfo AudioRecorder::getInputDevice()
{
	return _qAudioDeviceInfo;
}

void AudioRecorder::setVolume(qreal vol)
{
	if(vol >= 0 && vol <= 1)
		_qAudioInput->setVolume(vol);
}

void AudioRecorder::init(){
	_buffer.clear();
	_qAudioFormat.setChannelCount(1);
	_qAudioFormat.setSampleRate(AudioSettings::get()->sampleRate());
	_qAudioFormat.setSampleSize(AudioSettings::get()->sampleSize());
	_qAudioFormat.setSampleType(QAudioFormat::SignedInt);
	_qAudioFormat.setByteOrder(QAudioFormat::LittleEndian);
	_qAudioFormat.setCodec("audio/pcm");

	_qAudioDeviceInfo = AudioSettings::get()->inputDevice();
	QAudioDeviceInfo info(_qAudioDeviceInfo);
	if (!info.isFormatSupported(_qAudioFormat)) {
		qDebug() << "Default format not supported - trying to use nearest";
		//_qAudioFormat = info.nearestFormat(_qAudioFormat);
	}
	if (_qAudioInput){
		_qAudioInput->stop();
		delete _qAudioInput;
	}
	_qAudioInput = new QAudioInput(_qAudioDeviceInfo, _qAudioFormat, this);
	_qAudioInput->setVolume(1);
}

qint64 AudioRecorder::readData(char *data, qint64 maxlen)
{
	Q_UNUSED(data)
	Q_UNUSED(maxlen)
	return 0;
}

QByteArray* AudioRecorder::getData(quint16 size)
{

	QByteArray* data = 0;
	if(_buffer.size() >= size) {
		//data->resize(size);
		_bufferMutex.lock();
		//memcpy(data->data(),_buffer.data(),size);
		data = new QByteArray(qCompress((uchar*)_buffer.data(),size,9));
		_buffer.remove(0,size);
		_bufferMutex.unlock();
	}
	return data;
}

bool AudioRecorder::hasData()
{
	if (!this || !_running) {
		return false;
	}
	if (_buffer.size() > _emitEdge) {
		return true;
	}
	return false;
}

void AudioRecorder::stop()
{
	_running = false;
	emit stopSignal();
}

void AudioRecorder::start()
{
	_running = true;
	emit startSignal();
}

void AudioRecorder::kill()
{
	qDebug() << "AudioRecorder: kill";
	stop();
	_bufferMutex.lock();
	_buffer.clear();
	_bufferMutex.unlock();
}

void AudioRecorder::onRun()
{
	init();
	connect(this,SIGNAL(startSignal()),this,SLOT(startAudioRecorder()));
	connect(this,SIGNAL(stopSignal()),this,SLOT(stopAudioRecorder()));
	connect(this,SIGNAL(settingsChangedSignal()),this,SLOT(settingsChangedSlot()));
}



qint64 AudioRecorder::writeData(const char *data, qint64 len)
{
	if (len) {
		_bufferMutex.lock();
		_buffer.append(data,len);
		int k = _buffer.size()-_bufferSize;
		if (k > 0) {
			_buffer.remove(0,k);
		}
		_bufferMutex.unlock();

		if (_buffer.size() >= _emitEdge)
			emit readyRead();
	}
	return len;
}

QAudio::State AudioRecorder::getState()
{
	if (_qAudioInput)
		return (_qAudioInput->state());
	return QAudio::State::StoppedState;
}




