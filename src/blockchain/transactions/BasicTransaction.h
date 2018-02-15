
#pragma once

#include <vector>

using std::vector;

#include "blockchain/Transaction.h"
#include "TransactionInput.h"
#include "TransactionOutput.h"

using ecrp::io::be_ptr_istream;

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		// NB: transactions should be split based on their inputs' previous tx address in order to prevent double spending? 

		class BasicTransaction : Transaction {

		private: // MEMBERS

			TransactionInput _input;
			vector<TransactionOutput*> _outputs;

		public: // CONSTRUCTORS

			BasicTransaction();
			virtual ~BasicTransaction();

		public: // METHODS

			void addOutput(TransactionOutput* o);

			virtual void deserialize(be_ptr_istream& stream);

		};
	}
}