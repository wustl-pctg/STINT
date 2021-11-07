#include "treap.h"
#include "ktiming.h"
extern Node* read_treap;
extern Node* write_treap;

extern uint64_t total_ops;
extern uint64_t total_height;
extern uint64_t total_nodes;
#ifdef SAMPLING
uint64_t write_insertion_counter = 0;
uint64_t read_insertion_counter = 0;
uint64_t num_of_sp_queries = 0;
uint64_t num_of_nodes_visited = 0;
#endif
// ==============================================
// Wrapper functions for treap insert
// ==============================================

// Insert w_batch into the big write_treap and then check r_batch with write_treap
void insertAndCheckWriteTreap(IntervalChunk *w_batch, IntervalChunk *r_batch, MemAccess_t *accessor) {
    // insert w_batch into write_treap
	IntervalChunk *w_curr_chunk = w_batch;
	while (w_curr_chunk != NULL) {
		for (int i=0; i<w_curr_chunk->size(); i++) {
			Interval interval = w_curr_chunk->chunk[i];
			Node *node = createNode(interval, accessor);
#ifdef SAMPLING
			write_insertion_counter++;
			if (write_insertion_counter == 1) {
				//uint64_t num_of_nodes = 0;
				//uint64_t height = cal_tree_height(write_treap, num_of_nodes);
				
				total_ops++;
				//total_height += height;
				//total_nodes += num_of_nodes;

				//num_of_sp_queries = 0;
				//num_of_nodes_visited = 0;
				//clockmark_t begin, end;
				//begin = ktiming_getmark();
				write_treap = zipInsertNode(node, write_treap);
				//end = ktiming_getmark();
				//fprintf(stderr, "%zu, %f\n", height, ((double)ktiming_diff_usec(&begin, &end)*1.0e-3));
				//fprintf(stderr, "%zu, %f\n", num_of_sp_queries, ((double)ktiming_diff_usec(&begin, &end)*1.0e-3));
				//fprintf(stderr, "%zu, %zu, %zu, %f\n", num_of_nodes_visited, num_of_sp_queries, num_of_nodes_visited + num_of_sp_queries, ((double)ktiming_diff_usec(&begin, &end)*1.0e-3));
				write_insertion_counter = 0;
			} else {
				write_treap = zipInsertNode(node, write_treap);
			}
#else
			write_treap = zipInsertNode(node, write_treap);
#endif
		}
		//IntervalChunk *w_to_delete = w_curr_chunk;
		w_curr_chunk = w_curr_chunk->next_chunk;
		//delete w_to_delete;
	}

    // check r_batch with write_treap
	IntervalChunk *r_curr_chunk = r_batch;
	while (r_curr_chunk != NULL) {
		for (int i=0; i<r_curr_chunk->size(); i++) {
			Interval interval = r_curr_chunk->chunk[i];
			// Node *node = createNode(interval, accessor);
#ifdef SAMPLING
		total_ops++;	
#endif
   			bool hasRace = checkAllOverlapIntervals(write_treap, interval, accessor);
        	if (hasRace) exit(EXIT_FAILURE);
		}
		//IntervalChunk *r_to_delete = r_curr_chunk;
		r_curr_chunk = r_curr_chunk->next_chunk;
		//delete r_to_delete;
	}
}

// Insert r_batch into the big read_treap and then check w_batch with read_treap 
void insertAndCheckReadTreap(IntervalChunk *w_batch, IntervalChunk *r_batch, MemAccess_t *accessor) {
    // insert r_batch into read_treap
	IntervalChunk *r_curr_chunk = r_batch;
	while (r_curr_chunk != NULL) {
		for (int i=0; i<r_curr_chunk->size(); i++) {
			Interval interval = r_curr_chunk->chunk[i];
			Node *node = createNode(interval, accessor);
#ifdef SAMPLING
			read_insertion_counter++;
			if (read_insertion_counter == 1) {
				//uint64_t num_of_nodes = 0;
				//uint64_t height = cal_tree_height(read_treap, num_of_nodes);
				
				total_ops++;
				//total_height += height;
				//total_nodes += num_of_nodes;

				//num_of_sp_queries = 0;
				//num_of_nodes_visited = 0;
				//clockmark_t begin, end;
				//begin = ktiming_getmark();
				read_treap = readTreapInsert(node, read_treap, true);
				//end = ktiming_getmark();
				//fprintf(stderr, "%zu, %f\n", height, ((double)ktiming_diff_usec(&begin, &end)*1.0e-3));
				//fprintf(stderr, "%zu, %f\n", num_of_sp_queries, ((double)ktiming_diff_usec(&begin, &end)*1.0e-3));
				//fprintf(stderr, "%zu, %zu, %zu, %f\n", num_of_nodes_visited, num_of_sp_queries, num_of_nodes_visited + num_of_sp_queries, ((double)ktiming_diff_usec(&begin, &end)*1.0e-3));
				read_insertion_counter = 0;
			} else {
				read_treap = readTreapInsert(node, read_treap, true);
			}
#else
			read_treap = readTreapInsert(node, read_treap, true);
#endif
		}
		IntervalChunk *r_to_delete = r_curr_chunk;
		r_curr_chunk = r_curr_chunk->next_chunk;
		delete r_to_delete;
	}

    // check w_batch with read_treap
	IntervalChunk *w_curr_chunk = w_batch;
	while (w_curr_chunk != NULL) {
		for (int i=0; i<w_curr_chunk->size(); i++) {
#ifdef SAMPLING
			total_ops++;	
#endif
			Interval interval = w_curr_chunk->chunk[i];
			// Node *node = createNode(interval, accessor);
        	bool hasRace = checkAllOverlapIntervals(read_treap, interval, accessor);
        	if (hasRace) exit(EXIT_FAILURE);
		}
		IntervalChunk *w_to_delete = w_curr_chunk;
		w_curr_chunk = w_curr_chunk->next_chunk;
		delete w_to_delete;
	}
}

// ==============================================
// Sequential - Check an interval against a treap
// ==============================================

bool checkAllOverlapIntervals(Node *root, Interval& interval, MemAccess_t *accessor) {
	if (root == NULL) {
		return false;
	}

	Interval rInterval = root->interval;
	uint64_t rStart = rInterval.start;
	uint64_t rEnd = rInterval.end;
	// Interval iInterval = node->interval;
	uint64_t iStart = interval.start;    
	uint64_t iEnd = interval.end;
#ifdef SAMPLING
	num_of_nodes_visited++; 
#endif
	// If root doesn't overlap with the interval.
	// If the interval is before (to the left) the root, check the left subtree.
	if (iEnd < rStart) {
		return checkAllOverlapIntervals(root->left, interval, accessor);
	}

	// If the interval is after (to the right) the root, check the right subtree.
	if (iStart > rEnd) {
		return checkAllOverlapIntervals(root->right, interval, accessor);
	}

	// Check for races between the root and the interval before proceeding to insert.
	if (inParallel(root->accessor, accessor)) {
		// Report race.
		fprintf(stderr, "Race! New interval [0x%lx, 0x%lx] <-> Treap interval [0x%lx, 0x%lx]\n", iStart, iEnd, rStart, rEnd);
		fprintf(stderr, "When checking against a treap\n");
		// fprintf(stderr, "New interval: [e=%lu, h=%lu]\n", node->accessor->estrand->label,node->accessor->hstrand->label);
		// fprintf(stderr, "Treap interval: [e=%lu, h=%lu]\n", root->accessor->estrand->label,root->accessor->hstrand->label);
		exit(EXIT_FAILURE);
	}

	// If the root is completely in the interval, check left and right subtree
	// to find other overlaps.
	if (isCompleteOverlap(rInterval, interval)) {
		if (checkAllOverlapIntervals(root->left,interval, accessor)) {
			return true;
		}
		return checkAllOverlapIntervals(root->right,interval, accessor);
	}

	// If the interval is completely in the root interval, then the root is the
	// only overlap in the treap. Return it.
	if (isCompleteOverlap(interval, rInterval)) {
		return false;
	}

	// If the root partially overlaps with the interval.
	// If the root is partially before the interval, check the right subtree.
	if (rStart < iStart && rEnd < iEnd) {
		return checkAllOverlapIntervals(root->right, interval, accessor);
	}

	// If the root is partially after the interval, check the left subtree.
	if (iStart < rStart && iEnd < rEnd) {
		return checkAllOverlapIntervals(root->left, interval, accessor);
	}

	return false;
}

bool checkAllOverlapIntervalsDup(Node *root, Interval& interval, MemAccess_t *accessor) {
	if (root == NULL) {
		return false;
	}

	Interval rInterval = root->interval;
	uint64_t rStart = rInterval.start;
	uint64_t rEnd = rInterval.end;
	// Interval iInterval = node->interval;
	uint64_t iStart = interval.start;    
	uint64_t iEnd = interval.end;

#ifdef SAMPLING
	num_of_nodes_visited++; 
#endif

	// If root doesn't overlap with the interval.
	// If the interval is before (to the left) the root, check the left subtree.
	if (iEnd < rStart) {
		return checkAllOverlapIntervalsDup(root->left, interval, accessor);
	}

	// If the interval is after (to the right) the root, check the right subtree.
	if (iStart > rEnd) {
		return checkAllOverlapIntervalsDup(root->right, interval, accessor);
	}

	// Check for races between the root and the interval before proceeding to insert.
	if (inParallel(root->accessor, accessor)) {
		// Report race.
		fprintf(stderr, "Race! New interval [0x%lx, 0x%lx] <-> Treap interval [0x%lx, 0x%lx]\n", iStart, iEnd, rStart, rEnd);
		fprintf(stderr, "When checking against a treap\n");
		// fprintf(stderr, "New interval: [e=%lu, h=%lu]\n", node->accessor->estrand->label,node->accessor->hstrand->label);
		// fprintf(stderr, "Treap interval: [e=%lu, h=%lu]\n", root->accessor->estrand->label,root->accessor->hstrand->label);
		exit(EXIT_FAILURE);
	}

	// If the root is completely in the interval, check left and right subtree
	// to find other overlaps.
	if (isCompleteOverlap(rInterval, interval)) {
		if (checkAllOverlapIntervalsDup(root->left,interval, accessor)) {
			return true;
		}
		return checkAllOverlapIntervalsDup(root->right,interval, accessor);
	}

	// If the interval is completely in the root interval, then the root is the
	// only overlap in the treap. Return it.
	if (isCompleteOverlap(interval, rInterval)) {
		return false;
	}

	// If the root partially overlaps with the interval.
	// If the root is partially before the interval, check the right subtree.
	if (rStart < iStart && rEnd < iEnd) {
		return checkAllOverlapIntervalsDup(root->right, interval, accessor);
	}

	// If the root is partially after the interval, check the left subtree.
	if (iStart < rStart && iEnd < rEnd) {
		return checkAllOverlapIntervalsDup(root->left, interval, accessor);
	}

	return false;
}

// ==============================================
// Sequential - Insert write intervals into write treap and check along the way
// ==============================================

// This function checks for overlaps and does the necessary cutting to make sure
// that the intervals have proper accessors and do not overlap with each other.
// The actual insertion is done by insertZipTreapNodeWithoutIntersection.
Node *zipInsertNode(Node *node, Node *root) {
	Node *parent = NULL;
	Interval interval = node->interval;
	Node *overlap = firstOverlapNode(interval, root, &parent);
	Node *extra = NULL;
	int val = 1;
	int *needInsertion = &val;

	if (overlap != NULL) {
		if (parent == NULL) {
			root = zipInsertNodeHelper(node, root, &extra, needInsertion);
			if (extra) root = insertZipTreapNodeWithoutIntersection(extra, root);
		} else {
			bool left = false;
			if (parent->left != NULL) {
				left = (*parent->left == *overlap);
			}

			overlap = zipInsertNodeHelper(node, overlap, &extra, needInsertion);
			if (left) {
				parent->left = overlap;
			} else {
				parent->right = overlap;
			}
			if (extra) root = insertZipTreapNodeWithoutIntersection(extra, root);
		}
	}

	if (*needInsertion == 1) {
		root = insertZipTreapNodeWithoutIntersection(node, root);
	}
	return root;
}

Node *zipInsertNodeHelper(Node *node, Node *root, Node **extra, int *needInsertion) {
	// This function removes all complete overlap intervals and insert new segments.
	if (root == NULL) {
		*needInsertion = 1;
		return root;
	}

#ifdef SAMPLING
	num_of_nodes_visited++;
#endif
	
	Interval rInterval = root->interval;
	uint64_t rStart = rInterval.start;
	uint64_t rEnd = rInterval.end;
	Interval interval = node->interval;
	uint64_t iStart = interval.start;
	uint64_t iEnd = interval.end;

	// A. If root doesn't overlap with the interval.
	// A1. If the interval is before (to the left) the root.
	if (iEnd < rStart) {
		root->left = zipInsertNodeHelper(node, root->left, extra, needInsertion);
		return root;
	}
	// A2. If the interval is after (to the right) the root.
	if (iStart > rEnd) {
		root->right = zipInsertNodeHelper(node, root->right, extra, needInsertion);
		return root;
	}

	// Check for races between the root and the interval before proceeding.
	if (inParallel(root->accessor, node->accessor)) {
		// Report race.
		fprintf(stderr, "Race! New interval [0x%lx, 0x%lx] <-> Treap interval [0x%lx, 0x%lx]\n", iStart, iEnd, rStart, rEnd);
		fprintf(stderr, "When inserting into the writer treap\n");
		fprintf(stderr, "New interval: [e=%lu, h=%lu]\n", node->accessor->estrand->label,node->accessor->hstrand->label);
		fprintf(stderr, "Treap interval: [e=%lu, h=%lu]\n", root->accessor->estrand->label,root->accessor->hstrand->label);
		exit(EXIT_FAILURE);
	}

	// B. If root completely overlaps the interval.
	// B1. If they are equal.
	if (rStart == iStart && rEnd == iEnd) {
		root->accessor = node->accessor;
		*needInsertion = 0;
		return root;
	}
	// B2. If the root is completely in the interval:
	if (isCompleteOverlap(rInterval, interval)) {
		root->interval = interval;
		root->accessor = node->accessor;
		root->left = nonFullOverlapLeftChild(interval, node->accessor, root->left, true);
		root->right = nonFullOverlapRightChild(interval, node->accessor, root->right, true);
		*needInsertion = 0;
		return root;
	}
	// B3. If the interval is completely in the root interval.
	if (isCompleteOverlap(interval, rInterval)) {
		Interval newInterval1, newInterval2;
		if (rStart == iStart) {
			newInterval2 = { iEnd + 1, rEnd };
			root->interval = newInterval2;
		} else if (rEnd == iEnd) {
			newInterval1 = { rStart, iStart - 1 };
			root->interval = newInterval1;
		} else {
			newInterval1 = { rStart, iStart - 1 };
			newInterval2 = { iEnd + 1, rEnd };
			root->interval = newInterval1;
			Node *node2 = new Node();
			node2->interval = newInterval2;
			node2->accessor = root->accessor;
			node2->priority = rand();
			*extra = node2;
		}
		*needInsertion = 1;
		return root;
	}

	// C. If the root partially overlaps with the interval.
	// C1. If the root is partially before the interval.
	if (rStart < iStart && rEnd < iEnd) {
		Interval newInterval = { rStart, iStart - 1 };
		root->interval = newInterval;
		root->right = zipInsertNodeHelper(node, root->right, extra, needInsertion);
		return root;
	}
	// C2. If the root is partially after the interval.
	if (iStart < rStart && iEnd < rEnd) {
		Interval newInterval = { iEnd + 1, rEnd };
		root->interval = newInterval;
		root->left = zipInsertNodeHelper(node, root->left, extra, needInsertion);
		return root;
	}
	return root;
}

// Given the root is completely in the interval, find its first not full overlap
// left child and prune when necessary. Similar to nonFullOverlapRightChild.
Node *nonFullOverlapLeftChild(Interval &interval, MemAccess_t *accessor, Node *root, bool shouldCheckRace) {
	if (root == NULL) {
		return NULL;
	}

	Node *leftChild = root;
	uint64_t iStart = interval.start;
	uint64_t iEnd = interval.end;

	while (leftChild != NULL) {
#ifdef SAMPLING		
		num_of_nodes_visited++;
#endif
		// Remove its left subtree if it's also completely in the interval.
		if (isCompleteOverlap(leftChild->interval, interval)) {
			if (shouldCheckRace && inParallel(leftChild->accessor, accessor)) {
				// Report race.
				fprintf(stderr, "Race! New interval [0x%lx, 0x%lx] <-> Treap interval [0x%lx, 0x%lx]\n", iStart, iEnd, leftChild->interval.start, leftChild->interval.end);
				fprintf(stderr, "When inserting into a writer treap, nonFullOverlapLeftChild\n");
				//fprintf(stderr, "New interval: [e=%lu, h=%lu]\n", node->accessor->estrand->label,node->accessor->hstrand->label);
				//fprintf(stderr, "Treap interval: [e=%lu, h=%lu]\n", root->accessor->estrand->label,root->accessor->hstrand->label);
				exit(EXIT_FAILURE);
			}

			if (shouldCheckRace) {
				bool hasRace = checkAllOverlapIntervalsDup(leftChild->right, interval, accessor); 
				if (hasRace) exit(EXIT_FAILURE);
			}
			leftChild->right = NULL;
			leftChild = leftChild->left;
		} else {
			break;
		}
	}

	if (leftChild != NULL) {
		if (isOverlap(interval, leftChild->interval)) {
			if (shouldCheckRace && inParallel(leftChild->accessor, accessor)) {
				// Report race.
				fprintf(stderr, "Race! New interval [0x%lx, 0x%lx] <-> Treap interval [0x%lx, 0x%lx]\n", iStart, iEnd, leftChild->interval.start, leftChild->interval.end);
				fprintf(stderr, "When inserting into a writer treap, nonFullOverlapLeftChild\n");
				//fprintf(stderr, "New interval: [e=%lu, h=%lu]\n", node->accessor->estrand->label,node->accessor->hstrand->label);
				//fprintf(stderr, "Treap interval: [e=%lu, h=%lu]\n", root->accessor->estrand->label,root->accessor->hstrand->label);
				exit(EXIT_FAILURE);
			}
			
			Interval newInterval = { leftChild->interval.start, iStart - 1 };
			leftChild->interval = newInterval;
			if (shouldCheckRace) {
				bool hasRace = checkAllOverlapIntervalsDup(leftChild->right, interval, accessor); 
				if (hasRace) exit(EXIT_FAILURE);
			}
			leftChild->right = NULL;
			return leftChild;
		} else {
			if (leftChild->right != NULL) {
				leftChild->right = nonFullOverlapLeftChild(interval, accessor, leftChild->right, shouldCheckRace);
			}
		}
	}
	return leftChild;
}

// Given the root is completely in the interval, find its first not full
// overlap right child and prune when necessary.
Node *nonFullOverlapRightChild(Interval &interval, MemAccess_t *accessor, Node *root, bool shouldCheckRace) {
	if (root == NULL) {
    	return NULL;
	}

	uint64_t iStart = interval.start;
	uint64_t iEnd = interval.end;
	Node *rightChild = root;

	while (rightChild != NULL) {
#ifdef SAMPLING	
		num_of_nodes_visited++;
#endif		
		// Remove its left subtree if the rightChild also completely in the interval.
		if (isCompleteOverlap(rightChild->interval, interval)) {
			if (shouldCheckRace && inParallel(rightChild->accessor, accessor)) {
				// Report race.
				fprintf(stderr, "Race! New interval [0x%lx, 0x%lx] <-> Treap interval [0x%lx, 0x%lx]\n", iStart, iEnd, rightChild->interval.start, rightChild->interval.end);
				fprintf(stderr, "When inserting into a writer treap, nonFullOverlapRightChild\n");
				//fprintf(stderr, "New interval: [e=%lu, h=%lu]\n", node->accessor->estrand->label,node->accessor->hstrand->label);
				//fprintf(stderr, "Treap interval: [e=%lu, h=%lu]\n", root->accessor->estrand->label,root->accessor->hstrand->label);
				exit(EXIT_FAILURE);
			}

			if (shouldCheckRace) {
				bool hasRace = checkAllOverlapIntervalsDup(rightChild->left, interval, accessor); 
				if (hasRace) exit(EXIT_FAILURE);
			}	
			rightChild->left = NULL;
			rightChild = rightChild->right;
		} else {
			break;
		}
	}

	if (rightChild != NULL) {
		if (isOverlap(interval, rightChild->interval)) {
			if (shouldCheckRace && inParallel(rightChild->accessor, accessor)) {
				// Report race.
				fprintf(stderr, "Race! New interval [0x%lx, 0x%lx] <-> Treap interval [0x%lx, 0x%lx]\n", iStart, iEnd, rightChild->interval.start, rightChild->interval.end);
				fprintf(stderr, "When inserting into a writer treap, nonFullOverlapRightChild\n");
				//fprintf(stderr, "New interval: [e=%lu, h=%lu]\n", node->accessor->estrand->label,node->accessor->hstrand->label);
				//fprintf(stderr, "Treap interval: [e=%lu, h=%lu]\n", root->accessor->estrand->label,root->accessor->hstrand->label);
				exit(EXIT_FAILURE);
			}
			
			// RightChild partially overlaps. Its right subtree can't overlap with
			// the interval. So we are done here.
			Interval newInterval = { iEnd + 1, rightChild->interval.end };
			rightChild->interval = newInterval;
			if (shouldCheckRace) {
				bool hasRace = checkAllOverlapIntervalsDup(rightChild->left, interval, accessor); 
				if (hasRace) exit(EXIT_FAILURE);
			}
			rightChild->left = NULL;
			return rightChild;
		} else {
			// No overlap. Check its left child.
			if (rightChild->left != NULL) {
				rightChild->left = nonFullOverlapRightChild(interval, accessor, rightChild->left, shouldCheckRace);
			}
		}
	}
	return rightChild;
}

// ==============================================
// Sequential - Insert read intervals into read treaps (no checking involved)
// ==============================================

bool shouldKeepPrevReader(MemAccess_t *prevReader, MemAccess_t *currReader, bool isLeftmost) {
    bool keepPrevReader;
    if (isLeftmost) {
        keepPrevReader = leftmost(prevReader,currReader);
    } else {
        keepPrevReader = rightmost(prevReader, currReader);
    }
    return keepPrevReader;
}

// Likewise, this function checks for overlaps and does the necessary cutting.
// The actual insertion is done by insertZipTreapNodeWithoutIntersection.
Node *readTreapInsert(Node *node, Node *root, bool isLeftmost) {
    Node *parent = NULL;
    Node *overlap = firstOverlapNode(node->interval, root, &parent);
    vector<Node *> extraVector {};
    vector<Node *> *extraPointer = &extraVector;
    // 0: unset, 1: true, 2: false
    int val = 0;
    int *needInsertion = &val;

    if (overlap != NULL) {
        if (parent == NULL) {
            root = readTreapInsertHelper(node, root, extraPointer, needInsertion,isLeftmost,true);
            if (!extraVector.empty()) {
                for (int i=0; i<extraVector.size(); i++) {
                    root = insertZipTreapNodeWithoutIntersection(extraVector[i], root);
                }
            }
        } else {
            bool left = false;
            if (parent->left != NULL) {
                left = (*parent->left == *overlap);
            }

            overlap = readTreapInsertHelper(node, overlap, extraPointer, needInsertion,isLeftmost,true);
            if (left) {
                parent->left = overlap;
            } else {
                parent->right = overlap;
            }
            if (!extraVector.empty()) {
                for (int i=0; i<extraVector.size(); i++) {
                    root = insertZipTreapNodeWithoutIntersection(extraVector[i], root);
                }
            }
        }

    } else {
        if (*needInsertion == 0) *needInsertion = 1;
    }
    
    if (*needInsertion == 1) {
        root = insertZipTreapNodeWithoutIntersection(node, root);
    }
    return root;
}

Node *readTreapInsertHelper(Node *node, Node *root, vector<Node *> *extra, int *needInsertion, bool isLeftmost, bool isTheNode) {
    Interval interval = node->interval;
    if (root == NULL) {
        if (isTheNode) *needInsertion = 1;
        return root;
    }
#ifdef SAMPLING    
	num_of_nodes_visited++;
#endif
	Interval rInterval = root->interval;
    uint64_t rStart = rInterval.start;
    uint64_t rEnd = rInterval.end;
    uint64_t iStart = interval.start;
    uint64_t iEnd = interval.end;
    MemAccess_t *rootAccessor = root->accessor;
    MemAccess_t *nodeAccessor = node->accessor;

    // If root doesn't overlap with the interval.
    // If the interval is before (to the left) the root.
    if (iEnd < rStart) {
        root->left = readTreapInsertHelper(node, root->left, extra, needInsertion,isLeftmost,isTheNode);
        return root;
    }
    // If the interval is after (to the right) the root.
    if (iStart > rEnd) {
        root->right = readTreapInsertHelper(node, root->right, extra, needInsertion,isLeftmost,isTheNode);
        return root;
    }

    // If they are equal.
    if (rStart == iStart && rEnd == iEnd) {
        // if (*needInsertion == 0) *needInsertion = 2;
        // If they are in parallel, do nothing. Otherwise, update root's reader.
        if (!shouldKeepPrevReader(rootAccessor, nodeAccessor, isLeftmost)) {
            root->accessor = nodeAccessor;
        }
        if (isTheNode) {
	   *needInsertion = 2; 
        } else {
            node->interval = {1,0};
	}
        return root;
    }

    // If the root is completely in the interval:
    if (isCompleteOverlap(rInterval, interval)) {
        // If in parallel, keep the root and
        // break the interval and recurse on the left and right subtree.
        if (shouldKeepPrevReader(rootAccessor, nodeAccessor, isLeftmost)) {
            if (rStart == iStart) {
                node->interval = { rEnd + 1, iEnd };
                root->right = readTreapInsertHelper(node, root->right, extra, needInsertion,isLeftmost,isTheNode);
            } else if (rEnd == iEnd) {
                node->interval = { iStart, rStart - 1 };
                root->left = readTreapInsertHelper(node, root->left, extra, needInsertion,isLeftmost,isTheNode);
            } else {
                // Two intervals broken from the inserted node.
                node->interval = { iStart, rStart - 1 };
                node->priority = node->priority;
                node->accessor = nodeAccessor;

                Node *node2 = new Node();
                node2->interval = { rEnd + 1, iEnd };
                node2->priority = rand();
                node2->accessor = nodeAccessor;
                
                root->left = readTreapInsertHelper(node, root->left, extra, needInsertion,isLeftmost,isTheNode);
                root->right = readTreapInsertHelper(node2, root->right, extra, needInsertion,isLeftmost,false);
                (*extra).push_back(node2);
            }
        } else {
            Interval remove = {rStart, rEnd};
            root = removeNode(remove, root);
            root = readTreapInsertHelper(node, root, extra, needInsertion,isLeftmost,isTheNode);
        }
        if (isTheNode && *needInsertion == 0) *needInsertion = 1;
        return root;
    }

    // If the interval is completely in the root interval.
    if (isCompleteOverlap(interval, rInterval)) {
        // If in parallel, do nothing. Because we want to keep the old reader.
        if (!shouldKeepPrevReader(rootAccessor, nodeAccessor, isLeftmost)) {
            if (rStart == iStart) {
                root->interval = { iEnd + 1, rEnd };
            } else if (rEnd == iEnd) {
                root->interval = { rStart, iStart - 1 };
            } else {
                root->interval = { rStart, iStart - 1 };
                Node *node2 = new Node();
                node2->interval = { iEnd + 1, rEnd };
                node2->priority = rand();
                node2->accessor = rootAccessor;
                (*extra).push_back(node2);
            }
        } else {
            node->interval = {1,0};
        }
        if (isTheNode) *needInsertion = 1;
        return root;
    }

    // If the root partially overlaps with the interval.
    // If the root is partially before the interval.
    if (rStart < iStart && rEnd < iEnd) {
        Interval newInterval = { rStart, iStart - 1 };
        // If in parallel, update the interval of the inserted node.
         if (shouldKeepPrevReader(rootAccessor, nodeAccessor, isLeftmost)) {
            node->interval = {rEnd + 1, iEnd};
        } else {
            root->interval = newInterval;
        }

        root->right = readTreapInsertHelper(node, root->right, extra, needInsertion,isLeftmost,isTheNode);
        if (isTheNode && *needInsertion == 0) *needInsertion = 1;
        return root;
    }

    // If the root is partially after the interval.
    if (iStart < rStart && iEnd < rEnd) {
        Interval newInterval = { iEnd + 1, rEnd };

         if (shouldKeepPrevReader(rootAccessor, nodeAccessor, isLeftmost)) {
            node->interval = {iStart, rStart - 1};
        } else {
            root->interval = newInterval;
        }

        root->left = readTreapInsertHelper(node, root->left, extra, needInsertion,isLeftmost,isTheNode);
        if (isTheNode && *needInsertion == 0) *needInsertion = 1;
        return root;
    }

    return root;
}

// ==============================================
// Sequential - Common helper functions
// ==============================================

Node *createNode(Interval &interval, MemAccess_t *accessor) {
	Node *node = new Node();
	node->priority = rand();
	node->interval = interval;
	node->accessor = accessor;
    node->left = NULL;
    node->right = NULL;
	return node;
}

bool isCompleteOverlap(Interval a, Interval b) {
	return isOverlap(a, b) && (a.start >= b.start && a.end <= b.end);
}

bool isOverlap(Interval a, Interval b) {
	return (a.start <= b.end && a.end >= b.start);
}

// This is the actual insertion. It is written according to the zip tree insert algorithm.
Node *insertZipTreapNodeWithoutIntersection(Node *node, Node *root) {
	if (node->interval.start > node->interval.end) {
        return root;
    }

	// Insert node using zip tree insertion.
	if (root == NULL) {
		return node;
	}

	Interval interval = node->interval;
	uint64_t intervalStart = interval.start;
	int nodePriority = node->priority;
	Node *y = root;
	// Need to keep track of y's parent in order to assign x as its child.
	Node *yParent = NULL;
	bool left = true;
	while (y != NULL) {
#ifdef SAMPLING		
		num_of_nodes_visited++;
#endif		
		if (intervalStart > y->interval.start) {
			if (y->right == NULL || y->priority < nodePriority) {
				break;
			} else {
				yParent = y;
				left = false;
				y = y->right;
			}
		} else {
			if (y->left == NULL || y->priority < nodePriority) {
				break;
			} else {
				yParent = y;
				left = true;
				y = y->left;
			}
		}
	}

	//If there's no such y.
	if (y->priority >= node->priority) {
		if (y->interval.start > node->interval.start) {
			y->left = node;
		} else {
			y->right = node;
		}
		return root;
	}

	Node *dummySm = new Node();
	Node *dummyLg = new Node();
	Node *curSm = dummySm;
	Node *curLg = dummyLg;

	// unzip
	while (y != NULL) {
#ifdef SAMPLING		
		num_of_nodes_visited++;
#endif		
		if (y->interval.start < intervalStart) {
			curSm->right = y;
			curSm = y;
			y = y->right;
			curSm->right = NULL;
		} else {
			curLg->left = y;
			curLg = y;
			y = y->left;
			curLg->left = NULL;
		}
	}
	node->left = dummySm->right;
	node->right = dummyLg->left;

	if (yParent == NULL) {
		return node;
	}

	// Make the node a child of y's parent.
	if (left) {
		yParent->left = node;
	} else {
		yParent->right = node;
	}
	return root;
}

// Find the first overlap node in the treap.
Node *firstOverlapNode(Interval &interval, Node *root, Node **parent) {
	if (root == NULL) {
		return NULL;
	}
#ifdef SAMPLING	
	num_of_nodes_visited++;
#endif
	Interval curInterval = root->interval;
	uint64_t curStart = curInterval.start;
	uint64_t curEnd = curInterval.end;
	// Overlap with root.
	if (curStart <= interval.end && curEnd >= interval.start) {
		return root;
	}
	// If interval is before root, do recurssion on the left subtree.
	if (interval.end < curStart) {
		*parent = root;
		return firstOverlapNode(interval, root->left, parent);
	}
	// If interval is after root, do recurssion on the right subtree.
	if (interval.start > curEnd) {
		*parent = root;
		return firstOverlapNode(interval, root->right, parent);
	}
	return NULL;
}

// ==============================================
// Sequential - Remove functions
// ==============================================

// Left and right rotation for insertions.
// https://www.geeksforgeeks.org/treap-set-2-implementation-of-search-insert-and-delete/
Node *rightRotate(Node *y) {
	Node *x = y->left;
	Node *T2 = x->right;

	x->right = y;
	y->left = T2;

	return x;
}

Node *leftRotate(Node *x) {
	Node *y = x->right;
	Node *T2 = y->left;

	y->left = x;
	x->right = T2;

	return y;
}

// Remove one node from the treap.
// The input interval is one existing node in the treap.
// https://www.geeksforgeeks.org/treap-set-2-implementation-of-search-insert-and-delete/
Node *removeNode(Interval &interval, Node *root) {
	if (root == NULL) {
		return root;
	}
	// If the interval is the same as the root interval.
	if (interval == root->interval) {
		if (root->left == NULL) {
			// The new root is the right child.
			Node *new_root = root->right;
			delete(root);
			root = new_root;
		} else if (root->right == NULL) {
			Node *new_root = root->left;
			delete(root);
			root = new_root;
		} else if (root->left->priority < root->right->priority) {
			// Check priority.
			root = leftRotate(root);
			root->left = removeNode(interval, root->left);
		} else {
			root = rightRotate(root);
			root->right = removeNode(interval, root->right);
		}
		return root;
	}
	// If the interval is before the root interval.
	else if (interval.start < root->interval.start) {
		root->left = removeNode(interval, root->left);
	} else {
		// If the interval is after the root interval.
		root->right = removeNode(interval, root->right);
	}
	return root;
}

// Remove all intervals covered in the input range in the treap.
// It is used in clear_shadow_memory.
Node *removeRange(Interval &interval, Node *root) {
    Node *extra = NULL;
    root = removeRangeHelper(interval, root, &extra);
    if (extra) {
        root = insertZipTreapNodeWithoutIntersection(extra, root);
    }
    return root;
}

Node *removeRangeHelper(Interval &interval, Node *root, Node **extra) {
    if (root == NULL) {
        return root;
    }
    Interval rInterval = root->interval;
    uint64_t rStart = rInterval.start;
    uint64_t rEnd = rInterval.end;
    uint64_t iStart = interval.start;
    uint64_t iEnd = interval.end;
    // If they are the same. Remove the root.
    if (rInterval == interval) {
        root = removeNode(interval, root);
        return root;
    }
    // If the interval is before (to the left) the root.
    if (iEnd < rStart) {
        root->left = removeRangeHelper(interval, root->left,extra);
        return root;
    }
    // If the interval is after (to the right) the root.
    if (iStart > rEnd) {
        root->right = removeRangeHelper(interval, root->right,extra);
        return root;
    }

    // If the root is completely in the interval:
    if (isCompleteOverlap(rInterval, interval)) {
        root->right = nonFullOverlapRightChild(interval, NULL, root->right, false);
        root->left = nonFullOverlapLeftChild(interval, NULL, root->left, false);
        // Remove the root.
        root = removeNode(rInterval, root);
        return root;
    }

    // If the interval is completely in the root interval.
    if (isCompleteOverlap(interval, rInterval)){
        Interval newInterval1, newInterval2;
        if (rStart == iStart) {
            newInterval2 = { iEnd + 1, rEnd};
            root->interval = newInterval2;
        } else if (rEnd == iEnd) {
            newInterval1 = { rStart, iStart - 1};
            root->interval = newInterval1;
        } else {
            newInterval1 = { rStart, iStart - 1 };
            newInterval2 = { iEnd + 1, rEnd };
            root->interval = newInterval1;
            Node *node2 = new Node();
            // Insert this new node.
            node2->interval = newInterval2;
            node2->priority = rand();
            node2->accessor = root->accessor;
            *extra = node2;
        }
        return root;
    }

    // If the root partially overlaps with the interval.
    // If the root is partially before the interval.
    if (rStart < iStart && rEnd < iEnd) {
        root->interval = { rStart, iStart - 1 };
	Interval remove = {rEnd+1,iEnd};
        root->right = removeRangeHelper(remove, root->right,extra);
        return root;
    }

    // If the root is partially after the interval.
    if (iStart < rStart && iEnd < rEnd) {
        root->interval = { iEnd + 1, rEnd};
	Interval remove = {iStart,rStart-1};
        root->left = removeRangeHelper(remove, root->left,extra);
        return root;
    }

    return root;
}

uint64_t cal_tree_height(Node* root, uint64_t& num_of_nodes) {
	if (root == NULL) { 
		num_of_nodes = 0;
		return 0;
	}
	uint64_t left_nodes = 0;
	uint64_t right_nodes = 0;
	uint64_t left_height = cal_tree_height(root->left, left_nodes);
	uint64_t right_height = cal_tree_height(root->right, right_nodes);
	num_of_nodes = left_nodes + right_nodes + 1;
	return (left_height > right_height) ? left_height + 1 : right_height + 1;
}
