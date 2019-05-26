#include "globals.h"
#include "util.h"
#include "symtab.h"
#include "analyze.h"

#define MAX_SCOPES_COUNT 50
#define MAX_SCOPE_DEPTH 20
#define ERROR_MSG(a, b, c) (analyzeErrorMsg(a, b, c))

struct ScopeRec {
  int scopeId;
  int scopeDepth;
  int stackCounter;
  BucketList *symtab;
};

typedef void (*TraverseFunc)(TreeNode *);

/* Whole list of scopes created so far */
/* scopeWholeLilst[0] EQUALS the global scope */
static struct ScopeRec scopeWholeList[MAX_SCOPES_COUNT];
static int len_scopeWhileList;

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

static void enterScope(void) {
  struct ScopeRec *new_scope = &scopeWholeList[len_scopeWhileList++];
  new_scope->scopeId = scopeIdCounter++;
  new_scope->symtab = constructSymtab();
  new_scope->scopeDepth = h_scopeStack;
  
  scopeStack[h_scopeStack] = new_scope;
  ++h_scopeStack;
}

static void exitScope(TreeNode *tnode) {
  struct ScopeRec *exitingScope = scopeStack[--h_scopeStack];

  if(tnode != NULL) {
    // not a global scope
    tnode->attr.scope_ref = (void *)exitingScope;
  }
}

static struct ScopeRec *getCurrentScope() {
  return scopeStack[h_scopeStack - 1];
}

static void buildSymtab_pre(TreeNode *tnode) {
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
    getCurrentScope()->stackCounter = 4;

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
    exitScope(tnode);
    break;

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

      /* global variable symbols */
      if(scope->scopeId > 0) {
        scope->stackCounter -= 4 * arrSize;
        sym = newSymbol(tnode, scope->stackCounter);
      }
      /* local variable symbols */
      else {
        sym = newSymbol(tnode, scope->stackCounter);
        scope->stackCounter += 4 * arrSize;
      }

      st_insert(scope->symtab, sym);
    }
    // parameter
    else {
      if(tnode->type == VoidK) {
        sprintf(_buf, "parameter '%s' cannot be of type 'void'.", name);
        ERROR_MSG(PARAMETER_HAS_INCOMPLETE_TYPE, tnode->lineno, _buf);
        exit(-1);
      }

      sym = newSymbol(tnode, scope->stackCounter);
      st_insert(scope->symtab, sym);

      scope->stackCounter += 4;

      // you don't have to care whether the parameter is of type array or not
    }

    break;
  }
  case ExprK: {
    if(tnode->kind.expr != VarK && tnode->kind.expr != CallK) break;

    // var-expression / call-expression
    char const *name = tnode->attr.name;
    int i = h_scopeStack - 1;
    while(i >= 0) {
      struct ScopeRec *scope = scopeStack[i];
      struct SymbolRec *sym = st_lookup(scope->symtab, name);
      if(sym != NULL)  {
        addLineno(sym, tnode->lineno);
        break;
      }

      --i;
    }

    if(i < 0) {
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

void buildSymtab(TreeNode *syntaxTree) {
  len_scopeWhileList = 0;
  h_scopeStack = 0;
  functionFlag = 0;
  mainFlag = 0;
  scopeIdCounter = 0;
  functionLocCounter = 0;

  // enter the global scope
  enterScope();
  getCurrentScope()->stackCounter = 0;

  traverseSiblings(syntaxTree, buildSymtab_pre, buildSymtab_post);

  // exit the global scope
  exitScope(NULL);

  if(!mainFlag) {
    // main function has never been found
    ERROR_MSG(MAIN_FUNCTION_NOT_EXISTS, 0, "");
    exit(-1);
  }

  if(TraceAnalyze) {
    int i;
    for(i=0; i<len_scopeWhileList; ++i) {
      printSymbolTable(listing, scopeWholeList[i].symtab, scopeWholeList[i].scopeDepth);
    }
  }
}

void typeCheck(TreeNode *tnode) {
  // TODO
  printf("typeCheck! called");
}