#ifndef _ANALYZE_H_
#define _ANALYZE_H_

#include "symtab.h"

struct ScopeRec {
  int scopeId;
  int scopeDepth;
  int stackCounter;
  int blockSize;
  BucketList *symtab;
};

void buildSymtab(TreeNode *);

void typeCheck(TreeNode *);

#endif
