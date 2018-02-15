
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "Wallet.h"
#include "utils/utils.h"
#include "utils/streams.h"
#include "errors/Error.h"

using namespace std;
using namespace boost::uuids;
using namespace boost::property_tree;

//----------------------------------------------------------------------

namespace ecrp {
	namespace bank {

		Wallet::Wallet() {
			_encryptedSecret = NULL;
			_masterKey = NULL;
		}

		Wallet::~Wallet() {
		}

		string Wallet::getId() {
			return _id;
		}

		void Wallet::init() {
			uuid t = random_generator()();
			_id = to_string(t);
			_filename = _id + ".json";

			_masterKey = generateKey();
		}

		void Wallet::load() {
			ptree root;
			read_json(_filename, root);

			auto masterKey = root.get_child_optional("masterKey");
			if (masterKey) {
				auto d = masterKey.get().get_optional<string>("d");
				if (d) {
					_masterKey->d = b456(d.get());
				}

				auto q = masterKey.get().get_optional<string>("q");
				if (q) {
					_masterKey->q = b456(q.get());
				}
			}

			auto addressCount = root.get_optional<int>("addressCount");
			if (addressCount) {
				for (int k = 0; k < addressCount.get(); k++) {
					//_subKeys.push_back(deriveKey(_rootKey));
				}
			}

			// TODO: handle missing key errors
		}

		void Wallet::load(const string& filename) {
			_filename = filename;
			load();
		}

		void Wallet::save() {
			// Yup, when dealing with JSON serialization, Boost has been sucking asses for 4 years: https://svn.boost.org/trac10/ticket/9721
			// I don't want to deal with a new libs each time something doesn't work perfectly, so I'll be using simple string streams for the moment.
			// But I'm definitely moving to RapidJSON as soon as ecrp works.

			try {
				ostringstream s;
				s << "{\n";
				s << "\t" << "\"masterKey\":\n";
				s << "\t\t" << "{\n";
				s << "\t\t\t" << "\"d\": " << "\"" << _masterKey->d.toString() << "\"" << ",\n";
				s << "\t\t\t" << "\"q\": " << "\"" << _masterKey->q.toString() << "\"" << "\n";
				s << "\t\t" << "},\n";
				s << "\t" << "\"addressCount\": " << _derivativeKeys.size() << "\n";
				s << "}\n";

				ofstream f(_filename);
				if (!f.fail()) {
					f << s.str();
				}
			} catch (std::exception &) {
				throw Error("Writing to file '%s' failed.", _filename.c_str());
			}
		}

		void Wallet::save(const string& filename) {
			_filename = filename;
			save();
		}

		bool Wallet::isEncrypted() {
			return _encryptedSecret != NULL;
		}

		bool Wallet::isLocked() {
			return isEncrypted() && _masterKey == NULL;
		}

		void Wallet::setPassword(const string& password) {
			if (isLocked()) {
				throw Error("Cannot set a password if the wallet is locked.");
			}

			if (_encryptedSecret) {
				delete _encryptedSecret;
			}

			_encryptedSecret = lockKey(_masterKey, password);
		}

		bool Wallet::checkPassword(const string& password) {
			if (!isEncrypted()) {
				throw Error("Cannot check a password if the wallet is not encrypted.");
			}

			try {
				unlockKey(_encryptedSecret, password);
			} catch (std::exception &) {
				return false;
			}

			return true;
		}

		string Wallet::generateAddress() {
			if (isLocked()) {
				throw Error("Cannot generate an address if the wallet is locked.");
			}

			auto t = deriveKey(_masterKey, _derivativeKeys.size() + 1);
			_derivativeKeys.push_back(t);

			return t->q.toString().substr(0, ADDRESS_SIZE);
		}

		list<string> Wallet::getAddresses() {
			if (isLocked()) {
				throw Error("Cannot get addresses if the wallet is locked.");
			}

			list<string> output;
			std::list<int>::const_iterator iterator;
			for (auto i = _derivativeKeys.begin(); i != _derivativeKeys.end(); ++i) {
				auto t = *i;
				output.push_back(t->q.toString().substr(0, ADDRESS_SIZE));
			}
			return output;
		}

		string Wallet::getAddress(uint32_t addressNumber) {
			if (isLocked()) {
				throw Error("Cannot get address if the wallet is locked.");
			}

			if (addressNumber < 1) {
				throw Error("Cannot get the address #0, it is reserved.");
			}

			if (addressNumber > _derivativeKeys.size()) {
				throw Error("Cannot get the address #%d, only %d were generated.", addressNumber, _derivativeKeys.size());
			}

			int n = 1;
			for (auto i = _derivativeKeys.begin(); i != _derivativeKeys.end(); ++i) {
				if (n == addressNumber) {
					auto t = *i;
					return t->q.toString().substr(0, ADDRESS_SIZE);
				} else {
					n++;
				}
			}

			throw Error("Something went wrong.");
		}

		Transaction* Wallet::createTransaction(const string& fromAddress, const string& toAddress, int64_t amount, const string& changeAddress) {
			if (isLocked()) {
				throw Error("Cannot get address if the wallet is locked.");
			}

			b120 fromAddressRaw(fromAddress);

			/*for (auto i = _derivativeKeys.begin(); i != _derivativeKeys.end(); ++i) {
				auto t = *i;
				t->q

				if (n == addressNumber) {
					return to_string(t->q).substr(0, ADDRESS_SIZE);
				}
				else {
					n++;
				}
			}

			TransactionInput* i;
			i = new TransactionInput();
			i->

			Transaction t;
			t.addInput()*/
		}
	}
}