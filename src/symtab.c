#include "globals.h"
#include "symtab.h"
#include "util.h"

/* the number of BucketList in a hash table */
#define SIZE 211

/* the power of two used as multiplier in hash function */
#define SHIFT 7

enum _SymDecl {
  VARIABLE, PARAMETER, FUNCTION
};

typedef struct LineListRec {
  int lineno;
  struct LineListRec *next;
} *LineList;

struct SymbolRec {
  TreeNode *tnode;
  int loc;
  LineList lineList;
};

struct BucketListRec {
  struct SymbolRec *sym;
  struct BucketListRec *next;
};

static unsigned hash_digest(unsigned char *key) {
  unsigned temp = 0;
  while(*key) {
    temp = ((temp << SHIFT) + *key) % SIZE;
    ++key;
  }
  return temp;
}

BucketList *constructSymtab() {
  BucketList *bucketList = calloc(SIZE, sizeof(BucketList));
  // all fields in bucketList[i] are defaulted to null
  return bucketList;
}

void destroySymtab(BucketList *symtab) {
  int i;
  for(i=0; i<SIZE; ++i) {
    BucketList p = symtab[i], pn;
    while(p) {
      pn = p->next;
      deleteSymbol(p->sym);
      free(p);
      p = pn;
    }
  }
  free(symtab);
}

struct SymbolRec *newSymbol(TreeNode *tnode, int loc) {
  enum _SymDecl decl;
  if(tnode->nodekind == ParamK) {
    decl = PARAMETER;
  }
  else if(tnode->nodekind == DeclK) {
    decl = tnode->kind.decl == VarDeclK ? VARIABLE : FUNCTION;
  }
  else return NULL;

  struct SymbolRec *sym = malloc(sizeof(struct SymbolRec));
  sym->tnode = tnode;
  sym->loc = loc;
  sym->lineList = calloc(1, sizeof(struct LineListRec));
  sym->lineList->lineno = tnode->lineno;
  return sym;
}

void deleteSymbol(struct SymbolRec *sym) {
  LineList plist = sym->lineList, pnlist;
  while(plist) {
    pnlist = plist->next;
    free(plist);
    plist = pnlist;
  }
  free(sym);
}

void addLineno(struct SymbolRec *symbolRec, int lineno) {
  LineList plist = symbolRec->lineList;
  while(plist->next != NULL) plist = plist->next;
  plist->next = calloc(1, sizeof(struct LineListRec));
  plist->lineno = lineno;
}

int getDeclLineno(struct SymbolRec *symbolRec) {
  return symbolRec->tnode->lineno;
}

int getMemLoc(struct SymbolRec *symbolRec) {
  return symbolRec->loc;
}

void st_insert(BucketList *symtab, struct SymbolRec *symbolRec) {
  BucketList p = malloc(sizeof(struct BucketListRec));
  int h = hash_digest(symbolRec->tnode->attr.name);

  p->sym = symbolRec;
  p->next = symtab[h];
  symtab[h] = p;
}

// returns memory loc
struct SymbolRec *st_lookup(BucketList *symtab, char *name) {
  int h = hash_digest(name);
  BucketList p = symtab[h];
  while(p != NULL) {
    TreeNode* tnode = p->sym->tnode;
    if(strcmp(name, tnode->attr.name) == 0) return p->sym;
    p = p->next;
  }
  return NULL;
}