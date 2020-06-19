#include "requestexecutorandtokenhandler.h"
#include "authorizationinfo.h"

#include <chrono>

namespace
{
  std::uint64_t getCurrentTimeInSec()
  {
    const auto now     = std::chrono::system_clock::now();

    // transform the time into a duration since the epoch
    const auto epoch   = now.time_since_epoch();

    // cast the duration into seconds
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(epoch);

    // return the number of seconds
    return seconds.count();
  }
}

RequestExecutorAndTokenHandler::RequestExecutorAndTokenHandler() :
  m_oauthSession(new OauthGettingCodeSession(g_client_id,
                                             g_instagramApiEndpoint + g_authEndpoint,
                                             g_redirect_uri,
                                             g_scope))
{
}

bool RequestExecutorAndTokenHandler::GrantToken()
{
  std::string code = m_oauthSession->getAuthorizationCode();

  if(code == "")
    throw std::runtime_error("Error getting authorization code.");

  http_client client(U(g_instagramApiEndpoint));

  std::string grantAccessTokenBody = "client_id="      + g_client_id     +
                                     "&client_secret=" + g_client_secret +
                                     "&grant_type="    + g_grant_type    +
                                     "&redirect_uri="  + g_redirect_uri  +
                                     "&code="          + code;

  http_request req(methods::POST);

  req.set_request_uri(g_tokenEndpoint);
  req.set_body(grantAccessTokenBody);
  req.headers().set_content_type(U(g_contentType));

  pplx::task<void> tokenRequest = client.request(std::move(req)).then([this](http_response response)
  {
                                  std::string accessTokenResponse = response.extract_string().get();

                                  auto data = nlohmann::json::basic_json::parse(accessTokenResponse.c_str());

                                  m_tokenInfo.token     = data.at("access_token");
                                  m_tokenInfo.grantTime = getCurrentTimeInSec();
});

  try
  {
    tokenRequest.wait();
  }
  catch (const std::exception &e)
  {
    std::cerr << "Error. Caught an exception while getting/refreshing token: " <<  e.what() << std::endl;
    return false;
  }

  return true;
}

RequestExecutorAndTokenHandler::TokenInfo RequestExecutorAndTokenHandler::GetTokenInfo()
{
  return m_tokenInfo;
}

void RequestExecutorAndTokenHandler::SetToken(const std::string &token, const uint64_t &time)
{
  m_tokenInfo.token     = token;
  m_tokenInfo.grantTime = time;
}

std::vector<std::string> RequestExecutorAndTokenHandler::GetMediaUrls()
{
  if ((getCurrentTimeInSec() - m_tokenInfo.grantTime) > m_tokenInfo.validFor)
  {
    emit tokenExpired();

    return { "token expired" };
  }

  std::vector<std::string> urls;

  std::string mediaEndpoint = g_instagramApiEndpoint + "v1/users/self/media/recent/";
  std::string query         = "?access_token=" + m_tokenInfo.token;

  mediaEndpoint            += query;

  do
  {
    http_client client(U(mediaEndpoint));

    pplx::task<void> mediaRequest = client.request(methods::GET).then([&mediaEndpoint, &urls, this](http_response response)
    {
                                    try
                                    {
                                      std::string mediaResponse = response.extract_string().get();
                                      auto parsedMediaResponse  = nlohmann::json::basic_json::parse(mediaResponse.c_str());

                                      int statusCode = response.status_code();
                                      if (statusCode == 400)
                                      {
                                        auto meta       = parsedMediaResponse.at("meta");
                                        std::string err = meta.at("error_type");
                                        if (err == "OAuthAccessTokenException")
                                        {
                                          urls.clear();
                                          urls.push_back("token expired");

                                          return;
                                        }
                                      }

                                      mediaEndpoint = parsedMediaResponse.at("pagination").empty() ? "" : parsedMediaResponse.at("pagination").at("next_url");

                                      auto data = parsedMediaResponse.at("data");
                                      for (const auto& obj : data)
                                      {
                                        if (obj.find("images") != obj.end() && !obj.at("images").empty())
                                          parseImages(obj, urls);

                                        if (obj.find("videos") != obj.end() && !obj.at("videos").empty())
                                          parseVideos(obj, urls);

                                        if (obj.find("carousel_media") != obj.end() && !obj.at("carousel_media").empty())
                                        {
                                          auto carousel_media = obj.at("carousel_media");
                                          for (const auto& media : carousel_media)
                                          {
                                            if (media.find("images") != media.end() && !media.at("images").empty())
                                              parseImages(media, urls);

                                            if (media.find("videos") != media.end() && !media.at("videos").empty())
                                              parseVideos(media, urls);
                                          }
                                        }
                                      }
                                    }
                                    catch (const std::exception &e)
                                    {
                                      std::cerr << "Error while parsing data in method GetMediaUrls: "
                                                <<  e.what() << std::endl;
                                    }
    });

    mediaRequest.wait();

    if (urls.at(0) == "token expired")
    {
      emit tokenExpired();

      return { "token expired" };
    }

  } while(mediaEndpoint != "");

  return urls;
}

void RequestExecutorAndTokenHandler::parseImages(const nlohmann::json &data, std::vector<std::string> &urls)
{
  auto images          = data.at("images");
  auto standardImage   = images.at("standard_resolution");
  std::string imageUrl = standardImage.at("url");

  urls.push_back(std::move(imageUrl));
}

void RequestExecutorAndTokenHandler::parseVideos(const nlohmann::json &data, std::vector<std::string> &urls)
{
  auto videos          = data.at("videos");
  auto standardVideos  = videos.at("standard_resolution");
  std::string videoUrl = standardVideos.at("url");

  urls.push_back(std::move(videoUrl));
}
