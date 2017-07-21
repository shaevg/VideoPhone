#ifndef VIDEOPLAYERFORM_H
#define VIDEOPLAYERFORM_H

#include <QMainWindow>
#include <QLabel>

namespace Ui {
class VideoPlayerForm;
}

class VideoPlayerForm : public QMainWindow
{
	Q_OBJECT

public:
	explicit VideoPlayerForm(bool income, QWidget *parent = 0);
	~VideoPlayerForm();
	QLabel *getView();
	void setTitle(const QString& str);
signals:
	void formClosed();
	void acceptBtn();
	void cancelBtn();
	void setVolume(int volume);
private:
	Ui::VideoPlayerForm *ui;
	bool talking;
	int volume;
private slots:
	void closeEvent(QCloseEvent* event);
	void acceptBtnClick();
	void cancelBtnClick();
	void resizeEvent(QResizeEvent* event);
	void on_volumeSlider_valueChanged(int value);
	void startTalk();
	void fullScreenBtnClick();
	void soundBtnClick();
};

#endif // VIDEOPLAYERFORM_H
