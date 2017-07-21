#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QVector>
#include <QHostAddress>
#include <QTimer>
#include <QLabel>
#include <QCameraInfo>

#include "defines.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	Ui::MainWindow *ui;
	QTableWidget* _contacts;
	QVector<Contact> _contactVector;

	QList<QAudioDeviceInfo> _outputDeviceList;
	QList<QAudioDeviceInfo> _inputDeviceList;

	QList<QCameraInfo> _cameraDeviceList;

	QTimer _timer;
	void readConfigData();
	void refreshConfigData();
	int _port;
signals:
	void killApplication();
private slots:
	void on_addContactBtn_clicked();
	void on_deleteBtn_clicked();
	void on_contactList_itemSelectionChanged();
	void on_callBtn_clicked();
	void on_outputDevicesBox_currentIndexChanged(int index);
	void on_inputDevicesBox_currentIndexChanged(int index);
	void on_cameraDevicesBox_currentIndexChanged(int index);
	void closeEvent(QCloseEvent* event);
	void onTimer();
	void on_checkBox_toggled(bool checked);
	void on_qualitySlider_valueChanged(int value);
	void on_fpsSpinBox_valueChanged(int arg1);
	void on_microBox_toggled(bool checked);
	void on_volumeMicSlider_valueChanged(int value);
};

#endif // MAINWINDOW_H
