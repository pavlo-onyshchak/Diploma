#ifndef AUTHORIZATIONINFO_H
#define AUTHORIZATIONINFO_H

#include <iostream>

const std::string g_client_id            = "b45d8879a0be41fda12c5511c557c3a2";
const std::string g_client_secret        = "5b0e1849f9c04a28ba739dca6dc13e6b";
const std::string g_grant_type           = "authorization_code";
const std::string g_redirect_uri         = "http://localhost:7575";
const std::string g_scope                = "public_content";

const std::string g_instagramApiEndpoint = "https://api.instagram.com/";
const std::string g_authEndpoint         = "oauth/authorize/";
const std::string g_tokenEndpoint        = "oauth/access_token/";

const std::string g_contentType          = "application/x-www-form-urlencoded";

#endif // AUTHORIZATIONINFO_H
