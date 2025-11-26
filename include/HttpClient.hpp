#pragma once

#include "Remote.hpp"
#include "HttpUtils.hpp"

#include <string>
#include <map>
#include <memory>

namespace cpppwn {

//----------------------------------------
//
//----------------------------------------
class HttpClient {
public:
    HttpClient() : HttpClient(HttpConfig{}) {
    }
    
    explicit HttpClient(const HttpConfig& config);
    
    [[nodiscard]] HttpResponse request(
      const std::string& method,
      const std::string& url,
      const HttpHeaders& headers = {},
      const std::string& body = ""
    );
    
    [[nodiscard]] HttpResponse get(
      const std::string& url,
      const HttpHeaders& headers = {}
    );
    
    [[nodiscard]] HttpResponse post(
      const std::string& url,
      const std::string& body,
      const HttpHeaders& headers = {}
    );
    
    [[nodiscard]] HttpResponse post_form(
      const std::string& url,
      const std::map<std::string, std::string>& form_data,
      const HttpHeaders& headers = {}
    );
    
    [[nodiscard]] HttpResponse post_json(
      const std::string& url,
      const std::string& json,
      const HttpHeaders& headers = {}
    );
    
    [[nodiscard]] HttpResponse put(
      const std::string& url,
      const std::string& body,
      const HttpHeaders& headers = {}
    );
    
    [[nodiscard]] HttpResponse del(
      const std::string& url,
      const HttpHeaders& headers = {}
    );
    
    [[nodiscard]] HttpResponse head(
      const std::string& url,
      const HttpHeaders& headers = {}
    );
    
    [[nodiscard]] HttpResponse patch(
      const std::string& url,
      const std::string& body,
      const HttpHeaders& headers = {}
    );
    
    [[nodiscard]] HttpResponse options(
      const std::string& url,
      const HttpHeaders& headers = {}
    );
    
    bool download(const std::string& url, const std::string& output_path);
    
    static std::map<std::string, std::string> get_cookies(const HttpResponse& response);
    
    static HttpHeaders with_cookies(
      const HttpHeaders& headers,
      const std::map<std::string, std::string>& cookies
    );
    
    [[nodiscard]] const HttpConfig& config() const noexcept { return config_; }
    
    void set_config(const HttpConfig& config) { config_ = config; }
    
    [[nodiscard]] const std::map<std::string, std::string>& cookies() const noexcept { 
      return cookie_jar_; 
    }
    
    void set_cookies(const std::map<std::string, std::string>& cookies) {
      cookie_jar_ = cookies;
    }
    
    void clear_cookies() { 
      cookie_jar_.clear(); 
    }

private:
  std::string build_request(
    const std::string& method,
    const ParsedUrl& url,
    const HttpHeaders& headers,
    const std::string& body
  ) const;
    
  HttpConfig config_;
  std::unique_ptr<Remote> remote_;
  std::map<std::string, std::string> cookie_jar_;
};

}
