#include "audiosettings.h"
#include "audiorecorder.h"

AudioSettings *AudioSettings::get()
{
	static AudioSettings settings;
	return &settings;
}

AudioSettings::AudioSettings(QObject *parent) :
	QObject(parent),
	_sampleRate(8000),
	_sampleSize(16),
	_outputDevice(QAudioDeviceInfo::defaultOutputDevice()),
	_inputDevice(QAudioDeviceInfo::defaultInputDevice())
{

}

int AudioSettings::sampleRate()
{
	return _sampleRate;
}

int AudioSettings::sampleSize()
{
	return _sampleSize;
}

QAudioDeviceInfo& AudioSettings::outputDevice()
{
	return _outputDevice;
}

QAudioDeviceInfo& AudioSettings::inputDevice()
{
	return _inputDevice;
}

void AudioSettings::setSampleRate(int srate)
{
	_sampleRate = srate;
	if (AudioRecorder::getInstance()){
		AudioRecorder::getInstance()->settingsChanged();
	}
}

void AudioSettings::setSampleSize(int ssize)
{
	_sampleSize = ssize;
	if (AudioRecorder::getInstance()){
		AudioRecorder::getInstance()->settingsChanged();
	}
}

void AudioSettings::setOutputDevice(QAudioDeviceInfo device)
{
	_outputDevice = device;
}

void AudioSettings::setInputDevice(QAudioDeviceInfo device)
{
	_inputDevice = device;
	if (AudioRecorder::getInstance()){
		AudioRecorder::getInstance()->settingsChanged();
	}
}

int AudioSettings::volume() const
{
	return _volume;
}

void AudioSettings::setVolume(int volume)
{
	if (volume >= 0 && volume <=100) {
		_volume = volume;

		if (AudioRecorder::getInstance()){
			AudioRecorder::getInstance()->setVolume(((double)_volume)/100);
		}
	}
}


