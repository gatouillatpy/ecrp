
#pragma once

#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_service;

#include "processor.h"
#include "session.h"

//----------------------------------------------------------------------

namespace ecrp {
    
    class server {
        
        private: // MEMBERS

            tcp::acceptor _acceptor;
            tcp::socket _socket;

        public: // CONSTRUCTORS

            server(io_service &io_service, const tcp::endpoint &endpoint);

        private: // METHODS

            void listen();
    };
}