#ifndef BST_h
#define BST_h

#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include "mem_access.h"

using namespace std;

struct Interval {
	uint64_t start, end;

	bool operator==(const Interval& a) const {
		return (start == a.start && end == a.end);
	}
	bool operator<(const Interval& a) const {
		return (start < a.start);
	}
	bool operator>(const Interval& a) const {
		return (start > a.start);
	}
};

class IntervalChunk {
public:
	IntervalChunk() {
		num_of_intervals = 0;
		next_chunk = NULL;
		chunk = new Interval[DEFAULT_CAPACITY];
	}

	~IntervalChunk() {
		delete[] chunk;
		chunk = NULL;
		next_chunk = NULL;
		num_of_intervals = 0;
	}

	bool push_interval(Interval &interval) {
		if (num_of_intervals == DEFAULT_CAPACITY) return false;
		
		chunk[num_of_intervals++] = interval;
		return true;
	}

	void set_next_chunk_to(IntervalChunk* next) {
		next_chunk = next;
	}

	uint32_t size() {
		return num_of_intervals;
	}

	
	Interval& back() {
		// assume num_of_intervals > 0
		return chunk[num_of_intervals - 1];
	}
	
	Interval* chunk;
	uint32_t num_of_intervals;
	IntervalChunk* next_chunk;
	
private:
	static const uint32_t DEFAULT_CAPACITY = 256;
};

// Use max heap for treap priorities.
struct Node {
	Interval interval;
	Node *left;
	Node *right;
	int priority;
    MemAccess_t *accessor;

	bool operator==(const Node& a) const {
		return (interval == a.interval);
	}
};

extern "C" bool inParallel(MemAccess_t *reader1, MemAccess_t *reader2);
extern "C" bool leftmost(MemAccess_t *prevReader, MemAccess_t *currReader);
extern "C" bool rightmost(MemAccess_t *prevReader, MemAccess_t *currReader);

void insertAndCheckWriteTreap(IntervalChunk *w_batch, IntervalChunk *r_batch, MemAccess_t *accessor);
void insertAndCheckReadTreap(IntervalChunk *w_batch, IntervalChunk *r_batch, MemAccess_t *accessor);

bool checkAllOverlapIntervals(Node *root, Interval &interval, MemAccess_t *accessor);
bool checkAllOverlapIntervalsDup(Node *root, Interval &interval, MemAccess_t *accessor);

Node *zipInsertNode(Node *node, Node *root);
Node *zipInsertNodeHelper(Node *node, Node *root, Node **extra, int *needInsertion);
Node *nonFullOverlapLeftChild(Interval &interval, MemAccess_t *accessor, Node *root, bool shouldCheckRace);
Node *nonFullOverlapRightChild(Interval &interval, MemAccess_t *accessor, Node *root, bool shouldCheckRace);

bool shouldKeepPrevReader(MemAccess_t *prevReader, MemAccess_t *currReader, bool isLeftmost);
Node *readTreapInsert(Node *node, Node *root, bool isLeftmost);
Node *readTreapInsertHelper(Node *node, Node *root, vector<Node*> *extra, int *needInsertion, bool isLeftmost, bool isTheRoot);

Node *createNode(Interval &interval, MemAccess_t *accessor);
bool isCompleteOverlap(Interval a, Interval b);
bool isOverlap(Interval a, Interval b);
Node *insertZipTreapNodeWithoutIntersection(Node *node, Node *root);
Node *firstOverlapNode(Interval &interval, Node *root, Node **parent);

Node *rightRotate(Node *y);
Node *leftRotate(Node *x);
Node *removeNode(Interval &interval, Node *root);
Node *removeRange(Interval &interval, Node *root);
Node *removeRangeHelper(Interval &interval, Node *root, Node **extra);

uint64_t cal_tree_height(Node* root, uint64_t& total_nodes);
#endif
