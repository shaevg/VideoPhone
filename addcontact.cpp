#include "addcontact.h"
#include "ui_addcontact.h"
#include <QDebug>

AddContact::AddContact(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AddContact)
{
	ui->setupUi(this);
	QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
	QRegExp ipRegex ("^" + ipRange+ "\\." + ipRange+ "\\." + ipRange+ "\\." + ipRange + "$");
	QRegExpValidator *ipValidator = new QRegExpValidator(ipRegex, this);
	QRegExp nameRegex ("[0-9a-zA-Zа-яА-Я .()-]{4,50}");
	QRegExpValidator *nameValidator = new QRegExpValidator(nameRegex, this);
	ui->ipEdit->setValidator(ipValidator);
	ui->nameEdit->setValidator(nameValidator);
	ui->addButton->setEnabled(false);

	connect(ui->ipEdit,SIGNAL(textChanged(QString)),this,SLOT(on_change_something(void)));
	connect(ui->nameEdit,SIGNAL(textChanged(QString)),this,SLOT(on_change_something(void)));

	connect(ui->addButton,SIGNAL(clicked(bool)),SLOT(accept()));
	connect(ui->cancelButton,SIGNAL(clicked(bool)),SLOT(reject()));
}

AddContact::~AddContact()
{
	delete ui;
}

QString AddContact::getName()
{
	return ui->nameEdit->text();
}

QString AddContact::getIP()
{
	return ui->ipEdit->text();
}

bool AddContact::checkValidate()
{
	if (!ui->ipEdit->hasAcceptableInput())
		return false;
	if (!ui->nameEdit->hasAcceptableInput())
		return false;
	return true;
}



void AddContact::on_change_something(void)
{
	ui->addButton->setEnabled(checkValidate());
}
