
#pragma once

#include <list>

#include "crypto/Crypto.h"
#include "blockchain/Transaction.h"
#include "Wallet.h"

using std::list;
using std::string;
using ecrp::crypto::b456;
using ecrp::blockchain::Transaction;

//----------------------------------------------------------------------

namespace ecrp {
	namespace bank {

		class Bank {

		private: // MEMBERS

			list<Wallet*> _wallets;

		public: // CONSTRUCTORS

			Bank();

			virtual ~Bank();

		public: // METHODS

			void init();
			void load();

			Wallet* addWallet(Wallet* w);
			Wallet* createWalletFromPassword(const string& password);
			bool checkWalletPassword(uint32_t walletId, const string& passwordHash);
			Wallet* getWalletById(uint32_t walletId);
			b456 generateAddressForWallet(uint32_t walletId);
			list<string> getAddressesForWallet(uint32_t walletId);
			string getAddressForWallet(uint32_t walletId, b456 address);
			int64_t getBalanceForAddress(b456 address);
			Transaction* createTransaction(uint32_t walletId, b456 fromAddress, b456 toAddress, int64_t amount, b456 changeAddress);
		};
	}
}