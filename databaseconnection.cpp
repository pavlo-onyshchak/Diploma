#include "databaseconnection.h"
#include "databasedata.h"

#include <iostream>
#include <QVariant>
#include <QCryptographicHash>
#include <QSqlError>

DataBaseConnection::DataBaseConnection()
{
  if(QSqlDatabase::database(g_connectionName, false).connectionName() != g_connectionName)
  {
    m_dataBase = QSqlDatabase::addDatabase("QPSQL", g_connectionName);
  }
  else
  {
    m_dataBase = QSqlDatabase::database(g_connectionName, false);
  }

  if(!Connect())
  {
    throw std::runtime_error("Cannot connect to database.");
  }
}

DataBaseConnection::~DataBaseConnection()
{
  CloseConnection();
}

bool DataBaseConnection::Connect()
{
  m_dataBase.setHostName(g_hostName);
  m_dataBase.setPort(g_port);
  m_dataBase.setDatabaseName(g_databaseName);
  m_dataBase.setUserName(g_userName);
  m_dataBase.setPassword(g_password);

  return m_dataBase.open();
}

bool DataBaseConnection::isOpen()
{
  return m_dataBase.isOpen();
}

void DataBaseConnection::CloseConnection()
{
  m_dataBase.close();
}

bool DataBaseConnection::CheckIfNicknameExistsInTable(const QString& nickname)
{
  QSqlQuery query(m_dataBase);

  query.prepare("SELECT nickname "
                "FROM " + g_tableName + " WHERE nickname=\'" + nickname + "\'");

  if (query.exec())
  {
    bool result = query.next();

    query.clear();
    return result;
  }

  std::cerr << "Query execution failed in method CheckIfNicknameExistsInTable" << std::endl;

  query.clear();
  return false;
}

bool DataBaseConnection::CheckUserDataInTable(const QString &nickname, const QString &password)
{
  QSqlQuery query(m_dataBase);

  query.prepare("SELECT nickname, salt, password "
                "FROM " + g_tableName + " WHERE nickname=\'" + nickname + "\'");

  if (query.exec())
  {
    if(query.next())
    {
      QString salt = query.value(1).toString();
      QString pass = query.value(2).toString();

      query.clear();
      return checkHash(password.toStdString(), salt.toStdString(), pass.toStdString());
    }
  }

  query.clear();
  return false;
}

bool DataBaseConnection::AddValueToTable(const QString& nickname, const QString& password)
{
  QSqlQuery query(m_dataBase);

  auto pairSaltAndPass = EncryptData(password.toStdString());

  query.prepare("INSERT INTO " + g_tableName +
                " VALUES (:nickname, :salt, :password, :token, :grant_time)");

  query.bindValue(":nickname",   nickname);
  query.bindValue(":salt",       pairSaltAndPass.first.c_str());
  query.bindValue(":password",   pairSaltAndPass.second.c_str());
  query.bindValue(":token",      "");
  query.bindValue(":grant_time", 0);

  bool queryResult = query.exec();

  query.clear();

  return queryResult;
}

bool DataBaseConnection::UpdateTokenForUser(const QString &nickname, const QString &token, const uint64_t &time)
{
  QSqlQuery query(m_dataBase);

  query.prepare("UPDATE " + g_tableName + " SET token=:token,"
                                          " grant_time=:time WHERE nickname=\'" + nickname + "\'");

  query.bindValue(":token", token);
  query.bindValue(":time",  static_cast<uint>(time));

  bool queryResult = query.exec();

  query.clear();

  return queryResult;
}

std::pair<std::string, uint64_t> DataBaseConnection::GetTokenForUser(const QString &nickname)
{
  QSqlQuery query(m_dataBase);

  query.prepare("SELECT token, grant_time "
                "FROM " + g_tableName + " WHERE nickname=\'" + nickname + "\'");

  QString       token = "";
  std::uint64_t time  = 0;

  if (query.exec())
  {
    if (query.next())
    {
      token = query.value(0).toString();
      time  = query.value(1).toUInt();
    }
  }

  query.clear();

  return { token.toStdString(), time };
}

std::string DataBaseConnection::GenerateSalt()
{
  std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

  std::random_device rd;
  std::mt19937 generator(rd());

  std::shuffle(str.begin(), str.end(), generator);

  return str.substr(0, 16);    // assumes 16 < number of characters in str
}

std::pair<std::string, std::string> DataBaseConnection::EncryptData(const std::string &data)
{
  std::string generatedSalt = GenerateSalt();

  QCryptographicHash hash(QCryptographicHash::Sha256);
  hash.addData(data.c_str(), data.size());
  hash.addData(generatedSalt.c_str(), generatedSalt.size());

  std::string encryptedData = hash.result().toHex().data();

  return { generatedSalt, encryptedData };
}

bool DataBaseConnection::checkHash(const std::string &data, const std::string &salt, const std::string &hash)
{
  QCryptographicHash cryptoHash(QCryptographicHash::Sha256);
  cryptoHash.addData(data.c_str(), data.size());
  cryptoHash.addData(salt.c_str(), salt.size());

  std::string encryptedData = cryptoHash.result().toHex().data();

  return encryptedData == hash;
}


