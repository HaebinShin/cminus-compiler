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
static void genArgExpression(TreeNode *exprNode);
static void genExpression(TreeNode *tnode);
static void genAssignExpr(TreeNode *tnode);
static void genBinaryExpr(TreeNode *tnode);
static void genVarExprLHS(TreeNode *tnode);
static void genArrayExprLHS(TreeNode *tnode);
static void genVarExpr(TreeNode *tnode);
static void genArrayExpr(TreeNode *tnode);
static void genCallExpr(TreeNode *tnode);

static int normalizeLocalOffset(int offset) {
  assert((offset + 400) % 4 == 0);
  return offset / 4;
}

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
    emitComment("**** statement of a expression ****");
    genExpression(stmtNode);
    emitComment("**** ************************* ****");
    emitComment("\n");
  }
}

static void genCompdStmt(TreeNode *tnode) {
  assert(tnode->nodekind == StmtK && tnode->kind.stmt == CompdK);

  struct ScopeRec *scopeRef = tnode->scope_ref;
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
  }
  emitUncondBranching(currentRetLabel);
}

static void genArgExpression(TreeNode *exprNode) {
  assert(exprNode->nodekind == ExprK);

  ExprKind kind = exprNode->kind.expr;

  if(kind == VarK) {
    TreeNode *declNode = getTreeNode(exprNode->sym_ref);
    if(declNode->child[0]->attr.val >= 0) {
      genVarExprLHS(exprNode);
    }
  }
  else {
    genExpression(exprNode);
  }
}

/* RHS expression gen */
static void genExpression(TreeNode *exprNode) {
  // always end on $v0

  assert(exprNode->nodekind == ExprK);

  ExprKind kind = exprNode->kind.expr;

  if(kind == VarK) {
    genVarExpr(exprNode);
  }
  else if(kind == OpExprK) {
    if(exprNode->attr.op == ASSIGN) {
      genAssignExpr(exprNode);
    }
    else if(exprNode->attr.op == LBRACKET) {
      genArrayExpr(exprNode);
    }
    else {
      genBinaryExpr(exprNode);
    }
  }
  else if(kind == CallK) {
    genCallExpr(exprNode);
  }
  else {
    assert(kind == ConstK);
    emitConstExpr(exprNode->attr.val);
  }

}

static void genAssignExpr(TreeNode *tnode) {
  TreeNode *lhs, *rhs;
  lhs = tnode->child[0];
  rhs = tnode->child[1];

  if(lhs->kind.expr == VarK) {
    genVarExprLHS(lhs);
  }
  else {
    assert(lhs->kind.expr == OpExprK && lhs->attr.op == LBRACKET);
    genArrayExprLHS(lhs);
  }
  emitPushValue();

  genExpression(rhs);
  emitPopLHS();

  emitBinaryOp(ASSIGN);

  // $v0 holds rhs value
}

static void genBinaryExpr(TreeNode *tnode) {
  TreeNode *lhs, *rhs;
  lhs = tnode->child[0];
  rhs = tnode->child[1];

  genExpression(lhs);
  emitPushValue();

  genExpression(rhs);
  emitPopLHS();

  emitBinaryOp(tnode->attr.op);
}

static void genVarExprLHS(TreeNode *tnode) {
  struct ScopeRec *scope_ref = tnode->scope_ref;
  if(scope_ref->scopeId == 0) {
    emitGlobalRef(tnode->attr.name, GET_ADDRESS);
  }
  else {
    int relativeOffset = normalizeLocalOffset(tnode->loc);
    emitLocalRef(relativeOffset, GET_ADDRESS);
  }
}

static void genArrayExprLHS(TreeNode *tnode) {
  int isPointer = ((struct ScopeRec *)tnode->child[0]->scope_ref)->scopeId != 0;
  if(isPointer) genVarExpr(tnode->child[0]);
  else genVarExprLHS(tnode->child[0]);
  emitPushValue();

  genExpression(tnode->child[1]);
  emitPopLHS();

  emitArrayOp(GET_ADDRESS);
}

static void genVarExpr(TreeNode *tnode) {
  struct ScopeRec *scope_ref = tnode->scope_ref;
  if(scope_ref->scopeId == 0) {
    emitGlobalRef(tnode->attr.name, GET_VALUE);
  }
  else {
    int relativeOffset = normalizeLocalOffset(tnode->loc);
    emitLocalRef(relativeOffset, GET_VALUE);
  }
}

static void genArrayExpr(TreeNode *tnode) {
  int isPointer = ((struct ScopeRec *)tnode->child[0]->scope_ref)->scopeId != 0;
  if(isPointer) genVarExpr(tnode->child[0]);
  else genVarExprLHS(tnode->child[0]);
  emitPushValue();

  genExpression(tnode->child[1]);
  emitPopLHS();

  emitArrayOp(GET_VALUE);
}

static void genCallExpr(TreeNode *tnode) {
  char const *name = tnode->attr.name;
  TreeNode *argNode = tnode->child[0];

  if(strcmp(name, "input") == 0) {
    emitInputSyscall();
    return;
  }
  else if(strcmp(name, "output") == 0) {
    genExpression(argNode);
    emitOutputSyscall();
    return;
  }

  int cnt = 0;
  while(argNode) {
    genArgExpression(argNode);
    emitPushValue();

    argNode = argNode->sibling;
    ++cnt;
  }

  emitCallFunction(name);
  emitPopMultiple(cnt);
}

void codeGen(TreeNode *syntaxTree, char *codefile) {
  char header[128] = " Compiled from ";
  strcat(header, codefile);
  emitComment(header);
  fputc('\n', code);

  emitInitial();

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
