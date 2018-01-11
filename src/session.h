
#pragma once

#include <boost/asio.hpp>

using std::enable_shared_from_this;
using boost::asio::ip::tcp;

#include "request.h"
#include "response.h"

//----------------------------------------------------------------------

namespace ecrp {
    
    class session : public enable_shared_from_this<session> {
        
        private: // MEMBERS

            tcp::socket _socket;

        public: // CONSTRUCTORS

            session(tcp::socket socket);

            virtual ~session();

        public: // METHODS

            void start();

            void send(response &res);

        private: // METHODS

            void read();
            void readContent(request *pRequest);
    };
}