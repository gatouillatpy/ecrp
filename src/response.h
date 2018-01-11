
#pragma once

#include <deque>

#include <boost/asio.hpp>

using std::string;
using std::shared_ptr;
using std::ostringstream;
using std::deque;
using boost::asio::mutable_buffers_1;

#include "utils.h"

//----------------------------------------------------------------------

namespace ecrp {
    
    class session;

    class response {
        public: // CONSTANTS

            static const string STATUS_SUCCESS;
            static const string STATUS_FAILURE;

        private: // MEMBERS

            int _length;
            byte *_data;

            shared_ptr<session> _target;

            ostringstream _content;

        public: // CONSTRUCTORS

            response(shared_ptr<session> target);
            response(const response &other);

            virtual ~response();

        public: // METHODS

            mutable_buffers_1 getBuffer();

            void init(int id);
            void send();

            ostringstream &getContent() {
                return _content;
            }
    };

    typedef deque<response> response_queue;
}