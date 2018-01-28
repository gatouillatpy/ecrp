
#include <boost/asio.hpp>
#include <iostream>

#include "geodis/Registry.h"
#include "server/Server.h"
#include "server/Processor.h"
#include "handlers/AddPointHandler.h"
#include "handlers/RemovePointHandler.h"
#include "handlers/FindPointsHandler.h"

using std::exception;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using boost::asio::io_service;
using boost::asio::ip::tcp;

//----------------------------------------------------------------------

bool runServer = false;
int localPort = 23712;
int processorCount = 12;
int maxPagesInMemory = -1;

bool tryReadArgument(string &output, char *argv, int k) {
    try {
        output = string(&argv[k]);
        return true;
    } catch (exception &) {
        cerr << "Unable to parse argument as string after '" << argv[k - 1] << "'." << endl;
        exit(1001);
    }

    return false;
}

bool tryReadArgument(int &output, char *argv[], int k) {
    try {
        output = atoi(argv[k]);
        return true;
    } catch (exception &) {
        cerr << "Unable to parse argument as integer after '" << argv[k - 1] << "'." << endl;
        exit(1002);
    }

    return false;
}

bool tryReadArgument(double &output, char *argv[], int k) {
    try {
        output = atof(argv[k]);
        return true;
    } catch (exception &) {
        cerr << "Unable to parse argument as double after '" << argv[k - 1] << "'." << endl;
        exit(1003);
    }

    return false;
}

template<class T> bool tryReadArgument(T &output, int argc, char *argv[], int &k, const char *shortCommand, const char *longCommand) {
    if ((shortCommand != 0 && strcmp(argv[k], shortCommand) == 0) || (longCommand != 0 && strcmp(argv[k], longCommand) == 0)) {
        if (k + 1 < argc) {
            return tryReadArgument(output, argv, ++k);
        } else {
            cerr << "Missing argument after '" << argv[k] << "'." << endl;
            exit(1004);
        }
    }

    return false;
}

void parseArguments(int argc, char *argv[]) {
    for (int k = 1; k < argc; k++) {
        if (strcmp(argv[k], "run") == 0) {
            runServer = true;
            continue;
        }

        bool ok = false;
        ok = tryReadArgument(localPort, argc, argv, k, "-lp", "--localPort") || ok;
        ok = tryReadArgument(processorCount, argc, argv, k, "-pc", "--processorCount") || ok;
        ok = tryReadArgument(maxPagesInMemory, argc, argv, k, "-mp", "--maxPagesInMemory") || ok;

        if (!ok) {
            cerr << "Unknown argument '" << argv[k] << "'." << endl;
            exit(1005);
        }
    }

    if (localPort < 1024 || localPort > 65535) {
        cerr << "Argument 'localPort' out of range. It should be between 1024 and 65535." << endl;
        exit(1006);
    }

    if (processorCount < 1) {
        cerr << "Argument 'processorCount' out of range. It should be at least 1." << endl;
        exit(1007);
    }

    if (maxPagesInMemory != -1 && maxPagesInMemory < 256) {
		cerr << "Argument 'maxPagesInMemory' out of range. It should be at least 256 or -1 to disable the paging system." << endl;
        exit(1008);
    }
}

void printUsage() {
    cout << "Usage: ecrp run [OPTIONS]" << endl;
    cout << "Options:" << endl;
    cout << "  -lp / --localPort" << endl;
    cout << "  -pc / --processorCount" << endl;
    cout << "  -mp / --maxPagesInMemory" << endl;
}

#include "crypto/Crypto.h"

void testGCrypt() {
	using namespace ecrp::crypto;
	std::string data = "toto";
	PrivateKey privateKey = generateKey();
	cout << "privateKey.q: " << to_string(privateKey.q) << endl;
	cout << "privateKey.d: " << to_string(privateKey.d) << endl;
	Signature signature = signData((void*)data.c_str(), data.size(), privateKey);
	cout << "signature.r: " << to_string(signature.r) << endl;
	cout << "signature.s: " << to_string(signature.s) << endl;
	bool verified = verifyData((void*)data.c_str(), data.size(), signature, privateKey);
	cout << "verified? " << std::to_string(verified) << endl << endl;
	PrivateKey privateKey2 = deriveKey(privateKey);
	cout << "privateKey2.q: " << to_string(privateKey2.q) << endl;
	cout << "privateKey2.d: " << to_string(privateKey2.d) << endl;
	Signature signature2 = signData((void*)data.c_str(), data.size(), privateKey2);
	cout << "signature2.r: " << to_string(signature2.r) << endl;
	cout << "signature2.s: " << to_string(signature2.s) << endl;
	bool verified2 = verifyData((void*)data.c_str(), data.size(), signature2, privateKey2);
	cout << "verified2? " << std::to_string(verified2) << endl << endl;
}

int main(int argc, char *argv[]) {
	testGCrypt();
    parseArguments(argc, argv);

    if (runServer) {
        try {
            ecrp::geodis::Registry::init(maxPagesInMemory);
			ecrp::server::Processor::attachHandler(new ecrp::handlers::AddPointHandler());
			ecrp::server::Processor::attachHandler(new ecrp::handlers::RemovePointHandler());
			ecrp::server::Processor::attachHandler(new ecrp::handlers::FindPointsHandler());

            vector<ecrp::server::Processor *> processors;

            for (int k = 0; k < processorCount; k++) {
                processors.push_back(new ecrp::server::Processor());
            }

            io_service ios;
            tcp::endpoint endpoint(tcp::v4(), localPort);

            ecrp::server::Server server(ios, endpoint);

            cout << "Started ecrp server on port " << localPort << " with " << processorCount << " processors." << endl;

            ios.run();
        } catch (exception &e) {
            cerr << "EXCEPTION: " << e.what() << endl;
        }
    } else {
        printUsage();
    }

    return 0;
}