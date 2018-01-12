
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

#include "Point.h"

//----------------------------------------------------------------------

namespace ecrp {
	namespace geodis {

		class Node;
		class Leaf;

		class Tree {

		private: // MEMBERS

			std::unordered_map<int64_t, Leaf *> _leafPointers;
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

			Node *_root;

		public: // CONSTRUCTORS

			Tree(uint16_t level, uint16_t groupId);

			virtual ~Tree();

		public: // METHODS

			uint16_t getLevel() {
				return _level;
			};
			uint16_t getGroupId() {
				return _groupId;
			};

			void addPoint(uint32_t lat, uint32_t lon, uint64_t key);
			bool removePoint(uint32_t lat, uint32_t lon, uint64_t key);
			void findPoints(std::vector<FullPoint> &output, size_t limit, uint32_t minLat, uint32_t minLon, uint32_t maxLat, uint32_t maxLon);

			void releaseLeafFromMemory(int64_t p);

			void recyclerBackupCheck();

		private: // METHODS

			friend class Leaf;
			friend class Node;

			FILE *getNodeFile() {
				return _nodeFile;
			};

			Node *createNode(uint32_t minLat, uint32_t minLon, uint32_t maxLat, uint32_t maxLon);

			Leaf *getLeaf(Node *pTarget, int64_t p);

			int64_t allocateLeafPointer();
			void releaseLeafPointer(int64_t p);

		};
	}
}