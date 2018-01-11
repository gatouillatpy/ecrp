
#include <iostream>

#include "session.h"
#include "processor.h"

using std::cout;
using std::cerr;
using std::endl;
using std::move;
using boost::asio::ip::tcp;
using boost::asio::async_write;
using boost::system::error_code;
using boost::asio::error::eof;

//----------------------------------------------------------------------

namespace ecrp {
    
    session::session(tcp::socket socket) : _socket(move(socket)) {
    }

    session::~session() {
        cout << "Diconnected." << endl;
    }

    void session::start() {
        tcp::endpoint ep = _socket.remote_endpoint();

        cout << "Connected to " << ep.address().to_string() << ":" << ep.port() << "." << endl;

        read();
    }

    void session::send(response &res) {
        async_write(_socket, res.getBuffer(), [this](error_code e, size_t) {
            if (e) {
                cerr << "ERROR: Something bad happened." << endl;
            }
        });
    }

    void session::read() {
        request *pRequest = new request();
        pRequest->setTarget(shared_from_this());

        async_read(_socket, pRequest->getLengthBuffer(), [this, pRequest](error_code e, size_t) {
            if (!e) {
                if (pRequest->isLengthValid()) {
                    readContent(pRequest);
                } else {
                    cerr << "ERROR: The received message is too large." << endl;
                }
            } else {
                if (e.value() != eof) {
                    cerr << "ERROR: Something bad happened." << endl;
                }

                pRequest->setTarget(nullptr);
            }
        });
    }

    void session::readContent(request *pRequest) {
        async_read(_socket, pRequest->getDataBuffer(), [this, pRequest](error_code e, size_t) {
            if (!e) {
                processor::enqueueRequest(pRequest);

                read();
            } else {
                if (e.value() != eof) {
                    cerr << "ERROR: Something bad happened.\n";
                }

                pRequest->setTarget(nullptr);
            }
        });
    }
}