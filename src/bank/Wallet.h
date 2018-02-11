
#pragma once

#include <list>

#include "crypto/Crypto.h"
#include "blockchain/Transaction.h"

using std::list;
using std::string;

using namespace ecrp::crypto;
using namespace ecrp::blockchain;

//----------------------------------------------------------------------

namespace ecrp {
	namespace bank {

		class Wallet {

		public: // STATIC CONSTANTS

			static const size_t ADDRESS_SIZE_IN_BITS = 120;
			static const size_t ADDRESS_SIZE = ADDRESS_SIZE_IN_BITS >> 3;

		private: // MEMBERS

			string _id;
			string _filename;

			b512* _encryptedSecret;
			PrivateKey* _masterKey;
			list<DerivativeKey*> _derivativeKeys;

		public: // CONSTRUCTORS

			Wallet();

			virtual ~Wallet();

		public: // METHODS

			string getId();

			void init();
			void load();
			void load(const string& filename);
			void save();
			void save(const string& filename);

			bool isEncrypted();
			bool isLocked();

			void setPassword(const string& password);
			bool checkPassword(const string& password);

			string generateAddress();
			list<string> getAddresses();
			string getAddress(uint32_t addressNumber);

			Transaction* createTransaction(const string& fromAddress, const string& toAddress, int64_t amount, const string& changeAddress);
		};
	}
}