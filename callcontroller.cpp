#include "callcontroller.h"

#include "mediasettings.h"
#include "objects.h"

CallController *CallController::getInstance()
{
	return Objects::get()->_callController;
}

CallController::CallController(QObject *parent):
	QObject(parent),
	_audioRecorder(new AudioRecorder()),
	_videoRecorder(new VideoRecorder()),
	_server(new Server()),
	_videoThread(new QThread(this)),
	_audioThread(new QThread(this)),
	_serverThread(new QThread(this)),
	_videoRecorderEnable(true),
	_audioRecorderEnable(true)
{
	_connections.clear();
	Objects::get()->_videoRecorder = _videoRecorder;
	Objects::get()->_audioRecorder = _audioRecorder;
	Objects::get()->_server = _server;

	connect(_server, SIGNAL(receivedMessage(Message*)), this, SLOT(newMessage(Message*)));
	connect(_server, SIGNAL(receivedUnknownMessage(Message*)), this, SLOT(newUnknownMessage(Message*)));
	connect(_serverThread,SIGNAL(started()),_server,SLOT(start()));

	connect(_audioRecorder, SIGNAL(readyRead()), this, SLOT(readAudioRecord()));
	connect(_audioThread, SIGNAL(started()),_audioRecorder, SLOT(onRun()));

	connect(_videoRecorder, SIGNAL(readyRead()), this, SLOT(readVideoFrame()));
	connect(_videoThread, SIGNAL(started()),_videoRecorder, SLOT(onRun()));

	_videoRecorder->moveToThread(_videoThread);
	_audioRecorder->moveToThread(_audioThread);
	_server->moveToThread(_serverThread);

	_serverThread->start();
	_audioThread->start();
	_videoThread->start();
}

CallController::~CallController()
{

}

bool CallController::createCall(const QHostAddress& host)
{
	bool result = false;
	//QMessageBox::information(0,"Call Controller","Creating call with: " + host.toString(),QMessageBox::Ok);
	if (_connections.find(host.toIPv4Address()) == _connections.end()) {
		CallStateMachine* machine = new CallStateMachine(host, Server::getPort(), CallStateMachine::OUT);
		connect(machine,SIGNAL(statusChanged()),this,SLOT(checkConnections()));
		_connectionsMutex.lock();
		_connections.insert(host.toIPv4Address(), machine);
		_connectionsMutex.unlock();
		result = true;
	}
	return result;
}

void CallController::rejectCall(const QHostAddress &host)
{
	CallMap::iterator it = _connections.find(host.toIPv4Address());
	if (it != _connections.end()) {
		it.value()->rejectCall();
	}
}

void CallController::acceptCall(const QHostAddress &host)
{
	CallMap::iterator it = _connections.find(host.toIPv4Address());
	if (it != _connections.end()) {
		it.value()->acceptCall();
	}
}

void CallController::checkUserAsync(const QHostAddress &host)
{
	CallMap::iterator it = _connections.find(host.toIPv4Address());

	if (it == _connections.end()) {
		CallStateMachine* machine = new CallStateMachine(host, Server::getPort(), CallStateMachine::ECHO, this);
		_connections.insert(host.toIPv4Address(), machine);
		connect(machine,SIGNAL(userOnline(bool,QHostAddress)),this,SLOT(userOnlineFromMachine(bool,QHostAddress)));
	} else {
		if (it.value()->getState() == CallStateMachine::CANCELS){
			checkConnections();
			checkUserAsync(host);
		} else if (it.value()->getState() == CallStateMachine::ECHO) {
		} else {
			emit userOnline(it.value()->isConnected(),host);
		}
	}
}

void CallController::newMessage(Message *message)
{
	CallMap::iterator it = _connections.find(message->host()->toIPv4Address());
	if (it != _connections.end()) {
		it.value()->pushMessage(message);
	} else {
		switch (message->type()) {
		case Message::CONTROL:{
			//qDebug() << "Newcomer Control Message";
			newcomerControlMessage(message);
			break;
		}
		case Message::MEDIA:{
			//qDebug() << "Newcomer Media Message";
			newcomerMediaMessage(message);
			break;
		}
		default:
			break;
		}
	}
}

void CallController::newUnknownMessage(Message *message)
{
	newMessage(message);
}

void CallController::userOnlineFromMachine(const bool &isOnline, const QHostAddress &host)
{
	emit userOnline(isOnline,host);
}

void CallController::readAudioRecord()
{
	AudioRecorder* rec = 0;
	rec = AudioRecorder::getInstance();

	if (rec && !_audioRecorderEnable) {
		rec->stop();
		return;
	}

	while(rec && _audioRecorder->hasData()) {
		QByteArray* data = rec->getData(640);
		if (data && data->size() > 0 && _connections.size() > 0) {
			//_player.addData(*data);
			for(auto it = _connections.begin(); it != _connections.end(); ++it) {
				CallStateMachine* m = it.value();
				if (m->getState() == CallStateMachine::TALK) {
					m->sendAudioData(data);
				}
			}
			delete data;
		}
	}
}

void CallController::readVideoFrame()
{
	QByteArray* data = 0;
	VideoRecorder* rec = 0;
	rec = VideoRecorder::getInstance();

	if (rec && !_videoRecorderEnable) {
		rec->stop();
		return;
	}

	if (rec) {
		data = rec->getJpegFrame();

		if (data && data->size() > 0 && _connections.size() > 0) {
			for(auto it = _connections.begin(); it != _connections.end(); ++it) {
				CallStateMachine* m = it.value();
				if (m->getState() == CallStateMachine::TALK) {
					m->sendVideoData(data);
				}
			}
			delete data;
		}
	}
}

void CallController::checkConnections()
{
	bool talking = false;
	//qDebug()<<"Connections:";
	_connectionsMutex.lock();
	for(auto it = _connections.begin(); it != _connections.end();) {
		CallStateMachine* m = it.value();
		//qDebug()<<m->getHost().toString() << " state" << m->getState();
		if (m->getState() == CallStateMachine::CANCELS) {
			delete m;
			_connections.remove((it++).key());
		} else {
			if (m->getState() == CallStateMachine::TALK)
				talking = true;
			++it;
		}
	}
	_connectionsMutex.unlock();

	if (talking) {
		if (_audioRecorder->getState() && _audioRecorderEnable) {
			_audioRecorder->start();
		}
		if (!_videoRecorder->isRunning() && _videoRecorderEnable) {
			_videoRecorder->start();
		}
	} else {
		if (!_audioRecorder->getState()) {
			_audioRecorder->stop();
		}
		if (_videoRecorder->isRunning()) {
			_videoRecorder->stop();
		}
	}
}

void CallController::killApplication()
{
	qDebug() << "CallController: kill";

	while (_connections.size() > 0) {
		foreach (CallStateMachine* item, _connections) {
			if (item->getState() != CallStateMachine::CANCELS) {
				item->rejectCall();
			}
		}
	}
	Objects::get()->_audioRecorder = 0;
	Objects::get()->_videoRecorder = 0;
	//Objects::get()->_server = 0;

	while(!_audioRecorder->getState()) {
	}

	_videoRecorder->kill();
	while(_videoRecorder->isRunning()) {
	}

	_audioRecorder->deleteLater();
	_audioThread->quit();
	_audioThread->wait();

	_videoRecorder->deleteLater();
	_videoThread->quit();
	_videoThread->wait();

	_server->deleteLater();
	_serverThread->quit();
	_serverThread->wait();

	qDebug() << "all threads killed";
}

void CallController::newcomerControlMessage(Message *message)
{
	switch (message->subType()) {
	case SubType::INVITE: {
		/*
		 * Incoming call
		 * */
		CallStateMachine* machine = new CallStateMachine(*message->host(), *message->port(),
														 CallStateMachine::IN);
		_connections.insert(message->host()->toIPv4Address(),machine);
		connect(machine,SIGNAL(statusChanged()),this,SLOT(checkConnections()));
		break;
	}
	case SubType::ONLINE: {
		/*
		 * Online echo
		 * */
		Message* msg = new Message(Message::CONTROL, SubType::ONLINE,
								   *message->host(), *message->port());
		Server::getInstance()->sendMessage(msg);
		break;
	}
	case SubType::UNKNOWN: {
		/*
		 * Echo replay
		 * */
		Message* msg = new Message(Message::CONTROL, SubType::REPEAT,
								   *message->host(), *message->port());
		Server::getInstance()->sendMessage(msg);
		break;
	}
	case SubType::CANCEL: {
		break;
	}
	default:
		Message* msg = new Message(Message::CONTROL, SubType::CANCEL,
								   *message->host(), *message->port());
		Server::getInstance()->sendMessage(msg);
		break;
	}
	delete message;
}

void CallController::newcomerMediaMessage(Message *message)
{
	delete message;
}

void CallController::setAudioRecorderEnable(bool audioRecorderEnable)
{
	_audioRecorderEnable = audioRecorderEnable;
	if (audioRecorderEnable && _audioRecorder->getState()) {
		checkConnections();
	}
}

void CallController::setVideoRecorderEnable(bool videoRecorderEnable)
{
	_videoRecorderEnable = videoRecorderEnable;
	if (videoRecorderEnable && !_videoRecorder->isRunning()) {
		checkConnections();
	}
}
