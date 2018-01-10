
#pragma once

#include <map>

#include "tree.h"

//----------------------------------------------------------------------

namespace geodis
{
	class group
	{
	private: // MEMBERS

		uint16_t _id;
		std::string _name;

		std::multimap<uint16_t, tree*> _trees;

	public: // CONSTRUCTORS

		group(uint16_t id, const std::string& name);

		virtual ~group();

	public: // METHODS

		const uint16_t getId() { return _id; };
		const std::string getName() { return _name; };

		void addTree(tree* p) { _trees.insert(std::pair<uint16_t, tree*>(p->getLevel(), p)); };

		tree* getTreeByLevel(uint16_t level);

		std::multimap<uint16_t, tree*>::reverse_iterator beginIterator() { return _trees.rbegin(); };
		std::multimap<uint16_t, tree*>::reverse_iterator endIterator() { return _trees.rend(); };
	};

	class registry
	{
	public: // STATIC METHODS

		static void init(int maxPagesInMemory);

		static tree* getOrSpawnTree(uint16_t level, uint16_t groupId);
		static group* getOrCreateGroup(const std::string& name);
		static group* getGroupById(uint16_t id);

		static void refreshPage(tree* pTree, int64_t leafPointer);
		static void invalidatePage(tree* pTree, int64_t leafPointer);

	private: // STATIC METHODS

		static void initPagingSystem(int maxPagesInMemory);

		static tree* spawnTree(uint16_t level, uint16_t groupId);
		static group* createGroup(uint16_t id, const std::string& name);
	};
}
