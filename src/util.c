/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "scan.h"
#include "util.h"

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
void printToken(TokenType token, const char *tokenString) {
  switch (token) {
    case ELSE:
    case IF:
    case INT:
    case RETURN:
    case VOID:
    case WHILE:
      fprintf(listing, "reserved word: %s\n", tokenString);
      break;
    case PLUS:
      fprintf(listing, "+\n");
      break;
    case MINUS:
      fprintf(listing, "-\n");
      break;
    case STAR:
      fprintf(listing, "*\n");
      break;
    case SLASH:
      fprintf(listing, "/\n");
      break;
    case LT:
      fprintf(listing, "<\n");
      break;
    case LE:
      fprintf(listing, "<=\n");
      break;
    case GT:
      fprintf(listing, ">\n");
      break;
    case GE:
      fprintf(listing, ">=\n");
      break;
    case EQ:
      fprintf(listing, "==\n");
      break;
    case NE:
      fprintf(listing, "!=\n");
      break;
    case ASSIGN:
      fprintf(listing, "=\n");
      break;
    case SEMI:
      fprintf(listing, ";\n");
      break;
    case COMMA:
      fprintf(listing, ",\n");
      break;
    case LPAREN:
      fprintf(listing, "(\n");
      break;
    case RPAREN:
      fprintf(listing, ")\n");
      break;
    case LBRACKET:
      fprintf(listing, "[\n");
      break;
    case RBRACKET:
      fprintf(listing, "]\n");
      break;
    case LBRACE:
      fprintf(listing, "{\n");
      break;
    case RBRACE:
      fprintf(listing, "]\n");
      break;
    case ENDFILE:
      fprintf(listing, "EOF\n");
      break;
    case NUM:
      fprintf(listing, "NUM, val= %s\n", tokenString);
      break;
    case ID:
      fprintf(listing, "ID, name= %s\n", tokenString);
      break;
    case ERROR:
      fprintf(listing, "ERROR: %s\n", tokenString);
      break;
    default: /* should never happen */
      fprintf(listing, "Unknown token: %d\n", token);
  }
}

TreeNode *newDeclNode(DeclKind kind) {
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else {
    int i;
    int nChildren = 1;
    if(kind == FunDeclK) nChildren = 3;
    for(i=0; i<nChildren; ++i) t->child[i] = NULL;
    t->nChildren = nChildren;
    t->sibling = NULL;
    t->nodekind = DeclK;
    t->kind.decl = kind;
    t->type = VoidK;
    t->lineno = lineno;
  }

  return  t;
}

TreeNode *newParamNode(void) {
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else {
    int i;
    int nChildren = 1;
    for(i=0; i<nChildren; ++i) t->child[i] = NULL;
    t->nChildren = nChildren;
    t->sibling = NULL;
    t->nodekind = ParamK;
    t->type= VoidK;
    t->lineno = lineno;
  }

  return t;
}

TreeNode *newTypeNode(void) {
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else {
    int i;
    int nChildren = 0;
    t->nChildren = nChildren;
    t->sibling = NULL;
    t->nodekind = TypeK;
    t->type= VoidK;
    t->attr.val = -1;
    t->lineno = lineno;
  }

  return t;
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode *newStmtNode(StmtKind kind) {
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else {
    int i;
    int nChildren = 0;
    switch(kind) {
      // case ExprStmtK: nChildren = 1; break;
      case CompdK: nChildren = 2; break;
      case SelectK: nChildren = 3; break;
      case IterK: nChildren = 2; break;
      case RetK: nChildren = 1; break;
    }
    for (i = 0; i < nChildren; i++) t->child[i] = NULL;
    t->nChildren = nChildren;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode *newExprNode(ExprKind kind) {
  TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
  int i;
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else {
    int i;
    int nChildren = 1;
    switch(kind) {
      case OpExprK:
        nChildren = 2; break;
      case ConstK:
        nChildren = 0; break;
      default: break;
    }
    for (i=0; i<nChildren; ++i) t->child[i] = NULL;
    t->nChildren = nChildren;
    t->sibling = NULL;
    t->nodekind = ExprK;
    t->kind.expr = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char *copyString(char *s) {
  int n;
  char *t;
  if (s == NULL) return NULL;
  n = strlen(s) + 1;
  t = malloc(n);
  if (t == NULL)
    fprintf(listing, "Out of memory error at line %d\n", lineno);
  else
    strcpy(t, s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno += 2
#define UNINDENT indentno -= 2

/* printSpaces indents by printing spaces */
static void printSpaces(void) {
  int i;
  for (i = 0; i < indentno; i++) fprintf(listing, " ");
}

static char const *getOperatorName(TokenType token) {
  if(token == LBRACKET) return "Array Subscript";
  else return getTokenName(token);
}

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode *tree) {
  int i;
  INDENT;
  while (tree != NULL) {
    printSpaces();
    if(tree->nodekind == DeclK) {
      switch(tree->kind.decl) {
        case VarDeclK:
          fprintf(listing, "Variable Decl: %s\n", tree->attr.name);
          break;
        case FunDeclK:
          fprintf(listing, "Function Decl: %s\n", tree->attr.name);
          break;
      }
    }
    else if(tree->nodekind == ParamK) {
      if(tree->nChildren > 0) {
        fprintf(listing, "Parameter Decl: %s\n", tree->attr.name);
      }
      else {
        fprintf(listing, "Parameter Decl: (null)\n");
      }
    }
    else if(tree->nodekind == StmtK) {
      switch (tree->kind.stmt) {
        // case ExprStmtK: break;
        case CompdK:
          fprintf(listing, "Compound statement\n");
          break;
        case SelectK:
          fprintf(listing, "If\n");
          break;
        case IterK:
          fprintf(listing, "While\n");
          break;
        case RetK:
          fprintf(listing, "Return\n");
          break;
        default:
          fprintf(listing, "Unknown StmtnNode kind\n");
          break;
      }
    }
    else if (tree->nodekind == ExprK) {
      switch (tree->kind.expr) {
        case VarK:
          fprintf(listing, "ID: %s\n", tree->attr.name);
          break;
        case OpExprK:
          fprintf(listing, "Op: %s\n", getOperatorName(tree->attr.op));
          break;
        case CallK:
          fprintf(listing, "Call procedure: %s\n", tree->attr.name);
          break;
        case ConstK:
          fprintf(listing, "Const: %d\n", tree->attr.val);
          break;
        default:
          fprintf(listing, "Unknown ExprNode kind\n");
          break;
      }
    }
    else if(tree->nodekind == TypeK) {
      if(tree->attr.val == -1)
        fprintf(listing, "Type: %s\n", tree->type == IntK ? "Int" : "Void");
      else 
        fprintf(listing, "Type: array %d\n", tree->attr.val);
    }
    else
      fprintf(listing, "Unknown node kind\n");
    for (i = 0; i < tree->nChildren; ++i) printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}

void analyzeErrorMsg(enum AnalyzeError err, int lineno, char const *msg) {
  char *canonicalErrMsg = NULL;
  switch(err) {
  case MAIN_FUNCTION_NOT_EXISTS:
    canonicalErrMsg = "function 'main' does not exist.";
    break;
  case MAIN_FUNCTION_MUST_APPEAR_LAST:
    canonicalErrMsg = "function 'main' must appear last";
    break;
  case MAIN_FUNCTION_PAARM_TYPE_MUST_BE_VOID:
    canonicalErrMsg = "function 'main' must not have parameters";
    break;
  case MAIN_FUNCTION_RETURN_TYPE_MUST_BE_VOID:
    canonicalErrMsg = "function 'main' must have a return type of 'void'";
    break;
  case SYMBOL_REDIFINITION:
    canonicalErrMsg = "symbol redefinition error";
    break;
  case VARIABLE_HAS_INCOMPLETE_TYPE:
    canonicalErrMsg = "some variable has an incomplete type";
    break;
  case PARAMETER_HAS_INCOMPLETE_TYPE:
    canonicalErrMsg = "some parameter has an incomplete type";
    break;
  case IDENTIFIER_NOT_FOUND:
    canonicalErrMsg = "symbol not found error";
    break;
  case ZERO_SIZED_ARRAY_DECLARATION:
    canonicalErrMsg = "zero sized array declaration is forbidden";
    break;
  default:
    canonicalErrMsg = "@@ UNKNOWN ERROR !! @@";
    break;
  }

  int sps = 0, i;
  sps = fprintf(stderr, "ERROR in line %d : ", lineno);
  fprintf(stderr, canonicalErrMsg);
  fputc('\n', stderr);
  fflush(stderr);

  for(i=0; i<sps; ++i) fputc(' ', stderr);
  fprintf(stderr, msg);
  fputc('\n', stderr);
  fputc('\n', stderr);
  fflush(stderr);
}