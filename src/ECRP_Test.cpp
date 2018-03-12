
#include <iostream>

#include <assert.h>

using std::exception;
using std::cout;
using std::cerr;
using std::endl;

#include "utils/utils.h"
#include "utils/varints.h"
#include "crypto/Crypto.h"
#include "errors/Error.h"

using namespace ecrp::crypto;

const bool VERBOSE = false;
const int LOOP_COUNT = 2000;
const std::string COMMON_MSG = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";


extern "C" void
__chkstk_ms()
{
}

extern "C" void
__assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
}

extern "C" void
__assert(const char *file, int line, const char *failedexpr)
{
	__assert_func(file, line, NULL, failedexpr);
}

/*void ff() {
	try {
		PrivateKey* privateKey = generateKey();
		cout << "privateKey.q: " << privateKey->q.toString() << endl;
		cout << "privateKey.d: " << privateKey->d.toString() << endl;
		Signature* signature = signData((void*)COMMON_MSG.c_str(), COMMON_MSG.size(), privateKey);
		cout << "signature.r: " << signature->r.toString() << endl;
		cout << "signature.s: " << signature->s.toString() << endl;
		bool verified = verifyData((void*)COMMON_MSG.c_str(), COMMON_MSG.size(), signature, privateKey);
		cout << "verified? " << std::to_string(verified) << endl;
		DerivativeKey* privateKey2 = deriveKey(privateKey, 12);
		cout << "privateKey2.q: " << privateKey2->q.toString() << endl;
		cout << "privateKey2.d: " << privateKey2->d.toString() << endl;
		Signature* signature2 = signData((void*)COMMON_MSG.c_str(), COMMON_MSG.size(), privateKey2);
		cout << "signature2.r: " << signature2->r.toString() << endl;
		cout << "signature2.s: " << signature2->s.toString() << endl;
		bool verified2 = verifyData((void*)COMMON_MSG.c_str(), COMMON_MSG.size(), signature2, privateKey2);
		cout << "verified2? " << std::to_string(verified2) << endl;
		std::string password = "azerty123";
		b512* encryptedSecret = lockKey(privateKey, password);
		cout << "encryptedSecret: " << encryptedSecret->toString() << endl;
		PrivateKey* privateKey3 = unlockKey(encryptedSecret, password);
		cout << "privateKey3.q: " << privateKey3->q.toString() << endl;
		cout << "privateKey3.d: " << privateKey3->d.toString() << endl;
		cout << endl;
	} catch (const ecrp::Error& e) {
		cerr << e.what() << endl;
	}
}*/

void testGCrypt256() {
	using namespace ecrp::crypto;
	b256 secret("833FE62409237B9D62EC77587520911E9A759CEC1D19755B7DA901B96DCA3D42");
	b256 msg("DDAF35A193617ABACC417349AE20413112E6FA4E89A97EA20A9EEEE64B55D39A");
	BalancedPrivateKey* privateKey = new BalancedPrivateKey();
	generateKey(privateKey, &secret, sizeof(secret), true);
	cout << "privateKey.q: " << privateKey->q.toString() << endl;
	cout << "privateKey.d: " << privateKey->d.toString() << endl;
	BalancedSignature* signature = signData(&msg, sizeof(msg), privateKey);
	cout << "signature.r: " << signature->r.toString() << endl;
	cout << "signature.s: " << signature->s.toString() << endl;
	bool verified = verifyData(&msg, sizeof(msg), signature, privateKey);
	cout << "verified? " << std::to_string(verified) << endl << endl;
}

template<class bXXX> void testKeygen(const char* algoName) {
	uint64_t t0 = ecrp::getTimestampUTC();
	cout << "Generating " << LOOP_COUNT << " keys with the " << algoName  << " algorithm..." << endl;
	for (int i = 0; i < LOOP_COUNT; i++) {
		try {
			PrivateKey<bXXX>* privateKey = generateKey<bXXX>();
			if (VERBOSE) {
				cout << "privateKey.q: " << privateKey->q.toString() << endl;
				cout << "privateKey.d: " << privateKey->d.toString() << endl;
			}
			delete privateKey;
		} catch (const ecrp::Error& e) {
			cerr << e.what() << endl;
		}
	}
	uint64_t dt = ecrp::getTimestampUTC() - t0;
	double n = (double)(1000 * LOOP_COUNT) / dt;
	cout << "Done in " << dt << " ms. (" << std::setprecision(6) << n << " keys/s)" << endl;
}

template<class bXXX> void testSign(const char* algoName) {
	// Generating a unique key.
	PrivateKey<bXXX>* privateKey = NULL;
	while (privateKey == NULL) {
		try {
			privateKey = generateKey<bXXX>();
			Signature<bXXX>* signature = signData((void*)COMMON_MSG.c_str(), COMMON_MSG.size(), privateKey);
			bool verified = verifyData((void*)COMMON_MSG.c_str(), COMMON_MSG.size(), signature, privateKey);
			if (!verified) {
				delete privateKey;
				privateKey = NULL;
			}
			delete signature;
		} catch (const ecrp::Error& e) {
			cerr << e.what() << endl;
		}
	}

	uint64_t t0 = ecrp::getTimestampUTC();
	cout << "Signing " << LOOP_COUNT << " messages with the " << algoName << " algorithm..." << endl;
	for (int i = 0; i < LOOP_COUNT; i++) {
		try {
			Signature<bXXX>* signature = signData((void*)COMMON_MSG.c_str(), COMMON_MSG.size(), privateKey);
			if (VERBOSE) {
				cout << "signature.r: " << signature->r.toString() << endl;
				cout << "signature.s: " << signature->s.toString() << endl;
			}
			delete signature;
		} catch (const ecrp::Error& e) {
			cerr << e.what() << endl;
		}
	}
	uint64_t dt = ecrp::getTimestampUTC() - t0;
	double n = (double)(1000 * LOOP_COUNT) / dt;
	cout << "Done in " << dt << " ms. (" << std::setprecision(6) << n << " signatures/s)" << endl;

	delete privateKey;
}

template<class bXXX> void testVerify(const char* algoName) {
	// Generating a unique key and signature.
	PrivateKey<bXXX>* privateKey = NULL;
	Signature<bXXX>* signature = NULL;
	while (signature == NULL) {
		try {
			privateKey = generateKey<bXXX>();
			signature = signData((void*)COMMON_MSG.c_str(), COMMON_MSG.size(), privateKey);
			bool verified = verifyData((void*)COMMON_MSG.c_str(), COMMON_MSG.size(), signature, privateKey);
			if (!verified) {
				delete signature;
				signature = NULL;
				delete privateKey;
				privateKey = NULL;
			}
		} catch (const ecrp::Error& e) {
			cerr << e.what() << endl;
		}
	}

	uint64_t t0 = ecrp::getTimestampUTC();
	cout << "Verifying " << LOOP_COUNT << " signatures with the " << algoName << " algorithm..." << endl;
	for (int i = 0; i < LOOP_COUNT; i++) {
		try {
			bool verified = verifyData((void*)COMMON_MSG.c_str(), COMMON_MSG.size(), signature, privateKey);
			if (VERBOSE) {
				cout << "verified? " << std::to_string(verified) << endl;
			}
		}
		catch (const ecrp::Error& e) {
			cerr << e.what() << endl;
		}
	}
	uint64_t dt = ecrp::getTimestampUTC() - t0;
	double n = (double)(1000 * LOOP_COUNT) / dt;
	cout << "Done in " << dt << " ms. (" << std::setprecision(6) << n << " verifications/s)" << endl;

	delete signature;
	delete privateKey;
}

void testStrongKeygen() {
	testKeygen<b456>("Ed448");
}

void testStrongSign() {
	testSign<b456>("Ed448");
}

void testStrongVerify() {
	testVerify<b456>("Ed448");
}

void testBalancedKeygen() {
	testKeygen<b256>("Ed25519");
}

void testBalancedSign() {
	testSign<b256>("Ed25519");
}

void testBalancedVerify() {
	testVerify<b256>("Ed25519");
}

void testFastKeygen() {
	testKeygen<b176>("E-168");
}

void testFastSign() {
	testSign<b176>("E-168");
}

void testFastVerify() {
	testVerify<b176>("E-168");
}

int main(int argc, char *argv[]) {
	//testGCrypt256();
	//testFastKeygen();
	//testFastSign();
	//testFastVerify();
	testBalancedKeygen();
	testBalancedSign();
	testBalancedVerify();
	//testStrongKeygen();
	//testStrongSign();
	//testStrongVerify();
	system("pause");
	return 0;
}