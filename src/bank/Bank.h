
#pragma once

#include <unordered_map>

#include "crypto/Crypto.h"
#include "blockchain/Transaction.h"
#include "Wallet.h"

using std::unordered_map;
using std::string;
using ecrp::crypto::b456;
using ecrp::blockchain::Transaction;

//----------------------------------------------------------------------

namespace ecrp {
	namespace bank {

		class Bank {

		private: // MEMBERS

			unordered_map<string, Wallet*> _wallets;

		public: // CONSTRUCTORS

			Bank();

			virtual ~Bank();

		public: // METHODS

			void init();
			void load();

			Wallet* addWallet(Wallet* w);
			void setWalletPassword(const string& walletId, const string& password);
			bool checkWalletPassword(const string& walletId, const string& password);
			Wallet* getWalletById(const string& walletId);
			string generateAddressForWallet(const string& walletId);
			list<string> getAddressesForWallet(const string& walletId);
			string getAddressForWallet(const string& walletId, uint32_t addressNumber);
			int64_t getBalanceForAddress(string address);
			Transaction* createTransaction(const string& walletId, const string& fromAddress, const string& toAddress, int64_t amount, const string& changeAddress);
		};
	}
}