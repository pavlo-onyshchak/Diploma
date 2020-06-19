#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QMessageBox>
#include <QTimer>
#include <QStringListModel>
#include <QFileDialog>
#include "requestexecutorandtokenhandler.h"
#include "databaseconnection.h"
#include "downloadmanager.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), m_loginWnd(new LoginDialog)
, m_downloadManager(new DownloadManager)
, m_reqExec(new RequestExecutorAndTokenHandler)
, m_dataBase(new DataBaseConnection)
{
  ui->setupUi(this);

  QRect scr = QApplication::desktop()->screenGeometry();
  move( scr.center() - rect().center() );

  QStringList columns;
  columns << "File name" << "Extension" << "Status" << "Path";

  m_model =  new QStandardItemModel(ui->tableViewMessages);

  m_model->setHorizontalHeaderLabels(columns);

  ui->tableViewMessages->setModel(m_model);
  ui->tableViewMessages->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  connect(m_loginWnd, SIGNAL(SuccessfulLogin(QString, bool)), this, SLOT(LoginSuccessful(QString, bool)));
  connect(m_loginWnd, SIGNAL(SuccessfulCreatingAcc(QString)), this, SLOT(CreatingAccSuccessful(QString)));
  connect(m_downloadManager, SIGNAL(downloadMessage(QList<QStandardItem *>)), this, SLOT(downloadMessage(QList<QStandardItem *>)));
  connect(m_downloadManager, SIGNAL(progressChanged(int)), ui->progressBar, SLOT(setValue(int)));
  connect(m_downloadManager, SIGNAL(finished(bool)), ui->StartBACKUP, SLOT(setEnabled(bool)));
  connect(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), ui->tableViewMessages, SLOT(scrollToBottom()));
  connect(ui->actionLogin, SIGNAL(triggered(bool)), m_loginWnd, SLOT(show()));
  connect(ui->actionHelp, SIGNAL(triggered(bool)), this, SLOT(showMessageDlg()));
  connect(m_reqExec, SIGNAL(tokenExpired()), this, SLOT(changeButtonsFunctionality()));

  DisableFunctionality();
}

MainWindow::~MainWindow()
{
  delete m_reqExec;
  delete m_downloadManager;
  delete m_loginWnd;
  delete ui;
}

void MainWindow::EnableFunctionality()
{
  m_loginWnd->close();

  for(auto * widget : this->findChildren<QWidget *>())
  {
    if(widget == ui->menuBar || widget == ui->menuFile)
    {
      continue;
    }

    widget->setEnabled(true);
  }
  ui->actionLogin->setDisabled(true);
  ui->Relogin->setDisabled(true);
}

void MainWindow::EnableFunctionalityForLoggedUser()
{
  ui->LoginToInstagram->setText("ACTIVE");
  ui->LoginToInstagram->setStyleSheet("background-color: rgb(138, 226, 52);");
  ui->LoginToInstagram->setDisabled(true);
  ui->InstaLogin->setStyleSheet("border-image: url(:/img/instagramOn.png);");
}

void MainWindow::DisableFunctionality()
{
  for(auto * widget : this->findChildren<QWidget *>())
  {
    if(widget == ui->menuBar || widget == ui->menuFile)
    {
      continue;
    }

    widget->setDisabled(true);
  }
}

void MainWindow::LoginSuccessful(QString nickname, bool loggedIn)
{
  m_userNickname = nickname;
  ui->user->setText(m_userNickname);

  EnableFunctionality();

  m_loggedIn = loggedIn;
  if (m_loggedIn)
  {
    auto token = m_dataBase->GetTokenForUser(m_userNickname);
    m_reqExec->SetToken(token.first, token.second);

    EnableFunctionalityForLoggedUser();
  }
}

void MainWindow::CreatingAccSuccessful(QString nickname)
{
  m_userNickname = nickname;
  ui->user->setText(m_userNickname);

  EnableFunctionality();
}

void MainWindow::on_LoginToInstagram_clicked()
{
  if (m_reqExec->GrantToken())
  {
    auto tokenInfo = m_reqExec->GetTokenInfo();
    QString token = QString::fromUtf8(tokenInfo.token.c_str());
    if (!m_dataBase->UpdateTokenForUser(m_userNickname, token, tokenInfo.grantTime))
    {
      throw std::runtime_error("Error while updating user info in DataBase.");
    }

    m_loggedIn = true;
    EnableFunctionalityForLoggedUser();
  }
  else
  {
    QMessageBox::information(this, "Warn", "Error logging in to Instagram.");
  }
}

void MainWindow::on_SwitchAutoBackup_clicked()
{
  if(!m_autoBackupEnabled)
  {
    ui->SwitchAutoBackup->setText("AUTOBACKUP off");
    ui->SwitchAutoBackup->setStyleSheet("background-color: rgb(239, 41, 41);");
    ui->AutoBackup->setStyleSheet("border-image: url(:/img/switchOn.png);");
    m_autoBackupEnabled = true;
  }
  else
  {
    ui->SwitchAutoBackup->setText("AUTOBACKUP on");
    ui->SwitchAutoBackup->setStyleSheet("background-color: rgb(186, 189, 182);");
    ui->AutoBackup->setStyleSheet("border-image: url(:/img/switchOff.png);");
    m_autoBackupEnabled = false;
  }
}

void MainWindow::downloadMessage(QList<QStandardItem *> messages)
{
  int row    = m_model->rowCount();
  int column = 0;

  for (const auto& msg : messages)
  {
    m_model->setItem(row, column, msg);
    ++column;
  }
}

void MainWindow::on_StartBACKUP_clicked()
{
  if (ui->Storage->text() == "")
  {
    QMessageBox::information(this, "Info", "Select storage where content will be saved.");
    return;
  }
  if (!m_loggedIn)
  {
    QMessageBox::information(this, "Info", "User needs to be logged in to Instagram.");
    return;
  }

  m_model->setRowCount(0);
  ui->progressBar->setValue(0);

  auto urls = m_reqExec->GetMediaUrls();

  if (urls.at(0) == "token expired")
  {
    QMessageBox::information(this, "Info", "Error: token has been expired. Please relogin.");
  }
  else if (!urls.empty())
  {
    ui->StartBACKUP->setDisabled(true);

    QStringList urlsList;

    for(const auto& url : urls)
      urlsList.push_back(QString::fromUtf8(url.c_str()));

    m_downloadManager->append(urlsList);
    QTimer::singleShot(0, m_downloadManager, SLOT(execute()));
  }
  else
  {
    QMessageBox::information(this, "Info", "Error getting content, pleas try again later.");
  }
}

void MainWindow::on_SelectStorage_clicked()
{
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select Storage"),
                                                        "",
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if(dir != "")
  {
    dir += "/";

    m_downloadManager->setPath(dir);
    ui->Storage->setText(dir);
  }
}

void MainWindow::showMessageDlg()
{
  QMessageBox::information(this, "Info", "For more information please contact: mykhailyshyn.pavlo@gmail.com");
}

void MainWindow::on_Relogin_clicked()
{
  if (m_reqExec->GrantToken())
  {
    auto tokenInfo = m_reqExec->GetTokenInfo();
    QString token = QString::fromUtf8(tokenInfo.token.c_str());
    if (!m_dataBase->UpdateTokenForUser(m_userNickname, token, tokenInfo.grantTime))
    {
      throw std::runtime_error("Error while updating user info in DataBase.");
    }

    ui->Relogin->setDisabled(true);
    ui->LoginToInstagram->setText("ACTIVE");
    ui->LoginToInstagram->setStyleSheet("background-color: rgb(138, 226, 52);");
  }
  else
  {
    QMessageBox::information(this, "Warn", "Error relogging in to Instagram.");
  }
}

void MainWindow::changeButtonsFunctionality()
{
  ui->Relogin->setEnabled(true);
  ui->LoginToInstagram->setText("INACTIVE");
  ui->LoginToInstagram->setStyleSheet("background-color: rgb(239, 41, 41);");
}
