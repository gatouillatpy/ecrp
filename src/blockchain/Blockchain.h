
#pragma once

#include <list>

using std::list;

#include "MasterBlock.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace blockchain {

		class Blockchain {

		private: // MEMBERS

			list<MasterBlock*> _data;
			list<Transaction*> _pendingTransactions;

		public: // CONSTRUCTORS

			Blockchain();

			virtual ~Blockchain();

		public: // METHODS

			void init();

		private: // MEMBERS

			void load();
			void createGenesisBlock();

		};
	}
}