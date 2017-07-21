#ifndef DEFINES_H
#define DEFINES_H
#include <QtCore>
#include <QAudioDeviceInfo>

class Contact{

public:
	Contact():name(),ip(){
	}

	QString name;
	QString ip;
};
#endif // DEFINES_H
