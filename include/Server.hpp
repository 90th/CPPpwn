#pragma once

#include "Remote.hpp"

namespace cpppwn {

class Server {
public:
    explicit Server(uint16_t port);
    std::unique_ptr<Stream> accept();

private:
    asio::io_context io_;
    asio::ip::tcp::acceptor acceptor_;
};

}
