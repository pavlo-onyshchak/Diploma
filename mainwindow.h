#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logindialog.h"

namespace Ui
{
  class MainWindow;
}

class RequestExecutorAndTokenHandler;
class DataBaseConnection;
class DownloadManager;
class QStandardItemModel;
class QStandardItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

  private slots:
    void LoginSuccessful(QString nickname, bool loggedIn);
    void CreatingAccSuccessful(QString nickname);
    void on_LoginToInstagram_clicked();
    void on_SwitchAutoBackup_clicked();
    void downloadMessage(QList<QStandardItem *> messages);
    void on_StartBACKUP_clicked();
    void on_SelectStorage_clicked();
    void showMessageDlg();
    void on_Relogin_clicked();
    void changeButtonsFunctionality();

  private:
    void EnableFunctionality();
    void EnableFunctionalityForLoggedUser();
    void DisableFunctionality();

  private:
    Ui::MainWindow  *ui;
    LoginDialog     *m_loginWnd;
    DownloadManager *m_downloadManager;

    QString m_userNickname;

    RequestExecutorAndTokenHandler *m_reqExec = nullptr;

    std::unique_ptr<DataBaseConnection> m_dataBase;

    QStandardItemModel *m_model = nullptr;

    bool m_autoBackupEnabled = false;
    bool m_loggedIn          = false;
};

#endif // MAINWINDOW_H
