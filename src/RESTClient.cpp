#include <RESTClient.hpp>
#include <sstream>

#include "Helpers.hpp"

namespace cpppwn {

//----------------------------------------
// Constructor
//----------------------------------------
RESTClient::RESTClient(const std::string& base_url, const HttpConfig& config)
  : base_url_(base_url), client_(config) { 
    if(not base_url_.empty() && base_url_.back() == '/') {
      base_url_.pop_back();
    }
}

//----------------------------------------
//
//----------------------------------------
void RESTClient::set_auth_bearer(const std::string& token) {
  auth_token_ = token;
  auth_type_ = AuthType::Bearer;
}

//----------------------------------------
//
//----------------------------------------
void RESTClient::set_auth_basic(const std::string& username, const std::string& password) {
  auth_username_ = username;
  auth_password_ = password;
  auth_type_ = AuthType::Basic;
}

//----------------------------------------
//
//----------------------------------------
void RESTClient::set_auth_api_key(const std::string& key, const std::string& header_name) {
  api_key_ = key;
  api_key_header_ = header_name;
  auth_type_ = AuthType::ApiKey;
}

//----------------------------------------
// Set default headers
//----------------------------------------
void RESTClient::set_header(const std::string& name, const std::string& value) {
  default_headers_[name] = value;
}

//----------------------------------------
// Build headers with auth and defaults
//----------------------------------------
HttpHeaders RESTClient::build_headers(const HttpHeaders& additional) const {
  HttpHeaders headers = default_headers_;
    
  // Add authentication
  switch (auth_type_) {
    case AuthType::Bearer:
      headers["Authorization"] = "Bearer " + auth_token_;
      break;
            
    case AuthType::Basic: {
      std::string credentials = auth_username_ + ":" + auth_password_;
      headers["Authorization"] = "Basic " + base64_encode(credentials);
      break;
    }
        
    case AuthType::ApiKey:
      headers[api_key_header_] = api_key_;
      break;
            
    case AuthType::None:
      [[fallthrough]];
    default:
      break;
    }
    
    for(const auto& [key, value] : additional) {
      headers[key] = value;
    }
    
    return headers;
}

//----------------------------------------
// Build full URL
//----------------------------------------
std::string RESTClient::build_url(const std::string& endpoint) const {
  if(endpoint.empty() || endpoint[0] != '/') {
    return base_url_ + "/" + endpoint;
  }
  return base_url_ + endpoint;
}

//----------------------------------------
// Generic request with JSON body
//----------------------------------------
std::string RESTClient::request_json(
  const std::string& method,
  const std::string& endpoint,
  const std::string& json_body,
  const HttpHeaders& headers) {
    
  auto full_headers = build_headers(headers);
  full_headers["Content-Type"] = "application/json";
    
  std::string url = build_url(endpoint);
  auto response = client_.request(method, url, full_headers, json_body);
    
  if(not response.ok()) {
    throw RESTException(response.status_code, response.status_message, response.body);
  }
    
  return response.body;
}

//----------------------------------------
// GET request (raw JSON string)
//----------------------------------------
std::string RESTClient::get(const std::string& endpoint, const HttpHeaders& headers) {
  auto full_headers = build_headers(headers);
  std::string url = build_url(endpoint);
    
  auto response = client_.get(url, full_headers);
    
  if(not response.ok()) {
    throw RESTException(response.status_code, response.status_message, response.body);
  }
    
  return response.body;
}

//----------------------------------------
// POST request (raw JSON string)
//----------------------------------------
std::string RESTClient::post(const std::string& endpoint, const std::string& json_body, const HttpHeaders& headers) { 
  return request_json("POST", endpoint, json_body, headers);
}

//----------------------------------------
// PUT request (raw JSON string)
//----------------------------------------
std::string RESTClient::put(const std::string& endpoint, const std::string& json_body, const HttpHeaders& headers) {
  return request_json("PUT", endpoint, json_body, headers);
}

//----------------------------------------
// PATCH request (raw JSON string)
//----------------------------------------
std::string RESTClient::patch(const std::string& endpoint, const std::string& json_body, const HttpHeaders& headers) {
  return request_json("PATCH", endpoint, json_body, headers);
}

//----------------------------------------
// DELETE request
//----------------------------------------
std::string RESTClient::del(const std::string& endpoint, const HttpHeaders& headers) {
  auto full_headers = build_headers(headers);
  std::string url = build_url(endpoint);
    
  auto response = client_.del(url, full_headers);
    
  if(not response.ok()) {
    throw RESTException(response.status_code, response.status_message, response.body);
  }
    
  return response.body;
}

//----------------------------------------
// List resources (GET collection)
//----------------------------------------
std::string RESTClient::list(const std::string& resource, const std::map<std::string, std::string>& query_params, const HttpHeaders& headers) {
  std::string endpoint = "/" + resource;
    
  if(not query_params.empty()) {
    endpoint += "?";
    bool first = true;

    for(const auto& [key, value] : query_params) {
      if(not first) endpoint += "&";
      endpoint += key + "=" + value;
      first = false;
    }
  }
    
  return get(endpoint, headers);
}

//----------------------------------------
// Get single resource
//----------------------------------------
std::string RESTClient::retrieve(const std::string& resource, const std::string& id, const HttpHeaders& headers) {
  return get("/" + resource + "/" + id, headers);
}

//----------------------------------------
// Create resource (POST)
//----------------------------------------
std::string RESTClient::create(const std::string& resource, const std::string& json_body, const HttpHeaders& headers) {
  return post("/" + resource, json_body, headers);
}

//----------------------------------------
// Update resource (PUT)
//----------------------------------------
std::string RESTClient::update(const std::string& resource, const std::string& id, const std::string& json_body, const HttpHeaders& headers) {
  return put("/" + resource + "/" + id, json_body, headers);
}

//----------------------------------------
// Partial update (PATCH)
//----------------------------------------
std::string RESTClient::partial_update(const std::string& resource, const std::string& id, const std::string& json_body, const HttpHeaders& headers) {
  return patch("/" + resource + "/" + id, json_body, headers);
}

//----------------------------------------
// Delete resource
//----------------------------------------
void RESTClient::destroy(const std::string& resource, const std::string& id, const HttpHeaders& headers) {
  del("/" + resource + "/" + id, headers);
}

//----------------------------------------
// Paginated request helper
//----------------------------------------
PaginatedResponse RESTClient::get_paginated(
  const std::string& endpoint,
  int page,
  int per_page,
  const HttpHeaders& headers) {
    
  std::string url = endpoint;
  url += (endpoint.find('?') != std::string::npos) ? "&" : "?";
  url += "page=" + std::to_string(page);
  url += "&per_page=" + std::to_string(per_page);
    
  auto full_headers = build_headers(headers);
  auto response = client_.get(build_url(url), full_headers);
    
  if(not response.ok()) {
    throw RESTException(response.status_code, response.status_message, response.body);
  }
    
  PaginatedResponse result;
  result.data = response.body;
  result.page = page;
  result.per_page = per_page;
    
  // Try to extract total from headers or body
  if(response.headers.count("x-total-count")) {
    result.total = std::stoi(response.headers.at("x-total-count"));
  }
    
  return result;
}

//----------------------------------------
//
//----------------------------------------
HttpClient& RESTClient::http_client() {
  return client_;
}

//----------------------------------------
//
//----------------------------------------
const HttpClient& RESTClient::http_client() const {
  return client_;
}

}
