#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <memory>

namespace Ui
{
  class LoginDialog;
}

class DataBaseConnection;

class LoginDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();

    QPushButton* GetButtonForLogin();
    QPushButton* GetButtonForCreate();

  signals:
    void SuccessfulLogin(QString, bool);
    void SuccessfulCreatingAcc(QString);

  private slots:
    void on_login_clicked();
    void on_createAcc_clicked();

    void disableButton(QString);

  private:
    Ui::LoginDialog *ui;
    QPushButton     *m_loginButton;
    QPushButton     *m_createButton;

    std::unique_ptr<DataBaseConnection> m_dataBase;
};

#endif // LOGINDIALOG_H
