
#include <atomic>
#include <iostream>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/thread/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "Registry.h"

using std::atomic_int;
using std::cerr;
using std::endl;
using std::ostringstream;
using std::ofstream;
using boost::mutex;
using boost::condition_variable;
using boost::thread;
using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::unique_lock;

#if (defined(_MSC_VER) && (_MSC_VER > 600))
    #pragma warning(disable: 4996)
#endif

//----------------------------------------------------------------------

namespace ecrp {
	namespace geodis {

		Group::Group(uint16_t id, const string &name) : _id(id), _name(name) {
		}

		Group::~Group() {
		}

		Tree *Group::getTreeByLevel(uint16_t level) {
			auto t = _trees.find(level);

			if (t != _trees.end()) {
				return t->second;
			}

			return 0;
		}

		//----------------------------------------------------------------------

		extern void initPagingSystem(int maxPagesInMemory);

		const char *REGISTRY_FILENAME = "Registry.json";

		static void workerLoop();

#define TREE_ID(level, groupId) ((uint32_t)(level) << 16 | (uint32_t)(groupId))

		static atomic_int NEXT_GROUP_ID(0);

		static bool _isAlive;
		static bool _isClean;

		static unordered_map<uint32_t, Tree *> _allTrees;
		static unordered_map<uint16_t, Group *> _groupsById;
		static unordered_map<string, Group *> _groupsByName;

		static condition_variable _initPoint;
		static condition_variable _syncPoint;

		static mutex _initMutex;
		static mutex _syncMutex;
		static mutex _dataMutex;

		static thread _worker(&workerLoop);

		void Registry::init(int maxPagesInMemory) {
			if (maxPagesInMemory != -1) {
				initPagingSystem(maxPagesInMemory);
			}

			FILE *f = fopen(REGISTRY_FILENAME, "r");

			if (f != 0) {
				fclose(f);

				try {
					ptree content;
					read_json(REGISTRY_FILENAME, content);

					auto nextGroupId = content.get_optional<int>("nextGroupId");
					auto groups = content.get_child_optional("groups");

					if (nextGroupId) {
						NEXT_GROUP_ID = nextGroupId.get();
					}

					if (groups) {
						BOOST_FOREACH(const ptree::value_type & w, groups.get()) {
							auto id = w.second.get_optional<int>("id");
							auto name = w.second.get_optional<string>("name");

							if (id && name) {
								Group *p = createGroup(id.get(), name.get());

								auto levels = w.second.get_child_optional("levels");

								if (levels) {
									BOOST_FOREACH(const ptree::value_type & z, levels.get()) {
										auto level = z.second.get_value_optional<int>();

										if (level) {
											spawnTree(level.get(), p->getId());
										}
									}
								}
							}
						}
					}
				}
				catch (boost::exception &) {
					cerr << "Unable to read the Registry file." << endl;
					exit(1101);
				}

				_isClean = true;
			}
			else {
				_isClean = false;
			}

			_initPoint.notify_one();
		}

		Tree *Registry::getOrSpawnTree(uint16_t level, uint16_t groupId) {
			Tree *p;

			uint32_t id = TREE_ID(level, groupId);

			_dataMutex.lock();

			auto t = _allTrees.find(id);

			if (t != _allTrees.end()) {
				p = t->second;
			}
			else {
				p = spawnTree(level, groupId);

				_isClean = false;
				_syncPoint.notify_one();
			}

			_dataMutex.unlock();

			return p;
		}

		Group *Registry::getOrCreateGroup(const string &name) {
			Group *p;

			_dataMutex.lock();

			auto t = _groupsByName.find(name);

			if (t != _groupsByName.end()) {
				p = t->second;
			}
			else {
				p = createGroup(NEXT_GROUP_ID++, name);

				_isClean = false;
				_syncPoint.notify_one();
			}

			_dataMutex.unlock();

			return p;
		}

		Group *Registry::getGroupById(uint16_t id) {
			Group *p;

			_dataMutex.lock();

			auto t = _groupsById.find(id);

			if (t != _groupsById.end()) {
				p = t->second;
			}
			else {
				p = 0;
			}

			_dataMutex.unlock();

			return p;
		}

		static void workerLoop() {
			_isAlive = true;

			unique_lock<mutex> initLock(_initMutex);
			unique_lock<mutex> syncLock(_syncMutex);

			_initPoint.wait(initLock);

			while (_isAlive) {
				while (_isClean) {
					_syncPoint.wait(syncLock);
				}

				_dataMutex.lock();

				try {
					ostringstream s;
					s << "{" << endl;
					s << "\t" << "\"nextGroupId\": " << (int)NEXT_GROUP_ID << "," << endl;
					s << "\t" << "\"groups\": " << endl;
					s << "\t" << "[" << endl;

					auto wa = _groupsById.begin();
					auto wb = _groupsById.end();

					for (auto w = wa; w != wb; ) {
						auto p = w->second; w++;

						s << "\t\t" << "{" << endl;
						s << "\t\t\t" << "\"id\": " << (int)p->getId() << "," << endl;
						s << "\t\t\t" << "\"name\": " << "\"" << p->getName() << "\"" << "," << endl;
						s << "\t\t\t" << "\"levels\": " << "[";

						auto za = p->beginIterator();
						auto zb = p->endIterator();

						for (auto z = za; z != zb; ) {
							auto i = z->first; z++;

							s << (int)i;

							if (z != zb) {
								s << ", ";
							}
						}

						s << "]" << endl;
						s << "\t\t" << "}";

						if (w != wb) {
							s << ",";
						}

						s << endl;
					}

					s << "\t" << "]" << endl;
					s << "}" << endl;

					ofstream f(REGISTRY_FILENAME);
					if (!f.fail()) {
						f << s.str();
					}
				}
				catch (std::exception &) {
					cerr << "WARNING: Unable to write the Registry file." << endl;
				}

				_isClean = true;

				_dataMutex.unlock();
			}
		}

		Tree *Registry::spawnTree(uint16_t level, uint16_t groupId) {
			Tree *p = new Tree(level, groupId);

			uint32_t id = TREE_ID(level, groupId);

			_allTrees.insert(pair<uint32_t, Tree *>(id, p));

			auto t = _groupsById.find(id);

			if (t == _groupsById.end()) {
				cerr << "Something went wrong when trying to spawn a new Tree." << endl;
				exit(1111);
			}

			t->second->addTree(p);

			return p;
		}

		Group *Registry::createGroup(uint16_t id, const string &name) {
			Group *p = new Group(id, name);

			_groupsById.insert(pair<uint16_t, Group *>(id, p));
			_groupsByName.insert(pair<string, Group *>(name, p));

			return p;
		}
	}
}