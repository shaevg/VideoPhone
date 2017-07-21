#include "videoplayer.h"
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <vector>
#include <objects.h>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

VideoPlayer::VideoPlayer(bool income, QObject *parent) :
	QObject(parent),
	_timer(0),
	_form(0),
	_audioPlayer(0),
	_isIncome(income),
	_connected(false)
{
	_aliveMutex.lock();
	_alive = true;
	_aliveMutex.unlock();
}

VideoPlayer::~VideoPlayer()
{
	qDebug() << "~VideoPlayer: destuctor";
	if (_timer) {
		delete _timer;
	}

	if (_audioPlayer) {
		_audioPlayer->stop();
		_audioPlayer->deleteLater();
		_audioPlayer = 0;
	}
	_form = 0;
}

void VideoPlayer::kill()
{
	emit stopSignal();
}

void VideoPlayer::addMessage(Message *message)
{
	emit receiveMessage(message);
}

void VideoPlayer::setForm(VideoPlayerForm *form)
{
	_form = form;
}

void VideoPlayer::setConnected()
{
	_connected = true;
}

void VideoPlayer::pushMessage(Message *message)
{
	if (message->type() != Message::MEDIA) {
		delete message;
		return;
	}

	switch (message->subType()) {
	case SubType::AUDIO: {
		pushAudioFrame(message->data());
		break;
	}
	case SubType::VIDEO: {
		pushVideoFrame(message->data());
		break;
	}
	}

	delete message;
}

void VideoPlayer::setVolume(int volume)
{
	if (_audioPlayer) {
		try{
		_audioPlayer->setVolume(((double)volume)/100);
		} catch (...) {
			qDebug() << "Something wrong";
		}
	}
}

void VideoPlayer::startTalk()
{
	qDebug() << "VideoPlayer:startTalk";
	if (_timer) {
		_timer->stop();
	}
	//_audioPlayer;
}

void VideoPlayer::pushVideoFrame(QByteArray *data)
{
	QPixmap* picture = 0;
	try {
		picture = getPixmapFromArray(data,_form->getView()->width(),_form->getView()->height());
		if (picture && _form) {
			_form->getView()->setPixmap(*picture);
		}
		if (picture)
			delete picture;
	} catch (...) {
		qDebug() << "VideoPlayer::pushVideoFrame Error";
		try {
			if (picture) {
				delete picture;
			}
		} catch(...) {}
	}
}

void VideoPlayer::pushAudioFrame(QByteArray *data)
{
	try {
		if (_audioPlayer) {
			_audioPlayer->addData(*data);
		}
	} catch (...) {
		qDebug() << "VideoPlayer::pushAudioFrame error";
	}
}

QPixmap *VideoPlayer::getPixmapFromArray(QByteArray *data,int width, int height)
{
	QPixmap* pic = 0;
	try {
		std::vector<uint8_t> datagram;
		datagram.resize(data->size());
		memcpy(datagram.data(),data->data(),data->size());
		cv::Mat frame = cv::imdecode(cv::Mat(datagram), CV_LOAD_IMAGE_COLOR);

		int nWidth =  (height*frame.cols)/frame.rows;
		int nHeigth = (width*frame.rows)/frame.cols;

		if (nHeigth > height) {
			nHeigth = height;
		}
		if (nWidth > width) {
			nWidth = width;
		}
		cv::resize(frame,frame,cv::Size(nWidth,nHeigth));
		cv::cvtColor(frame,frame,CV_BGR2RGB);
		QImage img((uchar*)frame.data,frame.cols,frame.rows,frame.step,QImage::Format_RGB888);
		pic = new QPixmap(QPixmap::fromImage(img));
	} catch (...) {

	}
	return pic;
}

void VideoPlayer::onRun()
{
	connect(this,SIGNAL(stopSignal()),this,SLOT(stop()));
	connect(this,SIGNAL(receiveMessage(Message*)),this,SLOT(pushMessage(Message*)));
	_audioPlayer = new AudioPlayer();

	_timer = new QTimer();
	connect(_timer,SIGNAL(timeout()),this,SLOT(onTimer()));
	_audioPlayer->start();

	if (_isIncome) {
		onTimer();
	}
	_timer->start(4000);
	qDebug() << "Timer started";
}

void VideoPlayer::stop()
{
	qDebug() << "stop slot";
	_aliveMutex.lock();
	_alive = false;
	if (_timer) {
		_timer->stop();
	}
	_aliveMutex.unlock();
	emit finished();
}

void VideoPlayer::onTimer()
{
	if (_isIncome) {
		if (Objects::get()->_ring) {
			_audioPlayer->addData(*Objects::get()->_ring);
		}
	} else {
		if (_connected && Objects::get()->_beep) {
			_audioPlayer->addData(*Objects::get()->_beep);
		}
	}
}
