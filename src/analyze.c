#include "globals.h"
#include "analyze.h"
#include "symtab.h"

#define MAX_SCOPE_DEPTH 20

typedef void (*TraverseFunc)(TreeNode *);

static void NOOP(TreeNode *_) {
  /* DO NOTHING */
}

static void traverseSiblings(
    TreeNode *tnode, TraverseFunc funcPre, TraverseFunc funcPost);
static void traverseSingle(
    TreeNode *tnode, TraverseFunc funcPre, TraverseFunc funcPost);

static void traverseSiblings(
    TreeNode *tnode, TraverseFunc funcPre, TraverseFunc funcPost) {
  if (tnode != NULL) {
    traverseSingle(tnode, funcPre, funcPost);
    tnode = tnode->sibling;
  }
}

static void traverseSingle(
    TreeNode *tnode, TraverseFunc funcPre, TraverseFunc funcPost) {
  funcPre(tnode);
  int i;
  for (i = 0; i < tnode->nChildren; ++i) {
    traverseSiblings(tnode->child[i], funcPre, funcPost);
  }
  funcPost(tnode);
}

void enterScope(void) {
}

void exitScope(void) {
}

static void buildSymtab_pre(TreeNode *tnode) {
  switch (tnode->nodekind) {
  case DeclK:
    switch (tnode->kind.decl) {
      case VarDeclK:
        if (st_lookup()>0) {
          /* already in */
          break;
        }
        st_insert();
        break;
      case FuncDeclK:
        if (st_lookup()>0) {
          /* already in */
          break;
        }
        enterScope();
        
        st_insert();
        break;
    }
    break;
  case ParamK:
    if (st_lookup()>0) {
      /* alread in */
      break;
    }
    st_insert(); 
    break;
  case StmtK:
    if(tnode->kind.stmt != CompdK) break;
    enterScope();
    break;
  default:
    break;
  }
}

static void buildSymtab_post(TreeNode *tnode) {
  switch (tnode->nodekind) {
  case DeclK:
    break;
  case ParamK:
    break;
  case StmtK:
    if(tnode->kind.stmt != CompdK) break;
    exitScope();
    break;
  default:
    break;
  }
}

void buildSymtab(TreeNode *syntaxTree) {
  traverseSiblings(syntaxTree, buildSymtab_pre, buildSymtab_post);
  if (TraceAnalyze) {
    fprintf(listing, "\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode *t, char *message) {
  fprintf(listing, "Type error at line %d: %s\n", t->lineno, message);
  ERROR = TRUE;
}

void typeCheck(TreeNode *tnode) {
  // TODO
  printf("typeCheck! called");
}
