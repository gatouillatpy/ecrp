
#include <deque>
#include <cstdio>
#include <atomic>
#include <iostream>

#include <boost/thread/thread.hpp>

#include "Registry.h"
#include "Tree.h"

using std::deque;
using std::vector;
using std::to_string;
using std::cout;
using std::endl;
using boost::mutex;
using boost::condition_variable;
using boost::thread;
using boost::unique_lock;
using boost::shared_mutex;
using boost::shared_lock;

#if (defined(_MSC_VER) && (_MSC_VER > 600))
    #pragma warning(disable: 4996)
#endif

//----------------------------------------------------------------------

namespace ecrp {
	namespace geodis {

		class Dump {

		public: // MEMBERS

			Tree *pHolder;
			Node *pTarget;
			Leaf *pData;

		public: // CONSTRUCTORS

			Dump(Tree *_pHolder, Node *_pTarget, Leaf *_pData) : pHolder(_pHolder), pTarget(_pTarget), pData(_pData) {}

		public: // METHODS

			void execute();
		};

		typedef deque<Dump> dump_queue;

		//----------------------------------------------------------------------

		const uint32_t NULL_POINTER = 0;

		const int64_t UNINITIALIZED_LEAF_POINTER = -1;
		const int64_t RETIRED_LEAF_POINTER = INT64_MIN;

		const int16_t MAX_POINTS_PER_LEAF = 255;

		static void workerLoop();

		static bool _isAlive;

		static dump_queue _queue;
		static mutex _queueMutex;

		static condition_variable _syncPoint;
		static mutex _syncMutex;

		static thread _worker(&workerLoop);

		class Leaf { // 4096 packed bytes

		public: // MEMBERS

			int16_t dirty;
			int16_t size;

		private: // MEMBERS

			uint32_t _padding0;
			uint32_t _padding1;
			uint32_t _padding2;

		public: // MEMBERS

			Point points[MAX_POINTS_PER_LEAF];

		public: // CONSTRUCTORS

			Leaf() : dirty(true), size(0), _padding0(0xFFFFFFFF), _padding1(0xFFFFFFFF), _padding2(0xFFFFFFFF) {
				memset(points, 0, sizeof(points));
			}

		public: // METHODS

			void backup(Tree *pHolder, Node *pTarget);
			void restore(Tree *pHolder, Node *pTarget);
		};

		class Node {

		public: // MEMBERS

			int16_t dirty;

		private: // MEMBERS

			friend class Leaf;
			friend class NodeRecord;

			uint32_t _pointer;

			uint32_t _minLat;
			uint32_t _minLon;
			uint32_t _maxLat;
			uint32_t _maxLon;

			Node *_nwChild;
			Node *_neChild;
			Node *_swChild;
			Node *_seChild;

			int64_t _leafPointer;

		public: // CONSTRUCTORS

			Node(uint32_t pointer, uint32_t minLat, uint32_t minLon, uint32_t maxLat, uint32_t maxLon) :
				_pointer(pointer), _minLat(minLat), _minLon(minLon), _maxLat(maxLat), _maxLon(maxLon),
				_nwChild(0), _neChild(0), _swChild(0), _seChild(0), _leafPointer(-1), dirty(true) {
			}

			Node(uint32_t pointer) :
				_pointer(pointer), _minLat(0), _minLon(0), _maxLat(0), _maxLon(0),
				_nwChild(0), _neChild(0), _swChild(0), _seChild(0), _leafPointer(-1), dirty(true) {
			}

			virtual ~Node() {
				if (_nwChild != 0) {
					delete _nwChild;
					_nwChild = 0;
				}

				if (_neChild != 0) {
					delete _neChild;
					_neChild = 0;
				}

				if (_swChild != 0) {
					delete _swChild;
					_swChild = 0;
				}

				if (_seChild != 0) {
					delete _seChild;
					_seChild = 0;
				}
			}

		public: // METHODS

			void addPoint(Tree &holder, uint32_t lat, uint32_t lon, uint64_t key) {
				if (_leafPointer == UNINITIALIZED_LEAF_POINTER) {
					createLeaf(holder);
				}

				if (_leafPointer != RETIRED_LEAF_POINTER) {
					Leaf *pLeaf = holder.getLeaf(this, _leafPointer);

					auto n = pLeaf->size;

					if (n < MAX_POINTS_PER_LEAF) {
						Point &p = pLeaf->points[n];

						p.lat = lat;
						p.lon = lon;
						p.key = key;

						pLeaf->size++;
						pLeaf->dirty = true;

						enqueueDump(&holder, pLeaf);

						Registry::refreshPage(&holder, _leafPointer);
					}
					else { // n == MAX_POINTS_PER_LEAF
					 // TODO: handle the case when too many points are in the same place,
					 // limit the subdivisions to nodes with a size of 256x256,
					 // and use the padding fields of Leaf to store a linked list.

						Registry::refreshPage(&holder, _leafPointer); // Ensure the Leaf won't be released during the redistribution.

						createChildren(holder);

						for (int16_t k = 0; k < n; k++) {
							Point &p = pLeaf->points[k];

							addPointToChild(holder, p.lat, p.lon, p.key);
						}

						removeLeaf(holder);

						_leafPointer = RETIRED_LEAF_POINTER;

						addPointToChild(holder, lat, lon, key);
					}
				}
				else { // _leafPointer == RETIRED_LEAF_POINTER
					addPointToChild(holder, lat, lon, key);
				}
			}

			bool removePoint(Tree &holder, uint32_t lat, uint32_t lon, uint64_t key) {
				if (lat > _maxLat || lon > _maxLon || lat < _minLat || lon < _minLon) {
					return false;
				}

				bool ok = false;

				if (_leafPointer >= 0) { // TODO: if => while (handle next leaves with the linked list)
					Leaf *pLeaf = holder.getLeaf(this, _leafPointer);

					auto n = pLeaf->size;

					for (int16_t k = 0; k < n; k++) {
						Point &p = pLeaf->points[k];

						if (p.lat == lat && p.lon == lon && p.key == key) {
							n--;

							for (int16_t i = k; i < n; i++) {
								pLeaf->points[i] = pLeaf->points[i + 1];
							}

							pLeaf->size = n;
							pLeaf->dirty = true;

							enqueueDump(&holder, pLeaf);

							ok = true;
						}
					}

					Registry::refreshPage(&holder, _leafPointer);
				}
				else if (_leafPointer == RETIRED_LEAF_POINTER) {
					ok = _nwChild->removePoint(holder, lat, lon, key) || ok;
					ok = _neChild->removePoint(holder, lat, lon, key) || ok;
					ok = _swChild->removePoint(holder, lat, lon, key) || ok;
					ok = _seChild->removePoint(holder, lat, lon, key) || ok;
				}

				return ok;
			}

			void findPoints(Tree &holder, vector<FullPoint> &output, size_t limit, uint32_t minLat, uint32_t minLon, uint32_t maxLat, uint32_t maxLon) {
				if (minLat > _maxLat || minLon > _maxLon || maxLat < _minLat || maxLon < _minLon) {
					return;
				}

				size_t n = output.size();
				if (n >= limit) {
					return;
				}

				if (_leafPointer >= 0) {
					Leaf *pLeaf = holder.getLeaf(this, _leafPointer);

					uint16_t level = holder.getLevel();
					uint16_t groupId = holder.getGroupId();

					for (int16_t k = 0; k < pLeaf->size; k++) {
						Point &p = pLeaf->points[k];

						if (p.lat >= minLat && p.lat <= maxLat && p.lon >= minLon && p.lon <= maxLon) {
							output.push_back(FullPoint(p.lat, p.lon, level, p.key, groupId)); n++;

							if (n >= limit) {
								return;
							}
						}
					}

					Registry::refreshPage(&holder, _leafPointer);
				}
				else if (_leafPointer == RETIRED_LEAF_POINTER) {
					_nwChild->findPoints(holder, output, limit, minLat, minLon, maxLat, maxLon);
					_neChild->findPoints(holder, output, limit, minLat, minLon, maxLat, maxLon);
					_swChild->findPoints(holder, output, limit, minLat, minLon, maxLat, maxLon);
					_seChild->findPoints(holder, output, limit, minLat, minLon, maxLat, maxLon);
				}
			}

			void backup(Tree *pHolder);
			void restore(Tree *pHolder);

		private: // METHODS

			void enqueueDump(Tree *pHolder, Leaf *pData) {
				_queueMutex.lock();
				_queue.push_back(Dump(pHolder, this, pData));
				_syncPoint.notify_one();
				_queueMutex.unlock();
			}

			void addPointToChild(Tree &holder, uint32_t lat, uint32_t lon, uint64_t key) {
				if (lat > _nwChild->_minLat) {
					if (lon < _nwChild->_maxLon) {
						_nwChild->addPoint(holder, lat, lon, key);
					}
					else {
						_neChild->addPoint(holder, lat, lon, key);
					}
				}
				else {
					if (lon < _swChild->_maxLon) {
						_swChild->addPoint(holder, lat, lon, key);
					}
					else {
						_seChild->addPoint(holder, lat, lon, key);
					}
				}
			}

			void createChildren(Tree &holder) {
				uint32_t halfLat = ((uint64_t)_minLat + (uint64_t)_maxLat) / 2UL;
				uint32_t halfLon = ((uint64_t)_minLon + (uint64_t)_maxLon) / 2UL;

				_nwChild = holder.createNode(halfLat, _minLon, _maxLat, halfLon);
				_neChild = holder.createNode(halfLat, halfLon, _maxLat, _maxLon);
				_swChild = holder.createNode(_minLat, _minLon, halfLat, halfLon);
				_seChild = holder.createNode(_minLat, halfLon, halfLat, _maxLon);

				dirty = true;

				enqueueDump(&holder, 0);
			}

			void createLeaf(Tree &holder) {
				_leafPointer = holder.allocateLeafPointer();

				Leaf *pLeaf = holder.getLeaf(this, _leafPointer);

				dirty = true;

				enqueueDump(&holder, pLeaf);
			}

			void removeLeaf(Tree &holder) {
				holder.releaseLeafPointer(_leafPointer);

				_leafPointer = 0;

				dirty = true;

				enqueueDump(&holder, 0);
			}
		};

		class NodeRecord {

		private: // MEMBERS

			uint32_t _minLat;
			uint32_t _minLon;
			uint32_t _maxLat;
			uint32_t _maxLon;

			uint32_t _nwChildPointer;
			uint32_t _neChildPointer;
			uint32_t _swChildPointer;
			uint32_t _seChildPointer;

			int64_t _leafPointer;

		public: // METHODS

			void fromNode(Node *pTarget) {
				_minLat = pTarget->_minLat;
				_minLon = pTarget->_minLon;
				_maxLat = pTarget->_maxLat;
				_maxLon = pTarget->_maxLon;

				_nwChildPointer = pTarget->_nwChild != 0 ? pTarget->_nwChild->_pointer : NULL_POINTER;
				_neChildPointer = pTarget->_neChild != 0 ? pTarget->_neChild->_pointer : NULL_POINTER;
				_swChildPointer = pTarget->_swChild != 0 ? pTarget->_swChild->_pointer : NULL_POINTER;
				_seChildPointer = pTarget->_seChild != 0 ? pTarget->_seChild->_pointer : NULL_POINTER;

				_leafPointer = pTarget->_leafPointer;
			}

			void toNode(Tree *pHolder, Node *pTarget) {
				pTarget->_minLat = _minLat;
				pTarget->_minLon = _minLon;
				pTarget->_maxLat = _maxLat;
				pTarget->_maxLon = _maxLon;

				if (_nwChildPointer != NULL_POINTER) {
					pTarget->_nwChild = new Node(_nwChildPointer);
					pTarget->_nwChild->restore(pHolder);
				}

				if (_neChildPointer != NULL_POINTER) {
					pTarget->_neChild = new Node(_neChildPointer);
					pTarget->_neChild->restore(pHolder);
				}

				if (_swChildPointer != NULL_POINTER) {
					pTarget->_swChild = new Node(_swChildPointer);
					pTarget->_swChild->restore(pHolder);
				}

				if (_seChildPointer != NULL_POINTER) {
					pTarget->_seChild = new Node(_seChildPointer);
					pTarget->_seChild->restore(pHolder);
				}

				pTarget->_leafPointer = _leafPointer;
			}
		};

		//----------------------------------------------------------------------

		Tree::Tree(uint16_t level, uint16_t groupId) : _level(level), _groupId(groupId) {

			unique_lock<shared_mutex> lock(_globalAccess);

			string baseFilename = "ecrp_" + to_string(groupId) + "_" + to_string(level);
			string nodeFilename = baseFilename + ".idx";
			string leafFilename = baseFilename + ".dat";

			_nodeFile = fopen(nodeFilename.c_str(), "rb+");
			_leafFile = fopen(leafFilename.c_str(), "rb+");

			if (_nodeFile == 0) {
				_nodeFile = fopen(nodeFilename.c_str(), "wb+");

				assert(_nodeFile != 0);

				cout << "Created a new index file: " << nodeFilename << endl;

				_nextNodePointer = 0;

				_root = createNode(0, 0, UINT32_MAX, UINT32_MAX);
			}
			else {
				fseek(_nodeFile, 0L, SEEK_END);
				_nextNodePointer = ftell(_nodeFile);

				_root = new Node(0);
				_root->restore(this);
			}

			if (_leafFile == 0) {
				_leafFile = fopen(leafFilename.c_str(), "wb+");

				assert(_leafFile != 0);

				cout << "Created a new data file: " << leafFilename << endl;

				_nextLeafPointer = 0;
			}
			else {
				fseek(_leafFile, 0L, SEEK_END);
				_nextLeafPointer = ftell(_leafFile);
			}

			_recyclerFilename = baseFilename + ".rcl";

			FILE *recyclerFile = fopen(_recyclerFilename.c_str(), "rb+");

			if (recyclerFile != 0) {
				fseek(recyclerFile, 0L, SEEK_END);

				int n = (int)ftell(recyclerFile) / sizeof(int64_t);

				fseek(recyclerFile, 0L, SEEK_SET);

				for (int k = 0; k < n; k++) {
					int64_t q;

					fread(&q, sizeof(q), 1, recyclerFile);

					_leafPointersToRecycle.push_back(q);
				}

				fclose(recyclerFile);
			}
		}

		Tree::~Tree() {

			if (_nodeFile != 0) {
				fflush(_nodeFile);
				fclose(_nodeFile);

				_nodeFile = 0;
			}

			if (_leafFile != 0) {
				fflush(_leafFile);
				fclose(_leafFile);

				_leafFile = 0;
			}
		}

		void Tree::addPoint(uint32_t lat, uint32_t lon, uint64_t key) {
			unique_lock<shared_mutex> lock(_globalAccess);
			_root->addPoint(*this, lat, lon, key);
		}

		bool Tree::removePoint(uint32_t lat, uint32_t lon, uint64_t key) {
			unique_lock<shared_mutex> lock(_globalAccess);
			return _root->removePoint(*this, lat, lon, key);
		}

		void Tree::findPoints(vector<FullPoint> &output, size_t limit, uint32_t minLat, uint32_t minLon, uint32_t maxLat, uint32_t maxLon) {
			shared_lock<shared_mutex> lock(_globalAccess);
			_root->findPoints(*this, output, limit, minLat, minLon, maxLat, maxLon);
		}

		Node *Tree::createNode(uint32_t minLat, uint32_t minLon, uint32_t maxLat, uint32_t maxLon) {
			// No lock required here: only the backup/restore methods can use the Node file, and each one acquires a unique global lock when doing so.

			uint32_t pointer = _nextNodePointer;

			_nextNodePointer += sizeof(NodeRecord);

			return new Node(pointer, minLat, minLon, maxLat, maxLon);
		}

		int64_t Tree::allocateLeafPointer() {
			// No lock required here: only the addPoint/removePoint methods can call this method, and each one acquires a unique global lock.

			Leaf *p = new Leaf();
			int64_t q;

			_recyclerMutex.lock();

			if (_leafPointersToRecycle.empty()) {
				q = _nextLeafPointer;

				_nextLeafPointer += sizeof(Leaf);
			}
			else {
				q = _leafPointersToRecycle.front();

				_leafPointersToRecycle.pop_front();

				_recyclerDirty = true;
			}

			_recyclerMutex.unlock();

			_leafPointers.insert(pair<int64_t, Leaf *>(q, p));

			return q;
		}

		Leaf *Tree::getLeaf(Node *pTarget, int64_t q) {
			Leaf *p;

			auto t = _leafPointers.find(q);

			if (t != _leafPointers.end()) {
				p = (Leaf *)t->second;
			}
			else {
				p = new Leaf();
				p->restore(this, pTarget);

				_leafPointers.insert(pair<int64_t, Leaf *>(q, p));
			}

			return p;
		}

		void Tree::releaseLeafPointer(int64_t q) {
			releaseLeafFromMemory(q);

			Registry::invalidatePage(this, q);

			_recyclerMutex.lock();

			_leafPointersToRecycle.push_back(q);

			_recyclerDirty = true;

			_recyclerMutex.unlock();
		}

		void Tree::releaseLeafFromMemory(int64_t q) {
			auto t = _leafPointers.find(q);

			if (t != _leafPointers.end()) {
				Leaf *p = t->second;

				_leafPointers.erase(t);

				delete p;
			}
		}

		void Tree::recyclerBackupCheck() {
			if (_recyclerDirty) {
				_recyclerMutex.lock();

				FILE *recyclerFile = fopen(_recyclerFilename.c_str(), "wb+");

				if (recyclerFile != 0) {
					auto ta = _leafPointersToRecycle.begin();
					auto tb = _leafPointersToRecycle.end();

					for (auto t = ta; t < tb; t++) {
						int64_t q = *t;

						fwrite(&q, sizeof(q), 1, recyclerFile);
					}

					fclose(recyclerFile);
				}

				_recyclerDirty = false;

				_recyclerMutex.unlock();
			}
		}

		void Dump::execute() {
			if (pTarget->dirty) {
				pTarget->backup(pHolder);
				pTarget->dirty = false;
			}

			if (pData != 0 && pData->dirty) {
				pData->backup(pHolder, pTarget);
				pData->dirty = false;
			}

			pHolder->recyclerBackupCheck();
		}

		void Leaf::backup(Tree *pHolder, Node *pTarget) {
			unique_lock<shared_mutex> lock(pHolder->_globalAccess);

			pHolder->_leafFileMutex.lock();

			FILE *f = pHolder->_leafFile;
			fseek(f, (long)pTarget->_leafPointer, SEEK_SET);

			fwrite(this, sizeof(Leaf), 1, f);

			fflush(f);

			pHolder->_leafFileMutex.unlock();
		}

		void Leaf::restore(Tree *pHolder, Node *pTarget) {
			pHolder->_leafFileMutex.lock();

			FILE *f = pHolder->_leafFile;
			fseek(f, (long)pTarget->_leafPointer, SEEK_SET);

			fread(this, sizeof(Leaf), 1, f);

			dirty = false;

			pHolder->_leafFileMutex.unlock();
		}

		void Node::backup(Tree *pHolder) {
			unique_lock<shared_mutex> lock(pHolder->_globalAccess);

			FILE *f = pHolder->_nodeFile;
			fseek(f, _pointer, SEEK_SET);

			NodeRecord nr;
			nr.fromNode(this);

			fwrite(&nr, sizeof(NodeRecord), 1, f);

			fflush(f);
		}

		void Node::restore(Tree *pHolder) {
			// No need to lock here: a global unique lock is acquired by the constructor calling this method.

			FILE *f = pHolder->_nodeFile;
			fseek(f, _pointer, SEEK_SET);

			NodeRecord nr;

			fread(&nr, sizeof(NodeRecord), 1, f);

			nr.toNode(pHolder, this);

			dirty = false;
		}

		static void workerLoop() {
			_isAlive = true;

			unique_lock<mutex> syncLock(_syncMutex);

			while (_isAlive) {
				_queueMutex.lock();

				while (_queue.empty()) {
					_queueMutex.unlock();
					_syncPoint.wait(syncLock); // TODO: put a timed wait
					_queueMutex.lock();
				}

				Dump o = _queue.front();

				_queue.pop_front();
				_queueMutex.unlock();

				o.execute();
			}
		}
	}
}