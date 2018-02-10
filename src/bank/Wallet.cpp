
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

using namespace std;
using namespace boost::uuids;
using namespace boost::property_tree;

//----------------------------------------------------------------------

namespace ecrp {
	namespace bank {

		Wallet::Wallet() {
		}

		Wallet::~Wallet() {
		}

		void Wallet::init() {
			uuid t = random_generator()();
			_id = to_string(t);
			_filename = _id + ".json";

			//_rootKey = generateKey();
		}

		void Wallet::load() {
			ptree root;
			read_json(_filename, root);

			auto masterKey = root.get_child_optional("masterKey");
			if (masterKey) {
				auto d = masterKey.get().get_optional<string>("d");
				if (d) {
					from_string(_masterKey->d, d.get());
				}

				auto q = masterKey.get().get_optional<string>("q");
				if (q) {
					from_string(_masterKey->q, q.get());
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
				s << "\t\t\t" << "\"d\": " << "\"" << to_string(_masterKey->d) << "\"" << ",\n";
				s << "\t\t\t" << "\"q\": " << "\"" << to_string(_masterKey->q) << "\"" << "\n";
				s << "\t\t" << "},\n";
				s << "\t" << "\"addressCount\": " << _derivativeKeys.size() << "\n";
				s << "}\n";

				ofstream f(_filename);
				if (!f.fail()) {
					f << s.str();
				}
			} catch (std::exception &) {
				cerr << "WARNING: Unable to write the Registry file." << endl;
			}
		}

		void Wallet::save(const string& filename) {
			_filename = filename;
			save();
		}
	}
}