#ifndef MEDIASETTINGS_H
#define MEDIASETTINGS_H

#include <QObject>
#include <QAudioDeviceInfo>

class MediaSettings : public QObject
{
	Q_OBJECT
public:
	class Audio {
	public:
		static int getSampleRate();
		static void setSampleRate(int rate);
		static int getSampleSize();
		static void setSampleSize(int size);
		static QAudioDeviceInfo& getInputDevice();
		static void setInputDevice(QAudioDeviceInfo device);
		static QAudioDeviceInfo& getOutputDevice();
		static void setOutputDevice(QAudioDeviceInfo device);
	private:
		static int SAMPLE_RATE;
		static int SAMPLE_SIZE;
		static QAudioDeviceInfo AUDIO_INPUT_DEVICE;
		static QAudioDeviceInfo AUDIO_OUTPUT_DEVICE;
	};
private:
	explicit MediaSettings(QObject *parent = 0);
};

#endif // MEDIASETTINGS_H
