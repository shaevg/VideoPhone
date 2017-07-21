#ifndef AUDIOSETTINGS_H
#define AUDIOSETTINGS_H

#include <QObject>
#include <QAudioDeviceInfo>

class AudioSettings : public QObject
{
	Q_OBJECT
public:
	static AudioSettings* get();
	int sampleRate();
	int sampleSize();
	QAudioDeviceInfo &outputDevice();
	QAudioDeviceInfo &inputDevice();
	void setSampleRate(int srate);
	void setSampleSize(int ssize);
	void setOutputDevice(QAudioDeviceInfo device);
	void setInputDevice(QAudioDeviceInfo device);

	int volume() const;
	void setVolume(int volume);

private:
	int _sampleRate;
	int _sampleSize;
	int _volume;
	QAudioDeviceInfo _outputDevice;
	QAudioDeviceInfo _inputDevice;
	explicit AudioSettings(QObject *parent = 0);
	AudioSettings(const AudioSettings&) = delete;
	AudioSettings& operator=(const AudioSettings&) = delete;
};

#endif // AUDIOSETTINGS_H
