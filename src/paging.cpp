
#include "registry.h"

#include <unordered_map>

#include <boost/thread/thread.hpp>

using std::binary_function;
using boost::mutex;

//----------------------------------------------------------------------

namespace ecrp {
    
    typedef struct page_index {
        int16_t treeLevel;
        int16_t treeGroupId;

        int64_t leafPointer;

        page_index(int16_t _treeLevel, int16_t _treeGroupId, int64_t _leafPointer) : treeLevel(_treeLevel), treeGroupId(_treeGroupId), leafPointer(_leafPointer) {
        }
    } *page_index_pointer;

    struct page_index_hasher {
        size_t operator()(const page_index_pointer t) const {
            return (((size_t)t->treeGroupId << 24) & 0xFF000000) | (((size_t)t->treeLevel << 16) & 0x00FF0000) | ((size_t)(t->leafPointer >> 12) & 0x0000FFFF);
        }
    };

    struct page_index_comparator : public binary_function<page_index_pointer, page_index_pointer, bool> {
        bool operator()(const page_index_pointer &__x, const page_index_pointer &__y) const {
            return __x->treeGroupId == __y->treeGroupId && __x->treeLevel == __y->treeLevel && __x->leafPointer == __y->leafPointer;
        }
    };

    struct page {
        tree *pTree;

        page_index index;

        page *pPrevious;
        page *pNext;
    };

    static unordered_map<page_index_pointer, page *, page_index_hasher, page_index_comparator> _pageIndex;

    static mutex _indexMutex;

    static page *_pagePool;
    static int *_pagePoolMapper;

    static page *_pHead;
    static page *_pTail;

    static int _availablePages;

    static bool _disabled(true);

    void registry::initPagingSystem(int maxPagesInMemory) {
        _pHead = 0;
        _pTail = 0;

        _pagePool = (page *)malloc(maxPagesInMemory * sizeof(page));
        _pagePoolMapper = (int *)malloc(maxPagesInMemory * sizeof(int));

        memset(_pagePool, 0, maxPagesInMemory * sizeof(page));

        for (int i = 0; i < maxPagesInMemory; i++) {
            _pagePoolMapper[i] = i;
        }

        _availablePages = maxPagesInMemory;

        _disabled = false;
    }

    void registry::invalidatePage(tree *pTree, int64_t leafPointer) {
        if (_disabled) {
            return;
        }

        page_index i(pTree->getLevel(), pTree->getGroupId(), leafPointer);

        _indexMutex.lock();

        auto t = _pageIndex.find(&i);

        if (t != _pageIndex.end()) {
            page *p = t->second;

            _pagePoolMapper[_availablePages] = (int)(p - &_pagePool[0]); // TO CHECK

            _availablePages++;

            if (p == _pHead) {
                if (p == _pTail) {
                    _pHead = 0;
                    _pTail = 0;
                } else {
                    p->pNext->pPrevious = 0;

                    _pHead = p->pNext;
                }
            } else if (p == _pTail) {
                p->pPrevious->pNext = 0;

                _pTail = p->pPrevious;
            } else {
                p->pPrevious->pNext = p->pNext;
                p->pNext->pPrevious = p->pPrevious;
            }

            memset(p, 0, sizeof(page));
        }

        _indexMutex.unlock();
    }

    void registry::refreshPage(tree *pTree, int64_t leafPointer) {
        if (_disabled) {
            return;
        }

        page_index i(pTree->getLevel(), pTree->getGroupId(), leafPointer);

        _indexMutex.lock();

        auto t = _pageIndex.find(&i);

        page *p;
        page *q = _pHead;

        if (t != _pageIndex.end()) {
            p = t->second;

            if (p != q) {
                if (p->pPrevious != 0) {
                    p->pPrevious->pNext = p->pNext;
                }

                if (p->pNext != 0) {
                    p->pNext->pPrevious = p->pPrevious;
                } else {
                    _pTail = p->pPrevious;
                }

                p->pPrevious = 0;
                p->pNext = q;

                q->pPrevious = p;

                _pHead = p;
            }
        } else {
            if (_availablePages > 0) {
                _availablePages--;
            } else {
                _pagePoolMapper[0] = (int)(_pTail - &_pagePool[0]); // TO CHECK

                _pTail->pTree->releaseLeafFromMemory(_pTail->index.leafPointer);

                _pTail->pPrevious->pNext = 0;
                _pTail = _pTail->pPrevious;
            }

            int k = _pagePoolMapper[_availablePages];
            _pagePoolMapper[_availablePages] = -1;

            p = &_pagePool[k];
            p->pTree = pTree;
            p->index.treeLevel = pTree->getLevel();
            p->index.treeGroupId = pTree->getGroupId();
            p->index.leafPointer = leafPointer;

            if (q != 0) {
                p->pPrevious = 0;
                p->pNext = q;

                q->pPrevious = p;

                _pHead = p;
            } else {
                p->pPrevious = 0;
                p->pNext = 0;

                _pHead = p;
                _pTail = p;
            }

            _pageIndex[&p->index] = p;
        }

        _indexMutex.unlock();
    }
}