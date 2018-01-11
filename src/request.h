
#pragma once

#include <deque>

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using std::string;
using std::shared_ptr;
using std::deque;
using boost::property_tree::ptree;
using boost::asio::mutable_buffers_1;
using boost::asio::buffer;

//----------------------------------------------------------------------

namespace ecrp {
    
    class session;

    class request {
        public: // CONSTANTS

            static const short MAX_LENGTH = 2048;

            static const string ADD_POINT;
            static const string REMOVE_POINT;
            static const string FIND_POINTS;

        private: // MEMBERS

            short _length;
            char _data[MAX_LENGTH];

            shared_ptr<session> _target;

            ptree _content;
            bool _contentHasBeenDeserialized;

        public: // CONSTRUCTORS

            request();

            virtual ~request();

        public: // METHODS

            bool isLengthValid() {
                return _length < MAX_LENGTH;
            }

            mutable_buffers_1 getLengthBuffer() {
                return buffer(&_length, sizeof(_length));
            }
            mutable_buffers_1 getDataBuffer() {
                return buffer(_data, _length);
            }

            const ptree &getContent();

            void setTarget(shared_ptr<session> target);
            shared_ptr<session> getTarget() {
                return _target;
            }
    };

    typedef deque<request *> request_queue;
}