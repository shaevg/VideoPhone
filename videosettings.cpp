#include "videosettings.h"
#include "videorecorder.h"

VideoSettings::VideoSettings(QObject *parent) :
	QObject(parent),
	_height(480),
	_width(640),
	_period(50),
	_device(0),
	_jpegQuality(15),
	_hevcBitRate(400000)
{
}

VideoSettings *VideoSettings::get()
{
	static VideoSettings settings;
	return &settings;
}

int VideoSettings::width() const
{
	return _width;
}

int VideoSettings::height() const
{
	return _height;
}

int VideoSettings::hevcBitRate() const
{
	return _hevcBitRate;
}

quint8 VideoSettings::jpegQuality() const
{
	return _jpegQuality;
}

quint8 VideoSettings::device() const
{
	return _device;
}

int VideoSettings::period() const
{
	return _period;
}

void VideoSettings::setSize(int w, int h)
{
	_width = w;
	_height = h;
	if (VideoRecorder::getInstance()) {
		VideoRecorder::getInstance()->settingsChanged();
	}
}

void VideoSettings::setHevcBitRate(int bitRate)
{
	_hevcBitRate = bitRate;
	if (VideoRecorder::getInstance()) {
		VideoRecorder::getInstance()->settingsChanged();
	}
}

void VideoSettings::setJpegQuality(quint8 quality)
{
	_jpegQuality = quality;
	if (VideoRecorder::getInstance()) {
		VideoRecorder::getInstance()->jpegQualityChanged();
	}
}

void VideoSettings::setDevice(quint8 d)
{
	_device = d;
	if (VideoRecorder::getInstance()) {
		VideoRecorder::getInstance()->settingsChanged();
	}
}

void VideoSettings::setPeriod(int period)
{
	_period = period;
	if (VideoRecorder::getInstance()) {
		VideoRecorder::getInstance()->periodChanged();
	}
}
