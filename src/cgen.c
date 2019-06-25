#include "globals.h"
#include "analyze.h"
#include "cgen.h"
#include "code.h"

static void genCompdBlock(TreeNode *tnode) {
  assert(tnode->nodekind == StmtK && tnode->kind.stmt == CompdK);

  struct ScopeRec *scopeRef = tnode->attr.scope_ref;
  int blockSize = scopeRef->blockSize;

  emitBlockEnter(blockSize);
  // statements here
  emitBlockExit(blockSize);
}

void codeGen(TreeNode *syntaxTree, char *codefile) {
  char header[128] = " Compiled from ";
  strcat(header, codefile);
  emitComment(header);
  fputc('\n', code);

  TreeNode *pNode = syntaxTree;

  /* global scope declarations */
  while (pNode) {
    assert(pNode->nodekind == DeclK);

    if (pNode->kind.decl == VarDeclK) {
      // array size
      int size = pNode->child[0]->attr.val;
      // symbol name
      char const *name = pNode->attr.name;
      if (size == -1) size = 1;
      assert(size > 0);

      /* "memloc" is useless */
      emitGlobalVariable(name, size * 4);
    } else {
      char const *name = pNode->attr.name;

      emitFunctionEnter(name);
      genCompdBlock(pNode->child[2]);
      emitFunctionExit();
    }
    pNode = pNode->sibling;
  }
}
