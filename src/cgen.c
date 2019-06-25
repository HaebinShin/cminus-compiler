#include "globals.h"
#include "analyze.h"
#include "cgen.h"
#include "code.h"

enum label {
  IF_LABEL,
  ITER_LABEL,
  RET_LABEL
};

static int labelCounterIF = 0;
static int labelCounterITER = 0;
static int labelCounterRET = 0;

static char currentRetLabel[64];

static char const *nextLabel(enum label label) {
  static char _buf[64];
  switch(label) {
  case IF_LABEL:
    sprintf(_buf, "IF_%d", labelCounterIF);
    ++labelCounterIF;
    break;
  case ITER_LABEL:
    sprintf(_buf, "ITER_%d", labelCounterITER);
    ++labelCounterITER;
    break;
  case RET_LABEL:
    sprintf(_buf, "RET_%d", labelCounterRET);
    ++labelCounterRET;
    break;
  }

  return _buf;
}

static void genStatement(TreeNode *tnode);
static void genCompdStmt(TreeNode *tnode);
static void genSelectStmt(TreeNode *tnode);
static void genIterStmt(TreeNode *tnode);
static void genRetStmt(TreeNode *tnode);
static void genExpression(TreeNode *tnode);

static void genStatement(TreeNode *stmtNode) {
  assert(stmtNode->nodekind == StmtK || stmtNode->nodekind == ExprK);

  if(stmtNode->nodekind == StmtK) {
    if(stmtNode->kind.stmt == CompdK) {
      genCompdStmt(stmtNode);
    }
    else if(stmtNode->kind.stmt == SelectK) {
      genSelectStmt(stmtNode);
    }
    else if(stmtNode->kind.stmt == IterK) {
      genIterStmt(stmtNode);
    }
    else if(stmtNode->kind.stmt == RetK) {
      genRetStmt(stmtNode);
    }
    else {
      assert(!"unreachable");
    }
  }
  else {
    assert(stmtNode->nodekind == ExprK);
    genExpression(stmtNode);
  }
}

static void genCompdStmt(TreeNode *tnode) {
  assert(tnode->nodekind == StmtK && tnode->kind.stmt == CompdK);

  struct ScopeRec *scopeRef = tnode->attr.scope_ref;
  int blockSize = scopeRef->blockSize;

  emitBlockEnter(blockSize);

  TreeNode *stmtNode = tnode->child[1];
  while(stmtNode) {
    genStatement(stmtNode);
    stmtNode = stmtNode->sibling;
  }

  emitBlockExit(blockSize);
}

static void genSelectStmt(TreeNode *tnode) {
  if(tnode->nChildren == 2) {
    char label0[64];
    strcpy(label0, nextLabel(IF_LABEL));

    genExpression(tnode->child[0]);
    emitBranching(label0, 0);
    genStatement(tnode->child[1]);
    emitLabel(label0);
  }
  else {
    char label0[64], label1[64];
    strcpy(label0, nextLabel(IF_LABEL));
    strcpy(label1, nextLabel(IF_LABEL));

    genExpression(tnode->child[0]);
    emitBranching(label0, 0);
    genStatement(tnode->child[1]);
    emitUncondBranching(label1);
    emitLabel(label0);
    genStatement(tnode->child[2]);
    emitLabel(label1);
  }
}

static void genIterStmt(TreeNode *tnode) {
  char label0[64], label1[64];
  strcpy(label0, nextLabel(ITER_LABEL));
  strcpy(label1, nextLabel(ITER_LABEL));

  emitLabel(label0);
  genExpression(tnode->child[0]);
  emitBranching(label1, 0);
  genStatement(tnode->child[1]);
  emitUncondBranching(label0);
  emitLabel(label1);
}

static void genRetStmt(TreeNode *tnode) {
  if(tnode->nChildren == 1) {
    genExpression(tnode->child[0]);
    emitUncondBranching(currentRetLabel);
  }
}

static void genExpression(TreeNode *tnode) {
  // always end on $v0
  // TODO
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

      strcpy(currentRetLabel, nextLabel(RET_LABEL));

      emitFunctionEnter(name);
      genCompdStmt(pNode->child[2]);
      emitRaw("\n");
      emitLabel(currentRetLabel);
      emitFunctionExit();
    }
    pNode = pNode->sibling;
  }
}
