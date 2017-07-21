#ifndef ADDCONTACT_H
#define ADDCONTACT_H

#include <QDialog>

namespace Ui {
class AddContact;
}

class AddContact : public QDialog
{
	Q_OBJECT

public:
	explicit AddContact(QWidget *parent = 0);
	~AddContact();

	QString getName();
	QString getIP();

private:
	Ui::AddContact *ui;
	bool checkValidate();
signals:

public slots:
	void on_change_something(void);
};

#endif // ADDCONTACT_H
