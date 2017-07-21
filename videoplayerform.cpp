#include "videoplayerform.h"
#include "ui_videoplayerform.h"
#include <QDebug>

VideoPlayerForm::VideoPlayerForm(bool income, QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::VideoPlayerForm),
	talking(false),
	volume(0)
{
	ui->setupUi(this);
	this->setWindowIcon(QIcon(":/image/image/eleron_favicon.png"));
	ui->volumeSlider->hide();
	ui->view->setGeometry(0,0,this->width(),this->height());
	ui->screenBtn->hide();
	ui->screenBtn->setIcon(QIcon(":/image/image/fullScreen.png"));
	ui->soundBtn->hide();
	ui->soundBtn->setIcon(QIcon(":/image/image/sound.png"));

	connect(ui->screenBtn,SIGNAL(clicked(bool)),this,SLOT(fullScreenBtnClick()));
	connect(ui->cancelButton,SIGNAL(clicked(bool)),this,SLOT(cancelBtnClick()));
	connect(ui->soundBtn,SIGNAL(clicked(bool)),this,SLOT(soundBtnClick()));
	if (income) {
		connect(ui->acceptButton,SIGNAL(clicked(bool)),this,SLOT(acceptBtnClick()));
		ui->acceptButton->show();
		ui->cancelButton->show();
	} else {
		ui->acceptButton->hide();
		ui->cancelButton->show();
	}
}

VideoPlayerForm::~VideoPlayerForm()
{
	qDebug() << "VideoPlayerForm::~destructor";
	delete ui;
}

QLabel *VideoPlayerForm::getView()
{
	return ui->view;
}

void VideoPlayerForm::setTitle(const QString &str)
{
	this->setWindowTitle(str);
}

void VideoPlayerForm::startTalk()
{
	ui->view->setText("");
	talking = true;
	ui->acceptButton->hide();
	QString style = "QPushButton{"
					"background-color: rgba(170, 0, 0, 90);"
					"color: rgba(255, 255, 255, 150);"
					"font: 9pt \"Arial black\";"
					"border-radius: 37px;}"
					"QPushButton:hover{"
					"color: rgb(255, 255, 255);"
					"background-color: rgb(170, 0, 0);}";

	ui->cancelButton->setGeometry(20, this->height() - 90, ui->cancelButton->width(),ui->cancelButton->height());
	ui->cancelButton->setStyleSheet(style);
	ui->volumeSlider->show();
	ui->screenBtn->show();
	ui->soundBtn->show();
}

void VideoPlayerForm::fullScreenBtnClick()
{
	if (this->isFullScreen()) {
		this->showNormal();
		ui->screenBtn->setIcon(QIcon(":/image/image/fullScreen.png"));
	} else {
		this->showFullScreen();
		ui->screenBtn->setIcon(QIcon(":/image/image/normScreen.png"));
	}
}

void VideoPlayerForm::soundBtnClick()
{
	if (volume) {
		ui->soundBtn->setIcon(QIcon(":/image/image/sound.png"));
		ui->volumeSlider->setValue(volume);
		volume = 0;
	} else {
		ui->soundBtn->setIcon(QIcon(":/image/image/soundOff.png"));
		volume = ui->volumeSlider->value();
		ui->volumeSlider->setValue(0);
	}
}

void VideoPlayerForm::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event)
	qDebug() << "VideoPlayerForm: closed";
	emit formClosed();
}

void VideoPlayerForm::acceptBtnClick()
{
	disconnect(ui->acceptButton, SIGNAL(clicked(bool)), this, SLOT(acceptBtnClick()));
	emit acceptBtn();
	ui->view->setText("Подождите...");
	//ui->centralwidget->setL
}

void VideoPlayerForm::cancelBtnClick()
{
	emit cancelBtn();
}

void VideoPlayerForm::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);
	ui->view->setGeometry(0,0,this->width(),this->height());

	ui->volumeSlider->move(this->width() - ui->volumeSlider->width() - 3,
						   this->height()/2 -ui->volumeSlider->height()/2);

	ui->soundBtn->move(this->width() - ui->soundBtn->width() - 3,
					   ui->volumeSlider->y() + ui->volumeSlider->height() + 5);

	ui->screenBtn->move(this->width() - ui->screenBtn->width() - 3,
						ui->soundBtn->height() + ui->soundBtn->y() + 5);

	if (talking) {
		ui->cancelButton->move(20, this->height() - ui->cancelButton->height()-10);
	} else {
		ui->cancelButton->move(this->width()/2 - ui->cancelButton->width()*(!ui->acceptButton->isHidden()) - ui->cancelButton->width()/2,
									  this->height() - ui->cancelButton->height()-10);
		ui->acceptButton->move(this->width()/2 + (ui->acceptButton->width())/2,
									  this->height() - ui->acceptButton->height()-10);
	}

}

void VideoPlayerForm::on_volumeSlider_valueChanged(int value)
{
	if (value >= 0 && value <= 100) {
		emit setVolume(value);
	}
}
