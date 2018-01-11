
#pragma once

#include <map>

using std::string;
using std::multimap;
using std::pair;

#include "tree.h"

//----------------------------------------------------------------------

namespace ecrp {
    
    class group {
        
        private: // MEMBERS

            uint16_t _id;
            string _name;

            multimap<uint16_t, tree *> _trees;

        public: // CONSTRUCTORS

            group(uint16_t id, const string &name);

            virtual ~group();

        public: // METHODS

            const uint16_t getId() {
                return _id;
            };
            const string getName() {
                return _name;
            };

            void addTree(tree *p) {
                _trees.insert(pair<uint16_t, tree *>(p->getLevel(), p));
            };

            tree *getTreeByLevel(uint16_t level);

            multimap<uint16_t, tree *>::reverse_iterator beginIterator() {
                return _trees.rbegin();
            };
            multimap<uint16_t, tree *>::reverse_iterator endIterator() {
                return _trees.rend();
            };
    };

    class registry {
        
        public: // STATIC METHODS

            static void init(int maxPagesInMemory);

            static tree *getOrSpawnTree(uint16_t level, uint16_t groupId);
            static group *getOrCreateGroup(const string &name);
            static group *getGroupById(uint16_t id);

            static void refreshPage(tree *pTree, int64_t leafPointer);
            static void invalidatePage(tree *pTree, int64_t leafPointer);

        private: // STATIC METHODS

            static void initPagingSystem(int maxPagesInMemory);

            static tree *spawnTree(uint16_t level, uint16_t groupId);
            static group *createGroup(uint16_t id, const string &name);
    };
}