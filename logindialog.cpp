#include "logindialog.h"
#include "ui_logindialog.h"

#include <QDesktopWidget>
#include <QMessageBox>
#include "databaseconnection.h"

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog), m_dataBase(new DataBaseConnection)
{
  ui->setupUi(this);

  QRect scr = QApplication::desktop()->screenGeometry();
  move( scr.center() - rect().center() );

  m_loginButton  = ui->login;
  m_createButton = ui->createAcc;

  QRegExp rx ("^[A-Za-z0-9_-]{,16}$");  // for nickname

  ui->nickname->setValidator(new QRegExpValidator (rx, this));

  connect(ui->passwordSecond, SIGNAL(textChanged(QString)), this, SLOT(disableButton(QString)));
}

LoginDialog::~LoginDialog()
{
  delete ui;
}

QPushButton *LoginDialog::GetButtonForLogin()
{
  return m_loginButton;
}

QPushButton *LoginDialog::GetButtonForCreate()
{
  return m_createButton;
}

void LoginDialog::on_login_clicked()
{
  if(ui->nickname->text() == "" || ui->password->text() == "")
  {
    QMessageBox::information(this, "Info", "You need to fill in all necessary fields to login.");
    return;
  }

  QString nickname = ui->nickname->text();
  QString password = ui->password->text();

  if(!m_dataBase->CheckIfNicknameExistsInTable(nickname))
  {
    QMessageBox::information(this, "Info", "This user does not exist.");
    return;
  }

  if (!m_dataBase->CheckUserDataInTable(nickname, password))
  {
    QMessageBox::information(this, "Info", "Your password is incorrect.");
    return;
  }

  auto tokenAndTime = m_dataBase->GetTokenForUser(nickname);
  if (tokenAndTime.first != "" && tokenAndTime.second != 0)
  {
    emit SuccessfulLogin(nickname, true);
  }
  else
  {
    emit SuccessfulLogin(nickname, false);
  }
}

void LoginDialog::on_createAcc_clicked()
{
  if(ui->nickname->text() == "" || ui->password->text() == "" || ui->passwordSecond->text() == "")
  {
    QMessageBox::information(this, "Info", "You need to fill in all fields to create an account.");
    return;
  }
  if(ui->password->text().length() < 6)
  {
    QMessageBox::information(this, "Info", "Your password does not satisfy the current policy requirements (length > 6).");
    ui->passwordSecond->clear();
    return;
  }
  if(ui->password->text() != ui->passwordSecond->text())
  {
    QMessageBox::information(this, "Info", "Your password and confirmation password do not match, please try again.");
    ui->passwordSecond->clear();
    return;
  }

  QString nickname = ui->nickname->text();
  QString password = ui->password->text();

  if(m_dataBase->CheckIfNicknameExistsInTable(nickname))
  {
    QMessageBox::information(this, "Info", "This nickname already exists.");
    return;
  }

  if (!m_dataBase->AddValueToTable(nickname, password))
  {
    QMessageBox::information(this, "Warn", "Something went wrong while creating your account, please try again.");
    return;
  }

  emit SuccessfulCreatingAcc(nickname);
}

void LoginDialog::disableButton(QString)
{
  if(ui->passwordSecond->text() != "")
  {
    m_loginButton->setDisabled(true);
  }
  else
  {
    m_loginButton->setEnabled(true);
  }
}
