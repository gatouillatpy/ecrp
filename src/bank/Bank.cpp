
#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include "Bank.h"
#include "errors/Error.h"

using std::runtime_error;
using namespace boost::filesystem;

//----------------------------------------------------------------------

namespace ecrp {
	namespace bank {

		Bank::Bank() {
		}

		Bank::~Bank() {
		}

		void Bank::init() {
			load();
		}

		void Bank::load() {
			path p("./wallets");
			recursive_directory_iterator s(p), e;

			BOOST_FOREACH(path const& i, std::make_pair(s, e)) {
				if (is_regular_file(i)) {
					Wallet* w = new Wallet();
					w->load(i.string()); // TODO: handle errors
					addWallet(w);
				}
			}
		}

		Wallet* Bank::addWallet(Wallet* w) {
			_wallets[w->getId()] = w;
			return w;
		}

		void Bank::setWalletPassword(const string& walletId, const string& password) {
			Wallet* w = getWalletById(walletId);
			if (w) {
				w->setPassword(password);
			} else {
				throw Error("Unable to find the wallet with id '%s'.", walletId.c_str());
			}
		}

		bool Bank::checkWalletPassword(const string& walletId, const string& password) {
			Wallet* w = getWalletById(walletId);
			if (w) {
				return w->checkPassword(password);
			} else {
				throw Error("Unable to find the wallet with id '%s'.", walletId.c_str());
			}
		}

		Wallet* Bank::getWalletById(const string& walletId) {
			auto t = _wallets.find(walletId);
			if (t != _wallets.end()) {
				return t->second;
			} else {
				return NULL;
			}
		}

		string Bank::generateAddressForWallet(const string& walletId) {
			Wallet* w = getWalletById(walletId);
			if (w) {
				return w->generateAddress();
			} else {
				throw Error("Unable to find the wallet with id '%s'.", walletId.c_str());
			}
		}

		list<string> Bank::getAddressesForWallet(const string& walletId) {
			Wallet* w = getWalletById(walletId);
			if (w) {
				return w->getAddresses();
			} else {
				throw Error("Unable to find the wallet with id '%s'.", walletId.c_str());
			}
		}

		string Bank::getAddressForWallet(const string& walletId, uint32_t addressNumber) {
			Wallet* w = getWalletById(walletId);
			if (w) {
				return w->getAddress(addressNumber);
			} else {
				throw Error("Unable to find the wallet with id '%s'.", walletId.c_str());
			}
		}

		int64_t Bank::getBalanceForAddress(string address) {
			return _blockchain->getBalanceForAddress(address);
		}

		Transaction* Bank::createTransaction(const string& walletId, const string& fromAddress, const string& toAddress, int64_t amount, const string& changeAddress) {
			Wallet* w = getWalletById(walletId);
			if (w) {
				return _blockchain->createTransaction(w->getPrivateKey(), fromAddress, toAddress, amount, changeAddress);
			} else {
				throw Error("Unable to find the wallet with id '%s'.", walletId.c_str());
			}
		}

	}
}