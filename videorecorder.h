#ifndef VIDEORECORDER_H
#define VIDEORECORDER_H

#include <QIODevice>
#include <QByteArray>
#include <QPixmap>
#include <QMutex>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

#include <vector>

class VideoRecorder : public QIODevice {
	Q_OBJECT
public:
	static VideoRecorder* getInstance();
	explicit VideoRecorder(QObject *parent = 0);
	~VideoRecorder();
	int start();
	void stop();

	QByteArray* getJpegFrame();
	QByteArray* getHevcFrame();
	QPixmap* getPixmapFrame(int width, int height);

	void settingsChanged();
	void jpegQualityChanged();
	void periodChanged();
	bool isRunning();
	void kill();

	qint64 readData(char *data, qint64 maxlen);     /*Функция обязательная для QIODevice не используется*/
	qint64 writeData(const char *data, qint64 len); /*Функция переписывания данных из _qAudioInput в _buffer*/

signals:
public slots:
	void onRun();
private:
	int startRecording();
	void stopRecording();
	void readFrame();
	cv::VideoCapture _camera;
	cv::Mat _frame;
	std::vector<int> _qualityParams;
	int _period;

	std::vector<uchar> _imageJpeg;
	std::vector<uchar> _imageHevc;
	cv::Mat _imageFrame;
	bool _alive;
	QMutex _imageFrameMutex;
	QMutex _imageJpegMutex;
	bool _settingsChanged;
	bool _runVideoRecorder;
};

#endif // VIDEORECORDER_H
