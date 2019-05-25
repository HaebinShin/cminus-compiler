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
  char *name;
  int loc;
  int szArray;
  enum _SymDecl decl;
  TypeKind type;
  LineList lineList;
};

typedef struct BucketListRec {
  struct SymbolRec *sym;
  struct BucketListRec *next;
} *BucketList;

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

      // delete a linked list of line numbers
      LineList plist = p->sym->lineList, pnlist;
      while(plist) {
        pnlist = plist->next;
        free(plist);
        plist = pnlist;
      }

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
  sym->name = copyString(tnode->attr.name);
  sym->loc = loc;
  sym->szArray = tnode->child[0]->attr.val;
  sym->decl = decl;
  sym->type = tnode->child[0]->type;
  sym->lineList = calloc(1, sizeof(struct LineListRec));
  sym->lineList->lineno = tnode->lineno;
  return  sym;
}

void deleteSymbol(struct SymbolRec *sym) {
  free(sym->name);
  LineList plist = sym->lineList, pnlist;
  while(plist) {
    pnlist = plist->next;
    free(plist);
    plist = pnlist;
  }
  free(sym);
}

void addLineno(struct SymbolRec *symbolRec, int lineno) {
  // TODO
  LineList plist = symbolRec->lineList;
  while(plist->next != NULL) plist = plist->next;
  plist->next = calloc(1, sizeof(struct LineListRec));
  plist->lineno = lineno;
}

void st_insert(BucketList *symtab, struct SymbolRec *symbolRec) {
  BucketList p = malloc(sizeof(struct BucketListRec));
  int h = hash_digest(symbolRec->name);

  p->sym = symbolRec;
  p->next = symtab[h];
  symtab[h] = p;
}

// returns memory loc
int st_lookup(BucketList *symtab, char *name) {
  int h = hash_digest(name);
  BucketList p = symtab[h];
  while(p != NULL) {
    if(strcmp(name, p->sym->name) == 0) return p->sym->loc;
    p = p->next;
  }
  return -1;
}
