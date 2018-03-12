
#include "Blockchain.h"
#include "TransactionType.h"
#include "utils/utils.h"
#include "utils/streams.h"

using ecrp::io::be_file_istream;
using ecrp::io::be_ptr_istream;

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		const char* BLOCKCHAIN_FILENAME = "blocks.dat";

		Blockchain::Blockchain() {
		}

		Blockchain::~Blockchain() {
		}

		void Blockchain::init() {
			load();

			if (_data.size() == 0) {
				createGenesisBlock();
			}
		}

		void Blockchain::load() {
			be_file_istream fs;
			fs.open(BLOCKCHAIN_FILENAME);
			if (fs.is_open()) {
				while (!fs.eof()) {
					uint32_t length;
					fs.read(length);

					byte* buffer = (byte*)malloc(sizeof(byte) * length);
					fs.read(buffer, length);

					MasterBlock* mb = new MasterBlock();
					be_ptr_istream s(buffer, length);
					mb->deserialize(s);
					_data.push_back(mb);
				}
				fs.close();
			}
		}

		void Blockchain::createGenesisBlock() {
			/*uint32_t timestamp = ecrp::getUnixTimestampUTC();
			MasterBlock* mb = new MasterBlock(0, timestamp);
			Block* b = new Block(timestamp);
			Transaction* t = new Transaction(TransactionType::REWARD);
			TransactionOutput* o = new TransactionOutput();
			o->amount = 12;
			o->address = b128(); // TODO: replace with the real address
			t->addOutput(o);
			b->addTransaction(t);
			mb->addBlock(b);
			_data.push_back(mb);*/
		}
	}
}