#ifndef REQUESTEXECUTORANDTOKENHANDLER_H
#define REQUESTEXECUTORANDTOKENHANDLER_H

#include <QObject>
#include <memory>
#include "json.hpp"
#include "oauthgettingcodesession.h"

class RequestExecutorAndTokenHandler : public QObject
{
    Q_OBJECT

    struct TokenInfo
    {
      std::string   token;
      std::uint64_t grantTime = 0;
      std::uint64_t validFor  = 2 * 60 * 60; // expiration time
    };

  public:
    RequestExecutorAndTokenHandler();

    bool GrantToken();
    TokenInfo GetTokenInfo();
    void SetToken(const std::string& token, const std::uint64_t& time);
    std::vector<std::string> GetMediaUrls();

  signals:
    void tokenExpired();

  private:
    void parseImages(const nlohmann::json&, std::vector<std::string>&);
    void parseVideos(const nlohmann::json&, std::vector<std::string>&);

  private:
    std::unique_ptr<OauthGettingCodeSession> m_oauthSession;
    TokenInfo m_tokenInfo;
};

#endif // REQUESTEXECUTORANDTOKENHANDLER_H
