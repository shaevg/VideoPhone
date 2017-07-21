#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QIODevice>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QMutex>

class AudioRecorder : public QIODevice
{
	Q_OBJECT

private:
	/*Необходимые для записи звука вещи*/
	QAudioInput *_qAudioInput;
	QAudioDeviceInfo _qAudioDeviceInfo;
	QAudioFormat _qAudioFormat;

	/*Буффер _buffer и его настройки*/
	QByteArray _buffer;        /*Буфер для хранения записи перед отправкой*/
	quint16 _bufferSize; /*предельный размер буффера _buffer*/
	quint16 _emitEdge;   /*объём буфера при котором происходит событие QIODevice::ReadyRead*/
	bool _alive;
	bool _running;
	void init(); /*Инициализация.*/
	QMutex _bufferMutex;

public:
	explicit AudioRecorder(QObject *parent = 0);
	~AudioRecorder();
	static AudioRecorder* getInstance();
	/*Функции управления процессом записи*/

	QAudio::State getState(); /*Состояне процесса записи*/
	void settingsChanged();
	QAudioDeviceInfo getInputDevice();
	/*Функции для задания настроек работы буфера _buffer*/
	void setBufferSize(quint16 size);
	void setEmitEdge(quint16 edge);
	void setVolume(qreal vol);
	/*Функция getData(..) для чтения данных из буфера _buffer используется вместо QIODevice::readData
	 *Входной параметр:
	 *    size : количество байт, которое необходимо забрать из буфера _buffer
	 * Возвращает:
	 *    QByteArray* : указатель на массив байт, который получился в результате сжатия (qCompress) извлечённых size байт
	 * */
	QByteArray* getData(quint16 size);

	qint64 readData(char *data, qint64 maxlen);     /*Функция обязательная для QIODevice не используется*/
	qint64 writeData(const char *data, qint64 len); /*Функция переписывания данных из _qAudioInput в _buffer*/


	/*Функция hasData() возвращает:
	 * true  : если размер буффера _buffer больше или равен emitEdge
	 * false : если размер буффера _buffer меньше emitEdge
	 * */
	bool hasData();

	void stop();
	void start();
	void kill();
signals:
	//void update();
	void startSignal();
	void stopSignal();
	void settingsChangedSignal();
public slots:
	void onRun();
private slots:
	void startAudioRecorder();
	void stopAudioRecorder();
	void settingsChangedSlot();
};

#endif // AUDIORECORDER_H
