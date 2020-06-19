#ifndef OAUTHGETTINGCODESESSION_H
#define OAUTHGETTINGCODESESSION_H

#include "oauthcodelistener.h"
#include <QDesktopServices>
#include <QUrl>

class OauthGettingCodeSession
{
  public:
    OauthGettingCodeSession(utility::string_t client_id,
                            utility::string_t auth_endpoint,
                            utility::string_t callback_uri,
                            utility::string_t scope) :
      m_oauthConfig(client_id,
                    "",
                    auth_endpoint,
                    "",
                    callback_uri,
                    scope),
      m_listener(new OauthCodeListener(callback_uri, m_oauthConfig))
    {}

    std::string getAuthorizationCode()
    {
      return doAuthorization().get();
    }

  private:
    pplx::task<std::string> doAuthorization()
    {
      if (openBrowserAuth())
      {
        return m_listener->listenForCode();
      }
      else
      {
        return pplx::create_task([]() -> std::string {return "";});
      }
    }

    bool openBrowserAuth()
    {
      std::string auth_uri = m_oauthConfig.build_authorization_uri(false);

      ucout << "Opening browser in URI:" << std::endl;
      ucout << auth_uri << std::endl;

      return QDesktopServices::openUrl(QUrl(QString::fromUtf8(auth_uri.c_str()), QUrl::TolerantMode));
    }

    oauth2::experimental::oauth2_config m_oauthConfig;
    std::unique_ptr<OauthCodeListener>  m_listener;
};

#endif // OAUTHGETTINGCODESESSION_H
