#include "server.h"
#include <QDebug>
#include <QDataStream>
#include <QMessageBox>
#include <objects.h>

Server::Server(QObject *parent):
	QObject(parent),
	_udpSocket(0),
	_timer(0),
	_period(2000),
	_download(0),
	_upload(0),
	_dps(0),
	_ups(0)
{
}

Server::~Server()
{
	stop();
}

Server *Server::getInstance()
{
	return Objects::get()->_server;
}

void Server::start()
{
	_socketMutex.lock();
	_udpSocket = new QUdpSocket();
	connect(_udpSocket,SIGNAL(readyRead()), this, SLOT(receiveMessage()));
	if (_udpSocket->bind(QHostAddress::AnyIPv4, getPort())) {
		connect(this,SIGNAL(transmitMessageSignal(Message*)),this,SLOT(transmitMessage(Message*)));
		_timer = new QTimer();
		connect(_timer,SIGNAL(timeout()),this,SLOT(calculateStatistic()));
		qDebug() << "Udp Server started on Port: " << getPort();
		_udpSocket->open(QIODevice::ReadWrite);
		_timer->start(_period);
	} else {
		qDebug() << "Udp Setver Error";
		QMessageBox::warning(0,"Server","Server starting error",QMessageBox::Ok);
	}
	_socketMutex.unlock();
}

void Server::stop()
{
	_socketMutex.lock();
	if (_udpSocket) {
		_udpSocket->close();
		disconnect(this,SIGNAL(transmitMessageSignal(Message*)),this,SLOT(transmitMessage(Message*)));
		disconnect(_udpSocket,SIGNAL(readyRead()), this, SLOT(receiveMessage()));
		delete _udpSocket;
		_udpSocket = 0;
	}
	_socketMutex.unlock();
	if (_timer) {
		_timer->stop();
		delete _timer;
	}
}

void Server::sendMessage(Message *message)
{
	emit transmitMessageSignal(message);
}

void Server::getStatistic(int &dps, int &ups)
{
	dps = _dps;
	ups = _ups;
}

void Server::receiveMessage()
{
	QByteArray* datagram = new QByteArray;
	QHostAddress* host = new QHostAddress();
	quint16* port = new quint16;

	while (_udpSocket && _udpSocket->hasPendingDatagrams()) {
		_socketMutex.lock();
		datagram->resize(_udpSocket->pendingDatagramSize());
		_udpSocket->readDatagram(datagram->data(), datagram->size(), host, port);
		_socketMutex.unlock();
		_download += datagram->size();
		/*
	 * Имитация искажения*/
		/*
	int index = 3;
	quint8 mask = (0x3);
	datagram->insert(index, datagram->at(index) ^ mask);
	datagram->remove(index + 1, 1);
	*/
		Message* msg = new Message();
		switch (msg->fromArray(*host,*port,datagram)) {
		case 0:{
			//qDebug() << "Receive:"<< msg->host()->toString()<< " t:" << msg->type() << " s:" << msg->subType();
			emit receivedMessage(msg);
			break;
		}
		case 1:{
			qDebug() << "Receive Message Error";
			delete msg;
			break;
		}
		case 2:{
			//qDebug() << "IN: " << msg->host()->toString();
			emit receivedUnknownMessage(msg);
			break;
		}
		default:
			if (msg) {
				delete msg;
			}
			break;
		}
	}
	delete datagram;
	delete host;
	delete port;
}

int Server::transmitMessage(Message *message)
{
	if (!message) {
		return 1;
	}

	if (!message->port() || !message->host()) {
		delete message;
		return 1;
	}

	if(!_udpSocket) {
		delete message;
		return 1;
	}

	int result = 0;
	switch (message->type()) {
	case Message::MEDIA:{
		result = sendMediaData(message);
		break;
	}
	case Message::CONTROL: {
		result = sendControlData(message);
		//qDebug() << "Send:"<< message->host()->toString()<< " t:" << message->type() << " s:" << message->subType();
		break;
	}
	default:
		result = 1;
	}
	delete message;
	//qDebug() << "OUT: " << result <<" host:" << message->host()->toString();
	return result;
}

void Server::calculateStatistic()
{
	_dps = (_download*8)/_period;
	_ups = (_upload*8)/_period;
	_download = 0;
	_upload = 0;
}

int Server::sendMediaData(Message* message)
{
	int result = 0;
	switch (message->subType()) {
	case SubType::AUDIO:
	case SubType::VIDEO: {
		QByteArray* data = message->toByteArray();
		_socketMutex.lock();
		_udpSocket->writeDatagram(*data, *message->host(), *message->port());
		_socketMutex.unlock();
		_upload += data->size();
		delete data;
		result = 0;
		break;
	}
	default:
		result = 1;
		break;
	}
	return result;
}

int Server::sendControlData(Message *message)
{

	int result = 0;
	switch (message->subType()) {
	case SubType::INVITE:
	case SubType::WAIT:
	case SubType::OK:
	case SubType::CANCEL:
	case SubType::REPEAT:
	case SubType::ONLINE:{
		QByteArray* data = message->toByteArray();
		_socketMutex.lock();
		_udpSocket->writeDatagram(*data, *message->host(), *message->port());
		_socketMutex.unlock();
		_upload += data->size();
		delete data;
		result = 0;
		break;
	}
	default:
		result = 1;
		break;
	}
	return result;
}

quint16 Server::getPort()
{
	return Objects::get()->_serverPort;
}
