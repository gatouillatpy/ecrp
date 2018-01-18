
#pragma once

#include <vector>

using std::vector;

#include "utils/streams.h"
#include "Hash.h"
#include "TransactionInput.h"
#include "TransactionOutput.h"

using ecrp::io::be_ptr_istream;

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		// NB: transactions should be split based on their inputs' previous tx address in order to prevent double spending? 

		class Transaction {

		private: // CONSTANTS

			static const uint16_t MIN_COMPATIBLE_VERSION = 1;
			static const uint16_t CURRENT_VERSION = 1;

		private: // MEMBERS

			uint16_t _version;
			uint8_t _type;
			vector<TransactionInput*> _inputs;
			vector<TransactionOutput*> _outputs;

		public: // CONSTRUCTORS

			Transaction();
			Transaction(uint8_t type);

			virtual ~Transaction();

		public: // METHODS

			void deserialize(be_ptr_istream& stream);

			void addInput(TransactionInput* i);
			void addOutput(TransactionOutput* o);

		};
	}
}