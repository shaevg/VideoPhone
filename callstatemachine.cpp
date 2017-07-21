#include "callstatemachine.h"

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

CallStateMachine::CallStateMachine(const QHostAddress &host, const quint16 &port, CallState state, QObject *parent) :
	QObject(parent),
	_state(state),
	_host(host),
	_port(port),
	_connection(false),
	_timerShots(0),
	_videoPlayer(0),
	_videoPlayerForm(0),
	_videoThread(this)
{
	connect(&_timer,SIGNAL(timeout()),this,SLOT(onTimer()));

	if (_state == IN || _state == OUT || _state == TALK) {

		_videoPlayerForm = new VideoPlayerForm(_state == IN);
		_videoPlayerForm->setTitle("Talk: " + _host.toString());

		connect(_videoPlayerForm,SIGNAL(cancelBtn()),this,SLOT(rejectCall()));
		connect(this,SIGNAL(startTalk()),_videoPlayerForm,SLOT(startTalk()));

		if (_state == IN)
		{
			_videoPlayerForm->getView()->setText(_host.toString() + "\nВходящий вызов");
			connect(_videoPlayerForm,SIGNAL(acceptBtn()),this,SLOT(acceptCall()));
		} else {
			_videoPlayerForm->getView()->setText(_host.toString() + "\nСоединение");
		}

		connect(_videoPlayerForm,SIGNAL(formClosed()),this,SLOT(videoFormClosed()));

		_videoPlayer = new VideoPlayer(_state == IN);
		_videoPlayer->setForm(_videoPlayerForm);
		connect(_videoPlayerForm,SIGNAL(setVolume(int)),_videoPlayer,SLOT(setVolume(int)));
		connect(this,SIGNAL(startTalk()),_videoPlayer,SLOT(startTalk()));
		connect(&_videoThread,SIGNAL(started()),_videoPlayer,SLOT(onRun()));

		_videoPlayer->moveToThread(&_videoThread);

		_videoPlayerForm->show();
		_videoThread.start();
	}

	goToState(_state);
}

CallStateMachine::~CallStateMachine()
{
	if (_videoPlayer) {
		//_videoPlayer->deleteLater();
		_videoPlayer->deleteLater();
		_videoThread.quit();
		_videoThread.wait();
	}

	if (_videoPlayerForm) {
		_videoPlayerForm->hide();
		_videoPlayerForm->deleteLater();
	}
	qDebug() << "CallStateMachine::~destructor " << _host.toString();
}

void CallStateMachine::pushMessage(Message *message)
{
	switch (message->type()) {
	case Message::CONTROL: {
		receiveControlMessage(message);
		break;
	}
	case Message::MEDIA: {
		receiveMediaMessage(message);
		break;
	}
	default:
		Message* msg = new Message(Message::CONTROL, SubType::REPEAT,
								   *message->host(), *message->port());
		Server::getInstance()->sendMessage(msg);
		delete message;
		break;
	}
}

void CallStateMachine::sendAudioData(QByteArray *data)
{
	if (_state == TALK) {
		Message* msg = new Message(Message::MEDIA, SubType::AUDIO,
								   _host, _port, data);
		Server::getInstance()->sendMessage(msg);
	}
}

void CallStateMachine::sendVideoData(QByteArray *data)
{
	if (_state == TALK) {
		Message* msg = new Message(Message::MEDIA, SubType::VIDEO,
								   _host, _port, data);
		Server::getInstance()->sendMessage(msg);
	}
}

void CallStateMachine::rejectCall()
{
	goToState(FINISH);
}

void CallStateMachine::acceptCall()
{
	if (_state == IN) {
		goToState(TALK);
		qDebug() << "Call Accepted";
		emit startTalk();
		emit statusChanged();
	}
}

CallStateMachine::CallState CallStateMachine::getState()
{
	return _state;
}

QHostAddress CallStateMachine::getHost() const
{
	return _host;
}

quint16 CallStateMachine::getPort() const
{
	return _port;
}

bool CallStateMachine::isConnected() const
{
	return _connection;
}

void CallStateMachine::onTimer()
{
	if (_state == OUT || _state == TALK || _state == ECHO) {
		++_timerShots;
		if (_state == OUT) {
			QString str = "Соединение";
			for(int i = 0; i <_timerShots;++i) {
				str+=".";
			}
			_videoPlayerForm->getView()->setText(str);
		}
	}

	if (_timerShots > 4) {
		if (_state == ECHO) {
			emit userOnline(false, _host);
			_connection = false;
			goToState(CANCELS);
		} else {
			_connection = false;
			goToState(FINISH);
		}
	} else {
		goToState(_state);
	}
}

void CallStateMachine::videoFormClosed()
{
	rejectCall();
}

void CallStateMachine::goToState(CallState state)
{
	switch (state) {
	case START:{
		_state = START;
		break;
	}
	case IN: {
		_state = IN;
		_timer.stop();
		Message* msg = new Message(Message::CONTROL, SubType::WAIT, _host, _port);
		Server::getInstance()->sendMessage(msg);
		//delete msg;
		_connection = true;
		_timer.start(1000);
		break;
	}
	case OUT: {
		_state = OUT;
		_timer.stop();
		Message* msg = new Message(Message::CONTROL, SubType::INVITE, _host, _port);
		Server::getInstance()->sendMessage(msg);
		//delete msg;
		_timer.start(2000);
		break;
	}
	case TALK: {
		_state = TALK;
		_timer.stop();
		Message* msg = new Message(Message::CONTROL, SubType::OK, _host, _port);
		Server::getInstance()->sendMessage(msg);
		//delete msg;
		_timer.start(2000);
		break;
	}
	case FINISH: {
		_state = CANCELS;
		_timer.stop();
		_timerShots = 0;
		_connection = false;
		Message* msg = new Message(Message::CONTROL, SubType::CANCEL, _host, _port);
		Server::getInstance()->sendMessage(msg);
		//delete msg;
	}
	case CANCELS: {
		_state = CANCELS;
		_timer.stop();
		_timerShots = 0;
		_connection = false;
		emit statusChanged();
		break;
	}
	case ECHO: {
		_state = ECHO;
		_timer.stop();
		Message* msg = new Message(Message::CONTROL, SubType::ONLINE, _host, _port);
		Server::getInstance()->sendMessage(msg);
		//delete msg;
		_timer.start(2000);
		break;
	}
	}
}

void CallStateMachine::receiveControlMessage(Message *message)
{
	if (*message->host() != _host ||
			*message->port() != _port) {
		delete message;
		return;
	}

	if (message->subType() == SubType::REPEAT) {
		goToState(_state);
		return;
	}

	if (message->subType() == SubType::ONLINE && _state != ECHO) {
		Message* msg = new Message(Message::CONTROL, SubType::ONLINE, _host, _port);
		Server::getInstance()->sendMessage(msg);
		delete msg;
		delete message;
		return;
	}

	if (message->subType() == SubType::UNKNOWN) {
		Message* msg = new Message(Message::CONTROL, SubType::REPEAT, _host, _port);
		Server::getInstance()->sendMessage(msg);
		delete msg;
		delete message;
		return;
	}

	switch (_state) {
	case IN: {
		if (message->subType() == SubType::INVITE) {
			goToState(_state);
		} else if (message->subType() == SubType::CANCEL) {
			goToState(FINISH);
		}
		break;
	}
	case OUT: {
		if (message->subType() == SubType::INVITE) {
			goToState(TALK);
			emit startTalk();
			emit statusChanged();
		} else if (message->subType() == SubType::CANCEL) {
			goToState(FINISH);
		} else if (message->subType() == SubType::OK) {
			goToState(TALK);
			emit startTalk();
			emit statusChanged();
		} else if (message->subType() == SubType::WAIT) {
			_connection = true;
			_videoPlayerForm->getView()->setText(_host.toString() + "\nВызов");
			_videoPlayer->setConnected();
			_timer.stop();
			_timerShots = 0;
			_timer.start(2000);
		}
		break;
	}
	case TALK:{
		if (message->subType() == SubType::CANCEL) {
			goToState(FINISH);
		} else if (message->subType() == SubType::OK) {
			_timerShots = 0;
			_connection = true;
		} else if (message->subType() == SubType::WAIT) {
			goToState(OUT);
		}
		break;
	}
	case CANCELS:{
		break;
	}
	case ECHO:
		if (message->subType() == SubType::ONLINE) {
			emit userOnline(true, _host);
			goToState(CANCELS);
		}
		break;
	case START:
	case FINISH:{break;}
	}
	delete message;
}

void CallStateMachine::receiveMediaMessage(Message *message)
{
	if (_videoPlayer && _state == TALK) {
		_videoPlayer->addMessage(message);
	} else {
		delete message;
	}
}
