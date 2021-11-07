#include "utility.h"

extern int maxDepth;
extern int maxNumIntervals;

extern Node *read_treap;
extern Node *write_treap;


int countNumIntervals(Node *root, int sum) {
  if (root == NULL) return sum;

  sum++;
  sum = countNumIntervals(root->left, sum);
  sum = countNumIntervals(root->right, sum);
  return sum;
}

void updateMaxNumIntervalsHelper() {
  int currRead = countNumIntervals(read_treap,0);
  int currWrite = countNumIntervals(write_treap,0);
  int currMax = currRead > currWrite ? currRead : currWrite;
  maxNumIntervals = currMax > maxNumIntervals ? currMax : maxNumIntervals;
}

int countMaxDepth(Node *root, int depth) {
  if (root == NULL) return depth;

  depth++;
  int left = countMaxDepth(root->left, depth);
  int right = countMaxDepth(root->right, depth);
  return left > right ? left : right;
}

void updateMaxNumDepthHelper() {
  int currRead = countMaxDepth(read_treap,0);
  int currWrite = countMaxDepth(write_treap,0);
  int currMax = currRead > currWrite ? currRead : currWrite;
  maxDepth = currMax > maxDepth ? currMax : maxDepth;
}
