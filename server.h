#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QByteArray>
#include <QTimer>
#include <QMutex>
#include "message.h"

class Server: public QObject
{
	Q_OBJECT
public:
	static Server* getInstance();
	Server(QObject* parent = 0);
	~Server();
	static quint16 getPort();
	void stop();
	void sendMessage(Message *message);
	void getStatistic(int& dps,int& ups);
private slots:
	void start();
	void receiveMessage();
	int transmitMessage(Message *message);
	void calculateStatistic();
signals:
	void transmitMessageSignal(Message *message);
	void receivedMessage(Message* message);
	void receivedUnknownMessage(Message* message);
private:
	int sendMediaData(Message *message);
	int sendControlData(Message *message);

	QUdpSocket* _udpSocket;
	QMutex _socketMutex;

	QTimer* _timer;

	int _period;
	int _download;
	int _upload;
	int _dps;
	int _ups;
};

#endif // SERVER_H
