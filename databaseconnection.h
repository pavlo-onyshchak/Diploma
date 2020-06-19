#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H

#include <QSqlDatabase>
#include <QSqlQuery>

class DataBaseConnection
{
  public:
    DataBaseConnection();
    ~DataBaseConnection();

    bool Connect();
    bool isOpen();
    void CloseConnection();
    bool CheckIfNicknameExistsInTable(const QString &nickname);
    bool CheckUserDataInTable(const QString &nickname, const QString &password);
    bool AddValueToTable(const QString &nickname, const QString &password);
    bool UpdateTokenForUser(const QString &nickname, const QString &token, const std::uint64_t &time);
    std::pair<std::string, std::uint64_t> GetTokenForUser(const QString &nickname);

  private:
    std::string GenerateSalt();
    std::pair<std::string, std::string> EncryptData(const std::string& data);
    bool checkHash(const std::string& data, const std::string& salt, const std::string& hash);

  private:
    QSqlDatabase m_dataBase;

};

#endif // DATABASECONNECTION_H
