#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addcontact.h"
#include <QDebug>
#include <QVector>
#include <QMessageBox>
#include <QXmlStreamReader>

#include "callcontroller.h"

#include "audiosettings.h"
#include "videosettings.h"

#include "objects.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	_port(0)
{
	ui->setupUi(this);
	this->setWindowIcon(QIcon(":/image/image/eleron_favicon.png"));

	/***************LocalHost*****************/
	_contactVector.clear();
	ui->deleteBtn->setEnabled(false);
	ui->callBtn->setEnabled(false);
	_contacts = ui->contactList;

	Contact local;
	local.name = "localhost";
	local.ip = "127.0.0.1";
	_contactVector.push_back(local);

	readConfigData();

	foreach (Contact item, _contactVector) {
		_contacts->setRowCount(_contacts->rowCount()+1);
		_contacts->setItem(_contacts->rowCount()-1, 0, new QTableWidgetItem(item.name));
		_contacts->setItem(_contacts->rowCount()-1, 1, new QTableWidgetItem(item.ip));
	}

	if (_port) {
		Objects::get()->_serverPort = _port;
	}
	/*****************************************/

	CallController* cc = new CallController(this);
	Objects::get()->_callController = cc;




	QString out = AudioSettings::get()->outputDevice().deviceName();
	QString in = AudioSettings::get()->inputDevice().deviceName();

	_outputDeviceList = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);

	//ui->outputDevicesBox->addItem("Default Output Device");
	foreach (QAudioDeviceInfo item, _outputDeviceList) {
		ui->outputDevicesBox->addItem(item.deviceName());
		if (item.deviceName() == out)
			ui->outputDevicesBox->setCurrentIndex(ui->outputDevicesBox->count()-1);
	}

	_inputDeviceList = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
	//ui->inputDevicesBox->addItem("Default Input Device");
	foreach (QAudioDeviceInfo item, _inputDeviceList) {
		ui->inputDevicesBox->addItem(item.deviceName());
		if (item.deviceName() ==  in)
			ui->inputDevicesBox->setCurrentIndex(ui->inputDevicesBox->count()-1);
	}

	_cameraDeviceList = QCameraInfo::availableCameras();
	foreach (QCameraInfo item, _cameraDeviceList) {
		ui->cameraDevicesBox->addItem(item.description());
	}

	connect(&_timer,SIGNAL(timeout()),this,SLOT(onTimer()));
	_timer.start(1984);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::readConfigData()
{
	qDebug() << "Start";
	QFile* file = new QFile("config.xml");
	if (!file->open(QIODevice::ReadOnly|QIODevice::Text)) {
		qDebug() << "File not opend";
		return;
	}
	QXmlStreamReader xml(file);
	while (!xml.atEnd() && !xml.hasError()) {
		QXmlStreamReader::TokenType token = xml.readNext();
	   if (token == QXmlStreamReader::StartDocument)
		   continue;
	   if (token == QXmlStreamReader::StartElement)
	   {
		   if (xml.name() == "port") {
			   xml.readNext();
			   bool ok;
			   int port = xml.text().toInt(&ok);
			   if (ok) {
				   _port = port;
			   }
		   }

		   if (xml.name() == "item") {
			   token = xml.readNext();
			   token = xml.readNext();
			   if (token == QXmlStreamReader::StartElement && xml.name() == "name")
			   {
				   token = xml.readNext();
				   Contact item;
				   item.name = xml.text().toString();
				   token = xml.readNext();
				   token = xml.readNext();
				   token = xml.readNext();
				   token = xml.readNext();
				   item.ip = xml.text().toString();
				   QHostAddress h;
				   if (h.setAddress(item.ip))
					   _contactVector.push_back(item);
			   }
		   }
	   }
	}
	foreach (Contact item, _contactVector) {
		qDebug() << item.ip << " " << item.name;
	}
	qDebug() << "End";
}

void MainWindow::refreshConfigData()
{
	qDebug() << "Refresh config";
	QFile* file = new QFile("config.xml");

	if (!file->open(QIODevice::WriteOnly|QIODevice::Text)) {
		qDebug() << "File not opend";
		return;
	}

	QTextStream fstream(file);
	fstream << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
	fstream << "<config>\n<port>" + QString::number(Objects::get()->_serverPort) + "</port>\n";

	foreach (Contact item, _contactVector) {
		if(item.name != "localhost") {
			fstream << "<item>\n";
			fstream << "<name>" + item.name + "</name>\n";
			fstream << "<host>" + item.ip + "</host>\n";
			fstream << "</item>\n";
		}
	}

	fstream << "</config>\n";
	file->close();
}

void MainWindow::on_addContactBtn_clicked()
{
	Contact item;
	AddContact addContact(this);
	switch( addContact.exec() ) {
		case QDialog::Accepted:
			item.name = addContact.getName();
			item.ip = addContact.getIP();
			_contactVector.push_back(item);

			_contacts->setRowCount(_contacts->rowCount()+1);
			_contacts->setItem(_contacts->rowCount()-1, 0, new QTableWidgetItem(item.name));
			_contacts->setItem(_contacts->rowCount()-1, 1, new QTableWidgetItem(item.ip));
			break;
		case QDialog::Rejected:
			qDebug() << "Rejected";
			break;
		default:
			qDebug() << "Unexpected";
		}
	refreshConfigData();
}

void MainWindow::on_deleteBtn_clicked()
{
	QList<QTableWidgetItem *> list = _contacts->selectedItems();
	if (!list.isEmpty()) {
		int row = list[0]->row();
		_contacts->removeRow(row);
		_contactVector.remove(row);
	}
	refreshConfigData();
}

void MainWindow::on_contactList_itemSelectionChanged()
{
	if (_contacts->selectedItems().isEmpty()){
		ui->deleteBtn->setEnabled(false);
		ui->callBtn->setEnabled(false);
	} else {
		ui->deleteBtn->setEnabled(true);
		ui->callBtn->setEnabled(true);
	}
}

void MainWindow::on_callBtn_clicked()
{

	QList<QTableWidgetItem *> list = _contacts->selectedItems();
	if (!list.isEmpty()) {
		int row = list[0]->row();

		QHostAddress* host = new QHostAddress(_contactVector[row].ip);
		CallController::getInstance()->createCall(*host);
		delete host;
	}
}

void MainWindow::on_outputDevicesBox_currentIndexChanged(int index)
{
	AudioSettings::get()->setOutputDevice(_outputDeviceList[index]);
}

void MainWindow::on_inputDevicesBox_currentIndexChanged(int index)
{
	AudioSettings::get()->setInputDevice(_inputDeviceList[index]);
}

void MainWindow::on_cameraDevicesBox_currentIndexChanged(int index)
{
	VideoSettings::get()->setDevice((quint8)index);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event)
	qDebug() << "Main: Kill";
	Objects::get()->_callController->killApplication();
}

void MainWindow::onTimer()
{
	int dps = 0;
	int ups = 0;
	Objects::get()->_server->getStatistic(dps,ups);
	ui->uloadLabel->setText(QString::number(ups) + " kbps: out");
	ui->dloadLabel->setText(QString::number(dps) + " kbps: in");
	//qDebug() << "Stat: down: " << dps << " kbps up: " << ups << " kbps";
}

void MainWindow::on_checkBox_toggled(bool checked)
{
	ui->cameraDevicesBox->setEnabled(checked);
	ui->qualitySlider->setEnabled(checked);
	ui->fpsSpinBox->setEnabled(checked);
	Objects::get()->_callController->setVideoRecorderEnable(checked);
}

void MainWindow::on_qualitySlider_valueChanged(int value)
{
	VideoSettings::get()->setJpegQuality(value);
}

void MainWindow::on_fpsSpinBox_valueChanged(int arg1)
{
	int period = 1000/arg1;
	VideoSettings::get()->setPeriod(period);
}

void MainWindow::on_microBox_toggled(bool checked)
{
	ui->inputDevicesBox->setEnabled(checked);
	ui->volumeMicSlider->setEnabled(checked);
	Objects::get()->_callController->setAudioRecorderEnable(checked);
}

void MainWindow::on_volumeMicSlider_valueChanged(int value)
{
	try {
		AudioSettings::get()->setVolume(value);
	} catch (...) {
		qDebug() << "Something wrong";
	}
}
