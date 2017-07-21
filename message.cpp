#include "message.h"
#include <QDataStream>

Message::Message(const MessageType type, const int subType, const QHostAddress &host, const quint16 &port, const QByteArray *data):
	_host(new QHostAddress(host)),
	_port(new quint16(port)),
	_type(type),
	_subType(subType)
{
	if (data){
		_data = new QByteArray(*data);
	} else {
		_data = 0;
	}
}

Message::Message():
	_host(0),
	_port(0),
	_data(0),
	_type(Message::UNKNOWN),
	_subType(SubType::UNKNOWN)
{

}

Message::~Message()
{
	if (_host) {
		delete _host;
	}

	if (_port) {
		delete _port;
	}

	if (_data) {
		delete _data;
	}
}

QHostAddress *Message::host()
{
	return _host;
}

quint16 *Message::port()
{
	return _port;
}

QByteArray *Message::data()
{
	return _data;
}

int Message::type() const
{
	return _type;
}

int Message::subType() const
{
	return _subType;
}

QByteArray *Message::toByteArray() const
{
	QByteArray* array = new QByteArray();
	quint8 typeCode = codeType(_type);

	QDataStream out(array,QIODevice::WriteOnly);
	out << (quint16)0;
	out << (quint8)typeCode;
	if (_type == MEDIA) {
		out << (quint8)codeMediaType(_subType);
	} else {
		out << (quint8)_subType;
	}

	out << (quint32)0;

	if (_data && _data->size() > 0)
		array->append(*_data);

	out.device()->seek(0);
	out << (quint16)array->size();

	out.device()->seek(4);
	out << (quint32)hashLy(array,4);

	return array;
}

int Message::fromArray(const QHostAddress& host, const quint16& port, const QByteArray *array)
{
	if (array->size() < 8) {
		return 1;
	}

	QByteArray* arraycpy = new QByteArray(*array);

	QDataStream in (arraycpy, QIODevice::ReadOnly);
	quint16 size = 0;

	in >> size;
	if (size != arraycpy->size()) {
		return 1;
	}

	quint8 type = 0;
	quint8 subType = 0;
	quint32 hash = 0;

	in >> type;
	in >> subType;
	in >> hash;

	QByteArray* data = NULL;

	if (size > 8) {
		data = new QByteArray();
		data->append(arraycpy->data() + 8, size - 8);
	}

	_host = new QHostAddress(host);
	_port = new quint16(port);
	_type = decodeType(type);

	if (_type == MEDIA) {
		_subType = decodeMediaType(subType);
	} else {
		_subType = subType;
	}

	_data = data;

	QByteArray* checkData = this->generateHeader();
	quint32 curHash = hashLy(checkData);
	delete checkData;

	if (hash != curHash) {
		return 2;
	}

	return 0;
}

quint8 Message::codeType(const Message::MessageType &type) const
{
	switch (type) {
	case CONTROL:{
		return 0xF0;
		break;
	}
	case MEDIA:
		return 0x0F;
		break;
	default:
		return 0x00;
		break;
	}
}

Message::MessageType Message::decodeType(const quint8 &type) const
{
	quint8 maskControl = 0xF0^type;
	quint8 maskMedia = 0x0F^type;

	quint8 dControl = 0;
	quint8 dMedia = 0;
	for (int i = 0; i < 8; ++i) {
		dControl += 0x01 & maskControl;
		dMedia += 0x01 & maskMedia;
		maskControl >>= 1;
		maskMedia >>= 1;
	}
	if (dControl < 4)
		return CONTROL;
	if (dMedia < 4)
		return MEDIA;
	return UNKNOWN;
}

quint8 Message::codeMediaType(const int &type) const
{
	switch (type) {
	case SubType::AUDIO:{
		return 0xF0;
		break;
	}
	case SubType::VIDEO:
		return 0x0F;
		break;
	default:
		return 0x00;
		break;
	}
}

int Message::decodeMediaType(const quint8 &type) const
{
	quint8 maskAudio = 0xF0^type;
	quint8 maskVideo = 0x0F^type;

	quint8 dAudio = 0;
	quint8 dVideo = 0;
	for (int i = 0; i < 8; ++i) {
		dAudio += 0x01 & maskAudio;
		dVideo += 0x01 & maskVideo;
		maskAudio >>= 1;
		maskVideo >>= 1;
	}
	if (dAudio < 4)
		return SubType::AUDIO;
	if (dVideo < 4)
		return SubType::VIDEO;
	return SubType::UNKNOWN;
}

quint32 Message::hashLy(const QByteArray *data,const int& count) const
{
	quint32 hash = 0;
	if (count == 0){
		foreach (quint8 byte, *data) {
			hash = (hash * 1664525) + byte + 1013904223;
		}
	} else {
		quint8 byte = 0;
		for(int i = 0; i < count; ++i){
			byte = (quint8)data->at(i);
			hash = (hash * 1664525) + byte + 1013904223;
		}
	}

	return hash;
}

QByteArray* Message::generateHeader() const
{
	QByteArray* array = new QByteArray();
	quint8 typeCode = codeType(_type);
	qint16 size = 0;

	QDataStream out(array,QIODevice::WriteOnly);
	out << (quint16)0;
	out << (quint8)typeCode;
	if (_type == MEDIA) {
		out << (quint8)codeMediaType(_subType);
	} else {
		out << (quint8)_subType;
	}
	size += array->size() + sizeof(quint32);

	if (_data && _data->size() > 0) {
		size += _data->size();
	}

	out.device()->seek(0);
	out << size;

	return array;
}








