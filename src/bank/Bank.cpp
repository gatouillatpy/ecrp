
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include "Bank.h"

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
			_wallets.push_back(w);
			return w;
		}

		Wallet* Bank::createWalletFromPassword(const string& password) {
			Wallet* w = new Wallet();
			// TODO
			return w;
		}

		bool Bank::checkWalletPassword(uint32_t walletId, const string& passwordHash) {
			return false; // TODO
		}

		Wallet* Bank::getWalletById(uint32_t walletId) {
			return NULL; // TODO
		}

		b456 Bank::generateAddressForWallet(uint32_t walletId) {
			return b456(); // TODO
		}

		list<string> Bank::getAddressesForWallet(uint32_t walletId) {
			return list<string>(); // TODO
		}

		string Bank::getAddressForWallet(uint32_t walletId, b456 address) {
			return ""; // TODO
		}

		int64_t Bank::getBalanceForAddress(b456 address) {
			return 0; // TODO
		}

		Transaction* Bank::createTransaction(uint32_t walletId, b456 fromAddress, b456 toAddress, int64_t amount, b456 changeAddress) {
			return NULL; // TODO
		}

	}
}