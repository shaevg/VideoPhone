#ifndef MESSAGE_H
#define MESSAGE_H

#include <QHostAddress>
#include <QByteArray>

namespace SubType {
enum ControlType {UNKNOWN = 0, INVITE = 1, WAIT = 2, OK = 3, CANCEL = 4, REPEAT = 5, ONLINE = 6};
enum MediaType {AUDIO = 7, VIDEO = 8};
}

class Message
{
public:
	enum MessageType {UNKNOWN = 0, CONTROL = 1, MEDIA = 2};
	Message(const MessageType type, const int subType, const QHostAddress& host, const quint16& port, const QByteArray* data = 0);
	Message();
	~Message();
	QHostAddress* host();
	quint16* port();
	QByteArray* data();
	int type() const;
	int subType() const;
	QByteArray* toByteArray() const;
	int fromArray(const QHostAddress& host, const quint16& port, const QByteArray *array);

private:
	QHostAddress* _host;
	quint16* _port;
	QByteArray* _data;
	MessageType _type;
	int _subType;

	quint8 codeType(const MessageType &type) const;
	MessageType decodeType(const quint8 &type) const;

	quint8 codeMediaType(const int& type) const;
	int decodeMediaType(const quint8 &type) const;

	quint32 hashLy(const QByteArray *data, const int &count = 0) const;
	QByteArray* generateHeader() const;
};

#endif // MESSAGE_H
