
#pragma once

#include <map>

using std::string;
using std::multimap;
using std::pair;

#include "Tree.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace geodis {

		class Group {

		private: // MEMBERS

			uint16_t _id;
			string _name;

			multimap<uint16_t, Tree *> _trees;

		public: // CONSTRUCTORS

			Group(uint16_t id, const string &name);

			virtual ~Group();

		public: // METHODS

			const uint16_t getId() {
				return _id;
			};
			const string getName() {
				return _name;
			};

			void addTree(Tree *p) {
				_trees.insert(pair<uint16_t, Tree *>(p->getLevel(), p));
			};

			Tree *getTreeByLevel(uint16_t level);

			multimap<uint16_t, Tree *>::reverse_iterator beginIterator() {
				return _trees.rbegin();
			};
			multimap<uint16_t, Tree *>::reverse_iterator endIterator() {
				return _trees.rend();
			};
		};

		class Registry {

		public: // STATIC METHODS

			static void init(int maxPagesInMemory);

			static Tree *getOrSpawnTree(uint16_t level, uint16_t groupId);
			static Group *getOrCreateGroup(const string &name);
			static Group *getGroupById(uint16_t id);

			static void refreshPage(Tree *pTree, int64_t leafPointer);
			static void invalidatePage(Tree *pTree, int64_t leafPointer);

		private: // STATIC METHODS

			static void initPagingSystem(int maxPagesInMemory);

			static Tree *spawnTree(uint16_t level, uint16_t groupId);
			static Group *createGroup(uint16_t id, const string &name);
		};
	}
}