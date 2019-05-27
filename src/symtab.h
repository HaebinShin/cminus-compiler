#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#define INVALID_LOC_NUMBER (0x7fFFffFF)

typedef struct BucketListRec *BucketList;

BucketList *constructSymtab(void);
void destroySymtab(BucketList *symtab);

struct SymbolRec *newSymbol(TreeNode *tnode, int loc);
void deleteSymbol(struct SymbolRec *sym);

void addLineno(struct SymbolRec *symbolRec, int lineno);
int getDeclLineno(struct SymbolRec *symbolRec);
int getMemLoc(struct SymbolRec *symbolRec);
TreeNode *getTreeNode(struct SymbolRec *symbolRec);

void st_insert(BucketList *symtab, struct SymbolRec *symbolRec);
struct SymbolRec *st_lookup(BucketList *symtab, char const *name);

void printSymbolTable(FILE *out, BucketList *symtab, int scopeId);

#endif
