#ifndef VIDEOSETTINGS_H
#define VIDEOSETTINGS_H

#include <QObject>

class VideoSettings : public QObject
{
	Q_OBJECT
public:
	static VideoSettings* get();
	int width() const;
	int height() const;
	int hevcBitRate() const;
	quint8 jpegQuality() const;
	quint8 device() const;
	int period() const;

	void setSize(int w, int h);
	void setHevcBitRate(int bitRate);
	void setJpegQuality(quint8 quality);
	void setDevice(quint8 d);
	void setPeriod(int period);

private:
	int _height;
	int _width;
	int _period;
	quint8 _device;
	quint8 _jpegQuality;
	int _hevcBitRate;
	explicit VideoSettings(QObject *parent = 0);
	VideoSettings(const VideoSettings&) = delete;
	VideoSettings& operator=(const VideoSettings&) = delete;
};

#endif // VIDEOSETTINGS_H
