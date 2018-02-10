
#pragma once

#include <list>

#include "crypto/Crypto.h"

using std::list;
using std::string;

using namespace ecrp::crypto;

//----------------------------------------------------------------------

namespace ecrp {
	namespace bank {

		class Wallet {

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

			void init();
			void load();
			void load(const string& filename);
			void save();
			void save(const string& filename);

		};
	}
}