#ifndef OAUTHCODELISTENER_H
#define OAUTHCODELISTENER_H

#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <cpprest/oauth2.h>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features

class OauthCodeListener
{
public:
  OauthCodeListener(uri listen_uri, oauth2::experimental::oauth2_config& config) :
    m_listener(new experimental::listener::http_listener(listen_uri)),
    m_config(config)
  {
    m_listener->support([this](http::http_request request) -> void
                        {
                          std::string q = request.request_uri().query();
                          if (request.request_uri().path() == U("/") && q != U(""))
                          {
                            m_respLock.lock();

                            std::string code_param = q.find("code") != std::string::npos ? q.erase(0, 5) : "";
                            m_codeTask.set(code_param);

                            request.reply(status_codes::OK, U("Authorization code granted. "
                                                              "Now you can easily close this page and return to the program."));
                            m_respLock.unlock();
                          }
                          else
                          {
                            request.reply(status_codes::NotFound, U("Not found."));
                          }
                        });
    m_listener->open().wait();
  }
  ~OauthCodeListener()
  {
    m_listener->close().wait();
  }
  pplx::task<std::string> listenForCode()
  {
    return pplx::create_task(m_codeTask);
  }

private:
  std::unique_ptr<experimental::listener::http_listener> m_listener;
  pplx::task_completion_event<std::string>               m_codeTask;
  oauth2::experimental::oauth2_config&                   m_config;
  std::mutex                                             m_respLock;
};

#endif // OAUTHCODELISTENER_H
