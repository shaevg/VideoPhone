#include "videorecorder.h"
#include <QDebug>
#include <QImage>
#include <QThread>
#include <chrono>
#include "objects.h"

#include "videosettings.h"

VideoRecorder::VideoRecorder(QObject *parent) : QIODevice(parent)
{
	_alive = true;
	_qualityParams = std::vector<int>(2);              // Вектор параметров качества сжатия
	_qualityParams[0] = CV_IMWRITE_JPEG_QUALITY; // Кодек JPEG
	_qualityParams[1] = VideoSettings::get()->jpegQuality();
	_period = VideoSettings::get()->period();
}

VideoRecorder::~VideoRecorder()
{
	_alive = false;
}

VideoRecorder *VideoRecorder::getInstance()
{
	return Objects::get()->_videoRecorder;
}

int VideoRecorder::start()
{
	int result = 0;
	if (!this->isRunning()) {
		_runVideoRecorder = true;
	}
	return result;
}

void VideoRecorder::stop() {
	if (this->isRunning()) {
		_runVideoRecorder = false;
	}
}

int VideoRecorder::startRecording()
{
	stopRecording();
	_qualityParams[1] = VideoSettings::get()->jpegQuality();

	_camera.open(VideoSettings::get()->device());
	if (!_camera.isOpened()) {
		qDebug()<<"Camera not opened!";
		return 1;
	}

	// check if we succeeded
	_camera.set(CV_CAP_PROP_FRAME_WIDTH, VideoSettings::get()->width());
	_camera.set(CV_CAP_PROP_FRAME_HEIGHT, VideoSettings::get()->height());

	_camera >> _frame;
	if (_frame.empty()) {
		qDebug() << "ERROR! blank frame grabbed";
		return 1;
	}

	this->open(QIODevice::ReadWrite);
	return 0;
}

void VideoRecorder::stopRecording()
{
	_camera.release();
	this->close();
}

void VideoRecorder::readFrame()
{
	auto _start = std::chrono::steady_clock::now();
	if (_settingsChanged) {
		_settingsChanged = false;
		stopRecording();
		startRecording();
	}
	_camera >> _frame;

	_imageJpegMutex.lock();
	_imageJpeg.clear();
	cv::imencode(".jpg", _frame, _imageJpeg, _qualityParams);
	_imageJpegMutex.unlock();

	_imageFrameMutex.lock();
	cv::cvtColor(_frame, _imageFrame, CV_BGR2RGB);
	_imageFrameMutex.unlock();

	/*
	if (_encoder.encodeMat(frame, imgBuf)) {
		imgBuf.insert(imgBuf.begin(),0x02);
		socket->writeDatagram((const char*)imgBuf.data(),imgBuf.size(), _addr, 3636);
	}*/

	auto _stop = std::chrono::steady_clock::now();
	auto _periodCur = std::chrono::duration_cast<std::chrono::milliseconds>(_stop - _start);
	//qDebug() << "Period: " << _period.count() << " mks";
	emit readyRead();

	int t = _periodCur.count();
	if (t < _period && t > 0) {
		this->thread()->msleep(_period - t);
	}
}

void VideoRecorder::periodChanged()
{
	_period = VideoSettings::get()->period();
}

QByteArray *VideoRecorder::getJpegFrame()
{
	QByteArray* frame = 0;
	try {
		_imageJpegMutex.lock();
		if (_imageJpeg.size() > 0) {
			frame = new QByteArray((char*)_imageJpeg.data(),_imageJpeg.size());
		}
	} catch (...) {}
	try{
		_imageJpegMutex.unlock();
	} catch(...) {}

	return frame;
}

QByteArray *VideoRecorder::getHevcFrame()
{
	QByteArray* frame = 0;

	if (_imageHevc.size() > 0) {
		frame = new QByteArray((char*)_imageHevc.data(),_imageHevc.size());
	}

	return frame;
}

QPixmap *VideoRecorder::getPixmapFrame(int width, int height)
{
	_imageFrameMutex.lock();
	int nWidth =  (height*_imageFrame.cols)/_imageFrame.rows;
	int nHeigth = (width*_imageFrame.rows)/_imageFrame.cols;

	if (nHeigth > height) {
		nHeigth = height;
	}
	if (nWidth > width) {
		nWidth = width;
	}
	cv::resize(_imageFrame,_imageFrame,cv::Size(nWidth,nHeigth));
	QImage img((uchar*)_imageFrame.data,_imageFrame.cols,_imageFrame.rows,_imageFrame.step,QImage::Format_RGB888);
	_imageFrameMutex.unlock();
	QPixmap* pic = new QPixmap(QPixmap::fromImage(img));
	return pic;
}

void VideoRecorder::settingsChanged()
{
	if (isRunning()) {
		_settingsChanged = true;
	}
}

void VideoRecorder::jpegQualityChanged()
{
	_qualityParams[1] = VideoSettings::get()->jpegQuality();
}

bool VideoRecorder::isRunning()
{
	return this->isOpen();
}

void VideoRecorder::kill()
{
	_alive = false;
	_runVideoRecorder = false;
	stopRecording();
	QThread::msleep(100);
}

qint64 VideoRecorder::readData(char *data, qint64 maxlen)
{
	Q_UNUSED(data)
	Q_UNUSED(maxlen)
	return 0;
}

qint64 VideoRecorder::writeData(const char *data, qint64 len)
{
	Q_UNUSED(data)
	Q_UNUSED(len)
	return 0;
}

void VideoRecorder::onRun()
{
	_settingsChanged = false;
	_runVideoRecorder = false;
	while (_alive) {
		if (isRunning()) {
			if (_runVideoRecorder){
				readFrame();
			} else {
				stopRecording();
			}
		} else {
			if (_runVideoRecorder) {
				startRecording();
			}
		}
	}
}

