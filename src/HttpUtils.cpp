#include <HttpUtils.hpp>
#include <sstream>

#include "Helpers.hpp"

//----------------------------------------
//
//----------------------------------------
std::string HttpRequest::get_header(const std::string& name) const {
  std::string lower_name = name;
  std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    
  auto it = headers.find(lower_name);
  return (it != headers.end()) ? it->second : "";
}

//----------------------------------------
//
//----------------------------------------
bool HttpRequest::has_header(const std::string& name) const {
  std::string lower_name = name;
  std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
  return headers.find(lower_name) != headers.end();
}

//----------------------------------------
//
//----------------------------------------
std::string HttpRequest::get_cookie(const std::string& name) const {
  auto it = cookies.find(name);
  return (it != cookies.end()) ? it->second : "";
}

//----------------------------------------
//
//----------------------------------------
std::string HttpRequest::get_param(const std::string& name) const {
  auto it = query_params.find(name);
  return (it != query_params.end()) ? it->second : "";
}

//----------------------------------------
//
//----------------------------------------
HttpResponse::HttpResponse(int code)
  : status_code(code), status_message(get_status_message(code)) {
}

//----------------------------------------
//
//----------------------------------------
HttpResponse& HttpResponse::set_status(int code) {
  status_code = code;
  status_message = get_status_message(code);
  return *this;
}

//----------------------------------------
//
//----------------------------------------
HttpResponse& HttpResponse::set_header(const std::string& name, const std::string& value) {
  headers[name] = value;
  return *this;
}

//----------------------------------------
//
//----------------------------------------
bool HttpResponse::ok() const noexcept {
  return status_code >= 200 && status_code < 300;
}
    
//----------------------------------------
//
//----------------------------------------
std::string HttpResponse::get_header(const std::string& key) const {
  std::string lower_key = key;
  std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
        
  auto it = headers.find(lower_key);
  return (it != headers.end()) ? it->second : "";
}
    
//----------------------------------------
//
//----------------------------------------
bool HttpResponse::has_header(const std::string& key) const {
  std::string lower_key = key;
  std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
  return headers.find(lower_key) != headers.end();
}

//----------------------------------------
//
//----------------------------------------
HttpResponse& HttpResponse::set_cookie(const std::string& name, const std::string& value,
                                       const CookieOptions& options) {
  std::ostringstream cookie;
  cookie << name << "=" << value;
    
  if(options.max_age > 0) {
    cookie << "; Max-Age=" << options.max_age;
  }
    
  if(not options.path.empty()) {
    cookie << "; Path=" << options.path;
  }
    
  if(not options.domain.empty()) {
    cookie << "; Domain=" << options.domain;
  }
    
  if(options.secure) {
    cookie << "; Secure";
  }
    
  if(options.http_only) {
    cookie << "; HttpOnly";
  }
    
  if(not options.same_site.empty()) {
    cookie << "; SameSite=" << options.same_site;
  }
    
  cookies.push_back(cookie.str());
  return *this;
}

//----------------------------------------
//
//----------------------------------------
HttpResponse& HttpResponse::set_body(const std::string& content) {
  body = content;
  set_header("Content-Length", std::to_string(body.size()));
  return *this;
}

//----------------------------------------
//
//----------------------------------------
HttpResponse& HttpResponse::set_json(const std::string& json) {
  set_header("Content-Type", "application/json");
  return set_body(json);
}

//----------------------------------------
//
//----------------------------------------
HttpResponse& HttpResponse::set_html(const std::string& html) {
  set_header("Content-Type", "text/html; charset=utf-8");
  return set_body(html);
}

//----------------------------------------
//
//----------------------------------------
HttpResponse& HttpResponse::redirect(const std::string& location, int code) {
  status_code = code;
  status_message = get_status_message(code);
  set_header("Location", location);
  return *this;
}

//----------------------------------------
//
//----------------------------------------
std::string HttpResponse::to_string() const {
  std::ostringstream response;
  response << "HTTP/1.1 " << status_code << " " << status_message << "\r\n";
  response << "Date: " << get_http_date() << "\r\n";
  response << "Server: cpppwn-http/1.0\r\n";

  // Headers
  for(const auto& [name, value] : headers) {
    response << name << ": " << value << "\r\n";
  }
    
  for(const auto& cookie : cookies) {
    response << "Set-Cookie: " << cookie << "\r\n";
  }
    
  response << "\r\n";

  // Body
  if(not body.empty()) {
    response << body;
  }
    
  return response.str();
}

//----------------------------------------
//
//----------------------------------------
std::string HttpResponse::get_status_message(int code) {
  static const std::map<int, std::string> status_messages = {
    {200, "OK"},
    {201, "Created"},
    {204, "No Content"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {307, "Temporary Redirect"},
    {308, "Permanent Redirect"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {409, "Conflict"},
    {413, "Payload Too Large"},
    {415, "Unsupported Media Type"},
    {429, "Too Many Requests"},
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"}
  };
    
  auto it = status_messages.find(code);
  return (it != status_messages.end()) ? it->second : "Unknown";
}
