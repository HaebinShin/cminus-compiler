#include "globals.h"
#include "util.h"
#include "symtab.h"
#include "analyze.h"

#define MAX_SCOPES_COUNT 50
#define MAX_SCOPE_DEPTH 20
#define ERROR_MSG(a, b, c) (analyzeErrorMsg(a, b, c))

typedef void (*TraverseFunc)(TreeNode *);

/* Whole list of scopes created so far */
/* scopeWholeLilst[0] EQUALS the global scope */
static struct ScopeRec scopeWholeList[MAX_SCOPES_COUNT];
static int len_scopeWholeList;

/* for referencing parent scopes */
static struct ScopeRec *scopeStack[MAX_SCOPE_DEPTH];
static int h_scopeStack;

/* indicates that you are analyzing a function */
static int functionFlag;

/* indicates that you are in the middle of,
   or done with analyzing the main function */
static int mainFlag;

/* 1: global scope */
/* 0: function scope */

static int scopeIdCounter;
static int functionLocCounter;

static char* funcName;

static TreeNode *externDecl = NULL;

static void NOOP(TreeNode *_) {
  /* DO NOTHING */
}

static void traverseSiblings(
    TreeNode *tnode, TraverseFunc funcPre, TraverseFunc funcPost);
static void traverseSingle(
    TreeNode *tnode, TraverseFunc funcPre, TraverseFunc funcPost);

static void traverseSiblings(
    TreeNode *tnode, TraverseFunc funcPre, TraverseFunc funcPost) {
  while (tnode != NULL) {
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

static struct SymbolRec *lookupSymbol(char const *name) {
  int i = h_scopeStack - 1;
  while(i >= 0) {
    struct ScopeRec *scope = scopeStack[i];
    struct SymbolRec *sym = st_lookup(scope->symtab, name);
    if (sym != NULL) {
      return sym;
    }
    --i;
  } 
  return NULL;
}

static void enterExistingScope(struct ScopeRec *scope) {
  scopeStack[h_scopeStack] = scope;
  ++h_scopeStack;
}

static void enterScope(void) {
  struct ScopeRec *new_scope = &scopeWholeList[len_scopeWholeList++];
  new_scope->scopeId = scopeIdCounter++;
  new_scope->scopeDepth = h_scopeStack;
  new_scope->symtab = constructSymtab();
  
  enterExistingScope(new_scope);
}

static struct ScopeRec *exitScope() {
  struct ScopeRec *exitingScope = scopeStack[--h_scopeStack];

  return exitingScope;
}

static struct ScopeRec *getPrevScope() {
  return scopeStack[h_scopeStack - 2];
}

static struct ScopeRec *getCurrentScope() {
  return scopeStack[h_scopeStack - 1];
}

static void buildSymtab_pre(TreeNode *tnode) {
  /* **** INSERT NEW SYMBOLS **** */
  switch(tnode->nodekind) {
  case ParamK:
    if(tnode->nChildren == 0) {
      // indicates empty parameter list
      // skip actions
      break;
    }
    // if non-empty parameter,
    // flows through below
  case DeclK: {
    char _buf[128];

    char const *name = tnode->attr.name;
    struct ScopeRec *scope = getCurrentScope();
    struct SymbolRec *sym;

    sym = st_lookup(getCurrentScope()->symtab, name);
    if(sym != NULL) {
      // symbol already defined
      sprintf(_buf, "symbol '%s' is already defined at line %d.", name, getDeclLineno(sym));
      ERROR_MSG(SYMBOL_REDIFINITION, tnode->lineno, _buf);
      exit(-1);
    }

    /** Function Declaration **/
    if(tnode->nodekind == DeclK && tnode->kind.decl == FunDeclK) {
      sym = newSymbol(tnode, functionLocCounter);
      st_insert(scope->symtab, sym);

      ++functionLocCounter;
    }

    /** Variable Declaration **/
    else if(tnode->nodekind == DeclK && tnode->kind.decl == VarDeclK) {
      if(tnode->type == VoidK) {
        sprintf(_buf, "variable '%s' cannot be of type 'void'.", name);
        ERROR_MSG(VARIABLE_HAS_INCOMPLETE_TYPE, tnode->lineno, _buf);
        exit(-1);
      }

      int arrSize = tnode->child[0]->attr.val;
      if(arrSize == 0) {
        ERROR_MSG(ZERO_SIZED_ARRAY_DECLARATION, tnode->lineno, "");
        exit(-1);
      }
      if(arrSize == -1) arrSize = 1;

      /* local variable symbols */
      if(scope->scopeId > 0) {
        scope->stackCounter -= 4 * arrSize;
        scope->blockSize += 4* arrSize;
        sym = newSymbol(tnode, scope->stackCounter);
      }
      /* global variable symbols */
      else {
        scope->stackCounter += 4 * arrSize;
        sym = newSymbol(tnode, scope->stackCounter);
      }

      tnode->scope_ref = getCurrentScope();
      st_insert(scope->symtab, sym);
    }
    // parameter
    else {
      if(tnode->type == VoidK) {
        sprintf(_buf, "parameter '%s' cannot be of type 'void'.", name);
        ERROR_MSG(PARAMETER_HAS_INCOMPLETE_TYPE, tnode->lineno, _buf);
        exit(-1);
      }

      scope->stackCounter -= 4;
      sym = newSymbol(tnode, scope->stackCounter);

      tnode->scope_ref = getCurrentScope();
      st_insert(scope->symtab, sym);

      // you don't have to care whether the parameter is of type array or not
    }

    break;
  }
  default:
    break;
  }

  /* **** WORKING WITH SCOPES **** */
  switch (tnode->nodekind) {
  case StmtK:
    if(tnode->kind.stmt != CompdK) break;

    if(functionFlag) {
      // This compound statement is a function body
      // The function has already created new scope for this block
      // Thus, you don't have to enter the scope
      functionFlag = 0;
    }
    else enterScope();

    getCurrentScope()->stackCounter = -4;
    getCurrentScope()->blockSize = 0;
    if(h_scopeStack > 2) {
      getCurrentScope()->stackCounter = getPrevScope()->stackCounter;
    }

    break;

  case DeclK:
    if(tnode->kind.decl != FunDeclK) break;

    /** Function Declaration **/

    if(mainFlag) {
      // this function appears after the main function
      ERROR_MSG(MAIN_FUNCTION_MUST_APPEAR_LAST, tnode->lineno, "");
      exit(-1);
    }

    functionFlag = 1;
    enterScope();

    // calculate address of top address of topmost parameter
    int nParams = 0;
    TreeNode *paramNode = tnode->child[1];
    if(paramNode->nChildren > 0) {
      while(paramNode != NULL) {
        ++nParams;
        paramNode = paramNode->sibling;
      }
    }

    getCurrentScope()->stackCounter = 4 + 4*nParams;

    // check if 'main' function
    if(strcmp(tnode->attr.name, "main") == 0) {
      if(tnode->type != VoidK) {
        // return type is not 'void'
        ERROR_MSG(MAIN_FUNCTION_RETURN_TYPE_MUST_BE_VOID, tnode->lineno, "");
        exit(-1);
      }
      if(tnode->child[1]->nChildren > 0) {
        // non-void parameters
        ERROR_MSG(MAIN_FUNCTION_PAARM_TYPE_MUST_BE_VOID, tnode->lineno, "");
        exit(-1);
      }

      // no error, this is the valid main function
      mainFlag = 1;
    }

    break;

  default:
    break;
  }

}

static void buildSymtab_post(TreeNode *tnode) {
  switch (tnode->nodekind) {
  case StmtK:
    if(tnode->kind.stmt != CompdK) break;
    
    // store scope reference to AST node
    tnode->scope_ref = exitScope();
    break;

  case ExprK: {
    if(tnode->kind.expr != VarK && tnode->kind.expr != CallK) break;

    // var-expression / call-expression
    char const *name = tnode->attr.name;
    struct SymbolRec *sym = lookupSymbol(name);
    if(sym != NULL)  {
      addLineno(sym, tnode->lineno);
      tnode->loc = getMemLoc(sym);
      tnode->scope_ref = getTreeNode(sym)->scope_ref;
    }
    else {
      char _buf[128];
      sprintf(_buf, "identifier '%s' cannot be resolved.", name);
      ERROR_MSG(IDENTIFIER_NOT_FOUND, tnode->lineno, _buf);
      exit(-1);
    }
    break;
  }
  default:
    break;
  }
}

static void addExternalFunctions(void) {
  TreeNode *declRoot;
  TreeNode *paramNode;
  void *sym;

  /* int input(void) */
  declRoot = newDeclNode(FunDeclK);
  declRoot->child[0] = newTypeNode();
  declRoot->child[0]->type = IntK;
  declRoot->attr.name = "input";
  declRoot->child[1] = newParamNode();
  declRoot->child[1]->nChildren = 0;
  declRoot->child[2] = newStmtNode(CompdK);
  declRoot->type = IntK;

  sym = newSymbol(declRoot, functionLocCounter);
  st_insert(getCurrentScope()->symtab, sym);
  ++functionLocCounter;

  externDecl = declRoot;

  /* void output(int) */
  declRoot = newDeclNode(FunDeclK);
  declRoot->child[0] = newTypeNode();
  declRoot->child[0]->type = VoidK;
  declRoot->attr.name = "output";

  paramNode = newParamNode();
  paramNode->child[0] = newTypeNode();
  paramNode->child[0]->type = IntK;
  paramNode->attr.name = "";
  paramNode->type = IntK;

  declRoot->child[1] = paramNode;
  declRoot->child[2] = newStmtNode(CompdK);
  declRoot->type = VoidK;

  sym = newSymbol(declRoot, functionLocCounter);
  st_insert(getCurrentScope()->symtab, sym);
  ++functionLocCounter;

  externDecl->sibling = declRoot;
}

void buildSymtab(TreeNode *syntaxTree) {
  len_scopeWholeList = 0;
  h_scopeStack = 0;
  functionFlag = 0;
  mainFlag = 0;
  scopeIdCounter = 0;
  functionLocCounter = 0;

  // enter the global scope
  enterScope();
  getCurrentScope()->stackCounter = 0;
  getCurrentScope()->blockSize = 0;   // never used for global scope

  addExternalFunctions();

  traverseSiblings(syntaxTree, buildSymtab_pre, buildSymtab_post);

  // exit the global scope
  exitScope();

  assert(h_scopeStack == 0);
  assert(functionFlag == 0);
  assert(mainFlag == 1);

  if(!mainFlag) {
    // main function has never been found
    ERROR_MSG(MAIN_FUNCTION_NOT_EXISTS, 0, "");
    exit(-1);
  }

  if(TraceAnalyze) {
    int i;
    for(i=0; i<len_scopeWholeList; ++i) {
      printSymbolTable(listing, scopeWholeList[i].symtab, scopeWholeList[i].scopeDepth);
    }
  }
}

static void typeCheck_pre(TreeNode *tnode) {
  if(tnode->nodekind == StmtK && tnode->kind.stmt == CompdK) {
    enterExistingScope(tnode->scope_ref);
  } else if (tnode->nodekind == DeclK && tnode->kind.decl == FunDeclK) {
    funcName = tnode->attr.name;
  }
}

static void typeCheck_post(TreeNode *tnode) {
  NodeKind nodekind = tnode->nodekind;

  if(nodekind == ExprK) {
    ExprKind kind = tnode->kind.expr;

    if(kind == OpExprK) {
      if (tnode->attr.op == LBRACKET) {
        TreeNode *indexVal = tnode->child[1];
        if (indexVal->type == VoidK) {
          ERROR_MSG(ARRAY_SUBSCRIPT_TYPE_ERROR, tnode->lineno, "");
          exit(-1);
        }

        // check subscipted variable is array variable
        char *name = tnode->child[0]->attr.name;
        TreeNode *varNode = getTreeNode(lookupSymbol(name));
        int arrSize = varNode->child[0]->attr.val;

        if(arrSize == -1) {
          ERROR_MSG(SUBSCRIPTED_VALUE_TYPE_ERROR, tnode->lineno, "");
          exit(-1);
        }

        tnode->type = IntK;
      } else if (tnode->attr.op == ASSIGN) {
        TreeNode *lhs = tnode->child[0];
        TreeNode *rhs = tnode->child[1];
        if (lhs->type == VoidK) {
          ERROR_MSG(EXPRESSION_IS_NOT_ASSIGNABLE, tnode->lineno, "");
          exit(-1);
        }
        else if (rhs->type == VoidK) {
          ERROR_MSG(INCOMPATIBLE_ASSIGNMENT_ERROR, tnode->lineno, "");
          exit(-1);
        } else {
          tnode->type = rhs->type;
        }
      } else {
        TreeNode *left = tnode->child[0];
        TreeNode *right = tnode->child[1];
        // TODO MORE
        char _buf[128];
        char _left_type[10];
        char _right_type[10];
        if (left->type == VoidK || right->type == VoidK) {
          if (left->type == VoidK) sprintf(_left_type, "void");
          else sprintf(_left_type, "int");
          if (right->type == VoidK) sprintf(_right_type, "void");
          else sprintf(_right_type, "int");
          sprintf(_buf, "operand1 has type %s, operand2 has type %s", _left_type, _right_type);
          ERROR_MSG(INVALID_OPERANDS_BINARY_OPERATION, tnode->lineno, _buf);
          exit(-1);
        } else {
          tnode->type = IntK;
        }
      }
    }
    else if(kind == ConstK) {
      tnode->type = IntK;
    }
    else if(kind == VarK) {
      char const *name = tnode->attr.name;
      TreeNode *symNode = getTreeNode(lookupSymbol(name));
      assert(symNode != NULL);
      assert(symNode->type == IntK);
      tnode->type = symNode->type; 
    }
    else if(kind == CallK) {
      char const *name = tnode->attr.name;
      TreeNode *symNode = getTreeNode(lookupSymbol(name));
      assert(symNode != NULL);
      
      TreeNode *symParam = symNode->child[1];
      if(symParam->nChildren == 0) symParam = NULL;
      TreeNode *nowParam = tnode->child[0];

      while(nowParam) {
        if (symParam==NULL){
          ERROR_MSG(TOO_MANY_ARGUMENTS_ERROR, tnode->lineno, "");
          exit(-1);
        }
        if (nowParam->type == VoidK) {
          ERROR_MSG(
              INCOMPATIBLE_PARAMETER_PASSING, tnode->lineno,
              "expected int but actual was void");
          exit(-1);
        } else{
          nowParam = nowParam->sibling;
          symParam = symParam->sibling; 
        }
      }
      if (symParam) {
        ERROR_MSG(TOO_FEW_ARGUMENTS_ERROR, tnode->lineno, "");
        exit(-1);
      }
      tnode->type = symNode->type;
    }
    else {
      // UNREACHABLE
      assert(0);
    }
  }
  else if(nodekind == StmtK) {
    ExprKind kind = tnode->kind.expr;

    if(kind == CompdK) {
      exitScope();
    }
    else if(kind == SelectK) {
      if (tnode->child[0]->type == VoidK) {
        ERROR_MSG(
            STATEMENT_EXPRESSION_TYPE_ERROR, tnode->lineno,
            "'if' statement requires expression of type 'int'");
        exit(-1);
      }
    }
    else if(kind == IterK) {
      if (tnode->child[0]->type == VoidK) {
        ERROR_MSG(
            STATEMENT_EXPRESSION_TYPE_ERROR, tnode->lineno,
            "'while' statement requires expression of type 'int'");
        exit(-1);
      }
    }
    else if(kind == RetK) {
      // requires the function to match RETURN statement against
      TreeNode *symNode = getTreeNode(lookupSymbol(funcName));
      assert(symNode != NULL);
      
      TypeKind nowType;
      if (tnode->nChildren > 0){
        nowType = tnode->child[0]->type;
      } else {
        nowType = VoidK;
      }

      if (symNode->type != nowType) {
        ERROR_MSG(RETURN_TYPE_MISMATCH_ERROR, tnode->lineno, "");
        exit(-1);
      }
    }
    else {
      // UNREACHABLE
      assert(0);
    }
  }
}

void typeCheck(TreeNode *syntaxTree) {
  h_scopeStack = 0;

  // scopeWholeList[0] == global scope
  enterExistingScope(&scopeWholeList[0]);
  traverseSiblings(syntaxTree, typeCheck_pre, typeCheck_post);
  exitScope();
}
