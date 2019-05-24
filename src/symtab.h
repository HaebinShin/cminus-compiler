#ifndef _SYMTAB_H_
#define _SYMTAB_H_

typedef struct BucketListRec *BucketList;

BucketList *constructSymtab(void);
void destroySymtab(BucketList *symtab);

struct SymbolRec *newSymbol(TreeNode *tnode, int loc);
void deleteSymbol(struct SymbolRec *sym);

void addLineno(struct SymbolRec *symbolRec, int lineno);

void st_insert(BucketList *symtab, struct SymbolRec *symbolRec);
int st_lookup(BucketList *symtab, char *name);

#endif