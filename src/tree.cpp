
#include <deque>
#include <cstdio>
#include <atomic>
#include <iostream>

#include <boost/thread/thread.hpp>

#include "registry.h"
#include "tree.h"

#if (defined(_MSC_VER) && (_MSC_VER > 600))
#pragma warning(disable: 4996)
#endif

//----------------------------------------------------------------------

namespace geodis
{
	class dump
	{
	public: // MEMBERS

		tree* pHolder;
		node* pTarget;
		leaf* pData;

	public: // CONSTRUCTORS

		dump(tree* _pHolder, node* _pTarget, leaf* _pData) : pHolder(_pHolder), pTarget(_pTarget), pData(_pData) {}

	public: // METHODS

		void execute();
	};

	typedef std::deque<dump> dump_queue;

//----------------------------------------------------------------------

	const uint32_t NULL_POINTER = 0;

	const int64_t UNINITIALIZED_LEAF_POINTER = -1;
	const int64_t RETIRED_LEAF_POINTER = INT64_MIN;

	const int16_t MAX_POINTS_PER_LEAF = 255;

	static void workerLoop();

	static bool _isAlive;

	static dump_queue _queue;
	static boost::mutex _queueMutex;

	static boost::condition_variable _syncPoint;
	static boost::mutex _syncMutex;

	static boost::thread _worker(&workerLoop);

	class leaf // 4096 packed bytes
	{
	public: // MEMBERS

		int16_t dirty;
		int16_t size;

	private: // MEMBERS

		uint32_t _padding0;
		uint32_t _padding1;
		uint32_t _padding2;

	public: // MEMBERS

		point points[MAX_POINTS_PER_LEAF];

	public: // CONSTRUCTORS

		leaf() : dirty(true), size(0), _padding0(0xFFFFFFFF), _padding1(0xFFFFFFFF), _padding2(0xFFFFFFFF)
		{
			memset(points, 0, sizeof(points));
		}

	public: // METHODS

		void backup(tree* pHolder, node* pTarget);
		void restore(tree* pHolder, node* pTarget);
	};

	class node
	{
	public: // MEMBERS

		int16_t dirty;

	private: // MEMBERS

		friend class leaf;
		friend class node_record;

		uint32_t _pointer;

		uint32_t _minLat;
		uint32_t _minLon;
		uint32_t _maxLat;
		uint32_t _maxLon;

		node* _nwChild;
		node* _neChild;
		node* _swChild;
		node* _seChild;

		int64_t _leafPointer;

	public: // CONSTRUCTORS

		node(uint32_t pointer, uint32_t minLat, uint32_t minLon, uint32_t maxLat, uint32_t maxLon) :
			_pointer(pointer), _minLat(minLat), _minLon(minLon), _maxLat(maxLat), _maxLon(maxLon),
			_nwChild(0), _neChild(0), _swChild(0), _seChild(0), _leafPointer(-1), dirty(true)
		{
		}

		node(uint32_t pointer) : 
			_pointer(pointer), _minLat(0), _minLon(0), _maxLat(0), _maxLon(0),
			_nwChild(0), _neChild(0), _swChild(0), _seChild(0), _leafPointer(-1), dirty(true)
		{
		}

		virtual ~node()
		{
			if (_nwChild != 0)
			{
				delete _nwChild;
				_nwChild = 0;
			}

			if (_neChild != 0)
			{
				delete _neChild;
				_neChild = 0;
			}

			if (_swChild != 0)
			{
				delete _swChild;
				_swChild = 0;
			}

			if (_seChild != 0)
			{
				delete _seChild;
				_seChild = 0;
			}
		}

	public: // METHODS

		void addPoint(tree& holder, uint32_t lat, uint32_t lon, uint64_t key)
		{
			if (_leafPointer == UNINITIALIZED_LEAF_POINTER) createLeaf(holder);
			
			if (_leafPointer != RETIRED_LEAF_POINTER)
			{
				leaf* pLeaf = holder.getLeaf(this, _leafPointer);

				auto n = pLeaf->size;

				if (n < MAX_POINTS_PER_LEAF)
				{
					point& p = pLeaf->points[n];

					p.lat = lat;
					p.lon = lon;
					p.key = key;

					pLeaf->size++;
					pLeaf->dirty = true;

					enqueueDump(&holder, pLeaf);

					registry::refreshPage(&holder, _leafPointer);
				}
				else // n == MAX_POINTS_PER_LEAF
				{
					// TODO: handle the case when too many points are in the same place,
					// limit the subdivisions to nodes with a size of 256x256,
					// and use the padding fields of leaf to store a linked list.

					registry::refreshPage(&holder, _leafPointer); // Ensure the leaf won't be released during the redistribution.

					createChildren(holder);

					for (int16_t k = 0; k < n; k++)
					{
						point& p = pLeaf->points[k];

						addPointToChild(holder, p.lat, p.lon, p.key);
					}

					removeLeaf(holder);

					_leafPointer = RETIRED_LEAF_POINTER;

					addPointToChild(holder, lat, lon, key);
				}
			}
			else // _leafPointer == RETIRED_LEAF_POINTER
			{
				addPointToChild(holder, lat, lon, key);
			}
		}

		bool removePoint(tree& holder, uint32_t lat, uint32_t lon, uint64_t key)
		{
			if (lat > _maxLat || lon > _maxLon || lat < _minLat || lon < _minLon) return false;

			bool ok = false;

			if (_leafPointer >= 0) // TODO: if => while (handle next leaves with the linked list)
			{
				leaf* pLeaf = holder.getLeaf(this, _leafPointer);

				auto n = pLeaf->size;

				for (int16_t k = 0; k < n; k++)
				{
					point& p = pLeaf->points[k];

					if (p.lat == lat && p.lon == lon && p.key == key)
					{
						n--;

						for (int16_t i = k; i < n; i++)
						{
							pLeaf->points[i] = pLeaf->points[i + 1];
						}

						pLeaf->size = n;
						pLeaf->dirty = true;

						enqueueDump(&holder, pLeaf);

						ok = true;
					}
				}

				registry::refreshPage(&holder, _leafPointer);
			}
			else if (_leafPointer == RETIRED_LEAF_POINTER)
			{
				ok = _nwChild->removePoint(holder, lat, lon, key) || ok;
				ok = _neChild->removePoint(holder, lat, lon, key) || ok;
				ok = _swChild->removePoint(holder, lat, lon, key) || ok;
				ok = _seChild->removePoint(holder, lat, lon, key) || ok;
			}

			return ok;
		}

		void findPoints(tree& holder, std::vector<full_point>& output, size_t limit, uint32_t minLat, uint32_t minLon, uint32_t maxLat, uint32_t maxLon)
		{
			if (minLat > _maxLat || minLon > _maxLon || maxLat < _minLat || maxLon < _minLon) return;

			size_t n = output.size();
			if (n >= limit) return;

			if (_leafPointer >= 0)
			{
				leaf* pLeaf = holder.getLeaf(this, _leafPointer);

				uint16_t level = holder.getLevel();
				uint16_t groupId = holder.getGroupId();

				for (int16_t k = 0; k < pLeaf->size; k++)
				{
					point& p = pLeaf->points[k];

					if (p.lat >= minLat && p.lat <= maxLat && p.lon >= minLon && p.lon <= maxLon)
					{
						output.push_back(full_point(p.lat, p.lon, level, p.key, groupId)); n++;

						if (n >= limit) return;
					}
				}

				registry::refreshPage(&holder, _leafPointer);
			}
			else if (_leafPointer == RETIRED_LEAF_POINTER)
			{
				_nwChild->findPoints(holder, output, limit, minLat, minLon, maxLat, maxLon);
				_neChild->findPoints(holder, output, limit, minLat, minLon, maxLat, maxLon);
				_swChild->findPoints(holder, output, limit, minLat, minLon, maxLat, maxLon);
				_seChild->findPoints(holder, output, limit, minLat, minLon, maxLat, maxLon);
			}
		}

		void backup(tree* pHolder);
		void restore(tree* pHolder);

	private: // METHODS

		void enqueueDump(tree* pHolder, leaf* pData)
		{
			_queueMutex.lock();
			_queue.push_back(dump(pHolder, this, pData));
			_syncPoint.notify_one();
			_queueMutex.unlock();
		}

		void addPointToChild(tree& holder, uint32_t lat, uint32_t lon, uint64_t key)
		{
			if (lat > _nwChild->_minLat)
			{
				if (lon < _nwChild->_maxLon)
					_nwChild->addPoint(holder, lat, lon, key);
				else
					_neChild->addPoint(holder, lat, lon, key);
			}
			else
			{
				if (lon < _swChild->_maxLon)
					_swChild->addPoint(holder, lat, lon, key);
				else
					_seChild->addPoint(holder, lat, lon, key);
			}
		}

		void createChildren(tree& holder)
		{
			uint32_t halfLat = ((uint64_t)_minLat + (uint64_t)_maxLat) / 2UL;
			uint32_t halfLon = ((uint64_t)_minLon + (uint64_t)_maxLon) / 2UL;

			_nwChild = holder.createNode(halfLat, _minLon, _maxLat, halfLon);
			_neChild = holder.createNode(halfLat, halfLon, _maxLat, _maxLon);
			_swChild = holder.createNode(_minLat, _minLon, halfLat, halfLon);
			_seChild = holder.createNode(_minLat, halfLon, halfLat, _maxLon);

			dirty = true;

			enqueueDump(&holder, 0);
		}

		void createLeaf(tree& holder)
		{
			_leafPointer = holder.allocateLeafPointer();

			leaf* pLeaf = holder.getLeaf(this, _leafPointer);

			dirty = true;

			enqueueDump(&holder, pLeaf);
		}

		void removeLeaf(tree& holder)
		{
			holder.releaseLeafPointer(_leafPointer);

			_leafPointer = 0;

			dirty = true;

			enqueueDump(&holder, 0);
		}
	};

	class node_record
	{
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

		void fromNode(node* pTarget)
		{
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

		void toNode(tree* pHolder, node* pTarget)
		{
			pTarget->_minLat = _minLat;
			pTarget->_minLon = _minLon;
			pTarget->_maxLat = _maxLat;
			pTarget->_maxLon = _maxLon;

			if (_nwChildPointer != NULL_POINTER)
			{
				pTarget->_nwChild = new node(_nwChildPointer);
				pTarget->_nwChild->restore(pHolder);
			}

			if (_neChildPointer != NULL_POINTER)
			{
				pTarget->_neChild = new node(_neChildPointer);
				pTarget->_neChild->restore(pHolder);
			}

			if (_swChildPointer != NULL_POINTER)
			{
				pTarget->_swChild = new node(_swChildPointer);
				pTarget->_swChild->restore(pHolder);
			}

			if (_seChildPointer != NULL_POINTER)
			{
				pTarget->_seChild = new node(_seChildPointer);
				pTarget->_seChild->restore(pHolder);
			}

			pTarget->_leafPointer = _leafPointer;
		}
	};

//----------------------------------------------------------------------

	tree::tree(uint16_t level, uint16_t groupId) : _level(level), _groupId(groupId)
	{
		boost::unique_lock<boost::shared_mutex> lock(_globalAccess);

		std::string baseFilename = "geodis_" + std::to_string(groupId) + "_" + std::to_string(level);
		std::string nodeFilename = baseFilename + ".idx";
		std::string leafFilename = baseFilename + ".dat";

		_nodeFile = fopen(nodeFilename.c_str(), "rb+");
		_leafFile = fopen(leafFilename.c_str(), "rb+");

		if (_nodeFile == 0)
		{
			_nodeFile = fopen(nodeFilename.c_str(), "wb+");

			assert(_nodeFile != 0);

			std::cout << "Created a new index file: " << nodeFilename << std::endl;

			_nextNodePointer = 0;

			_root = createNode(0, 0, UINT32_MAX, UINT32_MAX);
		}
		else
		{
			fseek(_nodeFile, 0L, SEEK_END);
			_nextNodePointer = ftell(_nodeFile);

			_root = new node(0);
			_root->restore(this);
		}

		if (_leafFile == 0)
		{
			_leafFile = fopen(leafFilename.c_str(), "wb+");

			assert(_leafFile != 0);

			std::cout << "Created a new data file: " << leafFilename << std::endl;

			_nextLeafPointer = 0;
		}
		else
		{
			fseek(_leafFile, 0L, SEEK_END);
			_nextLeafPointer = ftell(_leafFile);
		}

		_recyclerFilename = baseFilename + ".rcl";

		FILE* recyclerFile = fopen(_recyclerFilename.c_str(), "rb+");

		if (recyclerFile != 0)
		{
			fseek(recyclerFile, 0L, SEEK_END);

			int n = (int)ftell(recyclerFile) / sizeof(int64_t);

			fseek(recyclerFile, 0L, SEEK_SET);

			for (int k = 0; k < n; k++)
			{
				int64_t q;

				fread(&q, sizeof(q), 1, recyclerFile);

				_leafPointersToRecycle.push_back(q);
			}

			fclose(recyclerFile);
		}
	}

	tree::~tree()
	{
		if (_nodeFile != 0)
		{
			fflush(_nodeFile);
			fclose(_nodeFile);

			_nodeFile = 0;
		}

		if (_leafFile != 0)
		{
			fflush(_leafFile);
			fclose(_leafFile);

			_leafFile = 0;
		}
	}

	void tree::addPoint(uint32_t lat, uint32_t lon, uint64_t key)
	{
		boost::unique_lock<boost::shared_mutex> lock(_globalAccess);

		_root->addPoint(*this, lat, lon, key);
	}

	bool tree::removePoint(uint32_t lat, uint32_t lon, uint64_t key)
	{
		boost::unique_lock<boost::shared_mutex> lock(_globalAccess);

		return _root->removePoint(*this, lat, lon, key);
	}

	void tree::findPoints(std::vector<full_point>& output, size_t limit, uint32_t minLat, uint32_t minLon, uint32_t maxLat, uint32_t maxLon)
	{
		boost::shared_lock<boost::shared_mutex> lock(_globalAccess);

		_root->findPoints(*this, output, limit, minLat, minLon, maxLat, maxLon);
	}

	node* tree::createNode(uint32_t minLat, uint32_t minLon, uint32_t maxLat, uint32_t maxLon)
	{
		// No lock required here: only the backup/restore methods can use the node file, and each one acquires a unique global lock when doing so.

		uint32_t pointer = _nextNodePointer;

		_nextNodePointer += sizeof(node_record);

		return new node(pointer, minLat, minLon, maxLat, maxLon);
	}

	int64_t tree::allocateLeafPointer()
	{
		// No lock required here: only the addPoint/removePoint methods can call this method, and each one acquires a unique global lock.

		leaf* p = new leaf();
		int64_t q;

		_recyclerMutex.lock();

		if (_leafPointersToRecycle.empty())
		{
			q = _nextLeafPointer;

			_nextLeafPointer += sizeof(leaf);
		}
		else
		{
			q = _leafPointersToRecycle.front();

			_leafPointersToRecycle.pop_front();

			_recyclerDirty = true;
		}

		_recyclerMutex.unlock();

		_leafPointers.insert(std::pair<int64_t, leaf*>(q, p));

		return q;
	}

	leaf* tree::getLeaf(node* pTarget, int64_t q)
	{
		leaf* p;

		auto t = _leafPointers.find(q);

		if (t != _leafPointers.end())
		{
			p = (leaf*)t->second;
		}
		else
		{
			p = new leaf();
			p->restore(this, pTarget);

			_leafPointers.insert(std::pair<int64_t, leaf*>(q, p));
		}

		return p;
	}

	void tree::releaseLeafPointer(int64_t q)
	{
		releaseLeafFromMemory(q);

		registry::invalidatePage(this, q);

		_recyclerMutex.lock();

		_leafPointersToRecycle.push_back(q);

		_recyclerDirty = true;

		_recyclerMutex.unlock();
	}

	void tree::releaseLeafFromMemory(int64_t q)
	{
		auto t = _leafPointers.find(q);

		if (t != _leafPointers.end())
		{
			leaf* p = t->second;

			_leafPointers.erase(t);

			delete p;
		}
	}

	void tree::recyclerBackupCheck()
	{
		if (_recyclerDirty)
		{
			_recyclerMutex.lock();

			FILE* recyclerFile = fopen(_recyclerFilename.c_str(), "wb+");

			if (recyclerFile != 0)
			{
				auto ta = _leafPointersToRecycle.begin();
				auto tb = _leafPointersToRecycle.end();

				for (auto t = ta; t < tb; t++)
				{
					int64_t q = *t;

					fwrite(&q, sizeof(q), 1, recyclerFile);
				}

				fclose(recyclerFile);
			}

			_recyclerDirty = false;

			_recyclerMutex.unlock();
		}
	}

	void dump::execute()
	{
		if (pTarget->dirty)
		{
			pTarget->backup(pHolder);
			pTarget->dirty = false;
		}

		if (pData != 0 && pData->dirty)
		{
			pData->backup(pHolder, pTarget);
			pData->dirty = false;
		}

		pHolder->recyclerBackupCheck();
	}

	void leaf::backup(tree* pHolder, node* pTarget)
	{
		boost::unique_lock<boost::shared_mutex> lock(pHolder->_globalAccess);

		pHolder->_leafFileMutex.lock();

		FILE* f = pHolder->_leafFile;
		fseek(f, (long)pTarget->_leafPointer, SEEK_SET);

		fwrite(this, sizeof(leaf), 1, f);

		fflush(f);

		pHolder->_leafFileMutex.unlock();
	}

	void leaf::restore(tree* pHolder, node* pTarget)
	{
		pHolder->_leafFileMutex.lock();

		FILE* f = pHolder->_leafFile;
		fseek(f, (long)pTarget->_leafPointer, SEEK_SET);

		fread(this, sizeof(leaf), 1, f);

		dirty = false;

		pHolder->_leafFileMutex.unlock();
	}

	void node::backup(tree* pHolder)
	{
		boost::unique_lock<boost::shared_mutex> lock(pHolder->_globalAccess);

		FILE* f = pHolder->_nodeFile;
		fseek(f, _pointer, SEEK_SET);

		node_record nr;
		nr.fromNode(this);

		fwrite(&nr, sizeof(node_record), 1, f);

		fflush(f);
	}

	void node::restore(tree* pHolder)
	{
		// No need to lock here: a global unique lock is acquired by the constructor calling this method.

		FILE* f = pHolder->_nodeFile;
		fseek(f, _pointer, SEEK_SET);

		node_record nr;

		fread(&nr, sizeof(node_record), 1, f);

		nr.toNode(pHolder, this);

		dirty = false;
	}

	static void workerLoop()
	{
		_isAlive = true;

		boost::unique_lock<boost::mutex> syncLock(_syncMutex);

		while (_isAlive)
		{
			_queueMutex.lock();

			while (_queue.empty())
			{
				_queueMutex.unlock();
				_syncPoint.wait(syncLock); // TODO: put a timed wait
				_queueMutex.lock();
			}

			dump o  = _queue.front();

			_queue.pop_front();
			_queueMutex.unlock();

			o.execute();
		}
	}
}
