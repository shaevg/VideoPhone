#include "mediasettings.h"
#include <QDebug>
#include "audiorecorder.h"

int MediaSettings::Audio::SAMPLE_RATE = 16000;
int MediaSettings::Audio::SAMPLE_SIZE = 16;
QAudioDeviceInfo MediaSettings::Audio::AUDIO_INPUT_DEVICE = QAudioDeviceInfo::defaultInputDevice();
QAudioDeviceInfo MediaSettings::Audio::AUDIO_OUTPUT_DEVICE = QAudioDeviceInfo::defaultOutputDevice();

MediaSettings::MediaSettings(QObject *parent) : QObject(parent)
{

}

int MediaSettings::Audio::getSampleRate()
{
	return SAMPLE_RATE;
}

void MediaSettings::Audio::setSampleRate(int rate)
{
	SAMPLE_RATE = rate;
}

int MediaSettings::Audio::getSampleSize()
{
	return SAMPLE_SIZE;
}

void MediaSettings::Audio::setSampleSize(int size)
{
	SAMPLE_SIZE = size;
}

QAudioDeviceInfo &MediaSettings::Audio::getInputDevice()
{
	return AUDIO_INPUT_DEVICE;
}

void MediaSettings::Audio::setInputDevice(QAudioDeviceInfo device)
{
	AUDIO_INPUT_DEVICE = device;
	AudioRecorder::getInstance()->settingsChanged();
}

QAudioDeviceInfo &MediaSettings::Audio::getOutputDevice()
{
	return AUDIO_OUTPUT_DEVICE;
}

void MediaSettings::Audio::setOutputDevice(QAudioDeviceInfo device)
{
	AUDIO_OUTPUT_DEVICE = device;
}


