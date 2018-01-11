
#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <deque>

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp>

using std::unordered_map;
using boost::mutex;
using boost::shared_mutex;

#include "point.h"

//----------------------------------------------------------------------

namespace ecrp {
    
    class node;
    class leaf;

    class tree {
        private: // MEMBERS

            std::unordered_map<int64_t, leaf *> _leafPointers;
            std::deque<int64_t> _leafPointersToRecycle;

            uint16_t _level;
            uint16_t _groupId;

            uint32_t _nextNodePointer;
            int64_t _nextLeafPointer;

            shared_mutex _globalAccess;

            mutex _leafFileMutex;

            std::string _recyclerFilename;
            mutex _recyclerMutex;
            bool _recyclerDirty;

            FILE *_nodeFile;
            FILE *_leafFile;

            node *_root;

        public: // CONSTRUCTORS

            tree(uint16_t level, uint16_t groupId);

            virtual ~tree();

        public: // METHODS

            uint16_t getLevel() {
                return _level;
            };
            uint16_t getGroupId() {
                return _groupId;
            };

            void addPoint(uint32_t lat, uint32_t lon, uint64_t key);
            bool removePoint(uint32_t lat, uint32_t lon, uint64_t key);
            void findPoints(std::vector<full_point> &output, size_t limit, uint32_t minLat, uint32_t minLon, uint32_t maxLat, uint32_t maxLon);

            void releaseLeafFromMemory(int64_t p);

            void recyclerBackupCheck();

        private: // METHODS

            friend class leaf;
            friend class node;

            FILE *getNodeFile() {
                return _nodeFile;
            };

            node *createNode(uint32_t minLat, uint32_t minLon, uint32_t maxLat, uint32_t maxLon);

            leaf *getLeaf(node *pTarget, int64_t p);

            int64_t allocateLeafPointer();
            void releaseLeafPointer(int64_t p);

    };
}