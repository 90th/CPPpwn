#include <RESTServer.hpp>
#include <RESTClient.hpp>

#include "Helpers.hpp"

namespace cpppwn {

//----------------------------------------
//
//----------------------------------------
RESTServer::RESTServer(uint16_t port, const std::string& bind_addr): server_(port, bind_addr) {
  setup_error_handlers();
}

//----------------------------------------
//
//----------------------------------------
RESTServer::RESTServer(uint16_t port, const TlsConfig& tls_config, const std::string& bind_addr)
  : server_(port, tls_config, bind_addr) {
    setup_error_handlers();
}

//----------------------------------------
// Setup default error handlers
//----------------------------------------
void RESTServer::setup_error_handlers() {
  // 404 Not Found
  on_not_found([](const HttpRequest&) {
    return json_response(404, {
      {"error", "Not Found"},
      {"message", "The requested resource was not found"}
    });
  });
    
  // 500 Internal Server Error
  on_error([](const HttpRequest&, const std::exception& e) {
    return json_response(500, {
      {"error", "Internal Server Error"},
      {"message", std::string(e.what())}
    });
  });
}

//----------------------------------------
//
//----------------------------------------
void RESTServer::get(const std::string& path, JsonHandler handler) {
  server_.get(path, [handler, this](const HttpRequest& req) {
    return handle_json_request(req, handler);
  });
}

//----------------------------------------
//
//----------------------------------------
void RESTServer::post(const std::string& path, JsonHandler handler) {
  server_.post(path, [handler, this](const HttpRequest& req) {
    return handle_json_request(req, handler);
  });
}

//----------------------------------------
//
//----------------------------------------
void RESTServer::put(const std::string& path, JsonHandler handler) {
  server_.put(path, [handler, this](const HttpRequest& req) {
    return handle_json_request(req, handler);
  });
}

//----------------------------------------
//
//----------------------------------------
void RESTServer::del(const std::string& path, JsonHandler handler) {
  server_.del(path, [handler, this](const HttpRequest& req) {
    return handle_json_request(req, handler);
  });
}

//----------------------------------------
//
//----------------------------------------
void RESTServer::patch(const std::string& path, JsonHandler handler) {
  server_.patch(path, [handler, this](const HttpRequest& req) {
    return handle_json_request(req, handler);
  });
}

//----------------------------------------
// Handle JSON request with error handling
//----------------------------------------
HttpResponse RESTServer::handle_json_request(const HttpRequest& req, JsonHandler handler) {
  try {
    return handler(req);
  } catch (const RESTException& e) {
    return json_response(e.status_code, {
      {"error", e.status_message},
      {"message", std::string(e.what())}
    });
  } catch (const std::exception& e) {
    if(error_handler_) {
      return error_handler_(req, e);
    }
      return json_response(500, {
        {"error", "Internal Server Error"},
        {"message", std::string(e.what())}
      });
  }
}

//----------------------------------------
// RESTful resource operations
//----------------------------------------
void RESTServer::resource(const std::string& name, const ResourceHandlers& handlers) {
  std::string base_path = "/" + name;
  std::string id_path = base_path + "/:id";
    
  // LIST: GET /resource
  if(handlers.list) {
    get(base_path, [handlers](const HttpRequest& req) {
      return handlers.list(req);
    });
  }
    
  // CREATE: POST /resource
  if(handlers.create) {
    post(base_path, [handlers](const HttpRequest& req) {
      return handlers.create(req);
    });
  }
    
  // RETRIEVE: GET /resource/:id
  if(handlers.retrieve) {
    get(id_path, [handlers, name](const HttpRequest& req) {
      // Extract ID from path
      std::string id = extract_id_from_path(req.path, name);
      return handlers.retrieve(req, id);
    });
  }
    
  // UPDATE: PUT /resource/:id
  if(handlers.update) {
    put(id_path, [handlers, name](const HttpRequest& req) {
      std::string id = extract_id_from_path(req.path, name);
      return handlers.update(req, id);
    });
  }
    
  // PARTIAL_UPDATE: PATCH /resource/:id
  if(handlers.partial_update) {
    patch(id_path, [handlers, name](const HttpRequest& req) {
      std::string id = extract_id_from_path(req.path, name);
      return handlers.partial_update(req, id);
    });
  }
    
  // DELETE: DELETE /resource/:id
  if(handlers.destroy) {
    del(id_path, [handlers, name](const HttpRequest& req) {
      std::string id = extract_id_from_path(req.path, name);
      handlers.destroy(req, id);
      return json_response(204); // No Content
    });
  }
}

//----------------------------------------
// Extract ID from path like /users/123
//----------------------------------------
std::string RESTServer::extract_id_from_path(const std::string& path, const std::string& resource) {
  // Path format: /resource/id
  std::string prefix = "/" + resource + "/";
  if(path.find(prefix) == 0) {
    return path.substr(prefix.length());
  }
  return "";
}

//----------------------------------------
// Middleware
//----------------------------------------
void RESTServer::use_middleware(Middleware middleware) {
  server_.use_middleware(middleware);
}

//----------------------------------------
// CORS middleware
//----------------------------------------
void RESTServer::enable_cors(const std::string& origin, const std::string& methods, const std::string& headers) {
  use_middleware([origin, methods, headers](const HttpRequest&, HttpResponse& res) {
    res.set_header("Access-Control-Allow-Origin", origin);
    res.set_header("Access-Control-Allow-Methods", methods);
    res.set_header("Access-Control-Allow-Headers", headers);
    return true;
  });
}

//----------------------------------------
// Error handlers
//----------------------------------------
void RESTServer::on_not_found(JsonHandler handler) {
  not_found_handler_ = handler;
    
  server_.get("/404", [this](const HttpRequest& req) {
    return not_found_handler_(req);
  });
}

//----------------------------------------
//
//----------------------------------------
void RESTServer::on_error(ErrorHandler handler) {
  error_handler_ = [handler](const HttpRequest& req, const std::exception& e) {
    return handler(req, e);
  };
}

//----------------------------------------
// Static JSON response helper
//----------------------------------------
HttpResponse RESTServer::json_response(
  int status_code,
  const std::map<std::string, std::string>& data) {
    
  std::string json = "{";
  bool first = true;
  for(const auto& [key, value] : data) {
    if(not first) json += ",";
      json += "\"" + key + "\":\"" + value + "\"";
      first = false;
  }
  json += "}";
  return HttpResponse(status_code).set_json(json);
}

//----------------------------------------
// Server control
//----------------------------------------
void RESTServer::start() {
  server_.start();
}

void RESTServer::stop() {
  server_.stop();
}

bool RESTServer::is_running() const noexcept {
  return server_.is_running();
}

//----------------------------------------
// Access underlying HTTP server
//----------------------------------------
HttpServer& RESTServer::http_server() {
  return server_;
}

//----------------------------------------
//
//----------------------------------------
const HttpServer& RESTServer::http_server() const {
  return server_;
}

} // namespace cpppwn
