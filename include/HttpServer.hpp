#pragma once

#include "Server.hpp"
#include "Remote.hpp"
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <atomic>

namespace cpppwn {

//----------------------------------------
//
//----------------------------------------
struct HttpRequest {
  std::string method;                              // HTTP method (GET, POST, etc.)
  std::string path;                                // Request path (e.g., "/api/users")
  std::string http_version;                        // HTTP version (e.g., "HTTP/1.1")
  std::map<std::string, std::string> headers;      // Request headers (lowercase keys)
  std::map<std::string, std::string> query_params; // Query parameters
  std::map<std::string, std::string> cookies;      // Parsed cookies
  std::map<std::string, std::string> form_data;    // Parsed form data
  std::string body;                                // Raw request body
                                                   //
  [[nodiscard]] std::string get_header(const std::string& name) const;
    
  [[nodiscard]] bool has_header(const std::string& name) const;
    
  [[nodiscard]] std::string get_cookie(const std::string& name) const;
    
  [[nodiscard]] std::string get_param(const std::string& name) const;
};

//----------------------------------------
//
//----------------------------------------
struct CookieOptions {
  int max_age = 0;                // Max age in seconds (0 = session cookie)
  std::string path = "/";         // Cookie path
  std::string domain;             // Cookie domain
  bool secure = false;            // Secure flag (HTTPS only)
  bool http_only = true;          // HttpOnly flag (no JavaScript access)
  std::string same_site = "Lax";  // SameSite attribute (Strict, Lax, None)
};

//----------------------------------------
//
//----------------------------------------
struct HttpResponse {
  int status_code = 200;                      // HTTP status code
  std::string status_message = "OK";          // Status message
  std::map<std::string, std::string> headers; // Response headers
  std::vector<std::string> cookies;           // Set-Cookie headers
  std::string body;                           // Response body
  
  explicit HttpResponse(int code = 200);
  
  HttpResponse& set_status(int code);
  
  HttpResponse& set_header(const std::string& name, const std::string& value);
  
  HttpResponse& set_cookie(const std::string& name, const std::string& value,
                           const CookieOptions& options = {});
  
  HttpResponse& set_body(const std::string& content);
  
  HttpResponse& set_json(const std::string& json);
  
  HttpResponse& set_html(const std::string& html);
  
  HttpResponse& redirect(const std::string& location, int code = 302);
  
  [[nodiscard]] std::string to_string() const;
  
  static std::string get_status_message(int code);
};

using RouteHandler = std::function<HttpResponse(const HttpRequest&)>;

using Middleware = std::function<bool(const HttpRequest&, HttpResponse&)>;

//----------------------------------------
//
//----------------------------------------
class HttpServer {
public:
  explicit HttpServer(uint16_t port, const std::string& bind_addr = "0.0.0.0");

  explicit HttpServer(uint16_t port, const TlsConfig& tls_config,
      const std::string& bind_addr = "0.0.0.0");
  
  ~HttpServer();
  
  void route(const std::string& method, const std::string& path, RouteHandler handler);
  
  void get(const std::string& path, RouteHandler handler);
  
  void post(const std::string& path, RouteHandler handler);
  
  void put(const std::string& path, RouteHandler handler);
  
  void del(const std::string& path, RouteHandler handler);
  
  void patch(const std::string& path, RouteHandler handler);
  
  void use_middleware(Middleware middleware);
  
  void serve_static(const std::string& url_prefix, const std::string& directory);
  
  void start();
  
  void stop();
  
  [[nodiscard]] bool is_running() const noexcept;
  
  // Prevent copying
  HttpServer(const HttpServer&) = delete;
  HttpServer& operator=(const HttpServer&) = delete;
    
private:
  void handle_client(std::unique_ptr<Remote> client);
  HttpRequest parse_request(const std::string& raw_request) const;
  HttpResponse handle_static_file(const HttpRequest& request) const;
  
  std::unique_ptr<Server> server_;
  std::map<std::string, RouteHandler> routes_;
  std::vector<Middleware> middlewares_;
  std::map<std::string, std::string> static_routes_;
  std::atomic<bool> running_;
};

}
