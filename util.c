/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the CMINUS compiler                          */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */

void printToken( TokenType token, const char* tokenString )
{ 
  fprintf(listing,"\t%d\t\t",lineno);
  switch (token)
  {
    case NUM: fprintf(listing, "%-20s\t%-20s\n", "NUM", tokenString); break;
    case ID: fprintf(listing, "%-20s\t%-20s\n", "ID", tokenString); break;

    case ELSE: fprintf(listing, "%-20s\t%-20s\n", "ELSE", tokenString); break;
    case IF: fprintf(listing, "%-20s\t%-20s\n", "IF", tokenString); break;
    case INT: fprintf(listing, "%-20s\t%-20s\n", "INT", tokenString); break;
    case RETURN: fprintf(listing, "%-20s\t%-20s\n", "RETURN", tokenString); break;
    case VOID: fprintf(listing, "%-20s\t%-20s\n", "VOID", tokenString); break;
    case WHILE: fprintf(listing, "%-20s\t%-20s\n", "WHILE", tokenString); break;

    case PLUS:
    case MINUS:
    case TIMES:
    case OVER:
    case LT:
    case LTEQ:
    case GT:
    case GTEQ:
    case EQ:
    case NOTEQ: 
    case ASSIGN:
    case SEMI:
    case COMMA:
    case LPAREN:
    case RPAREN:
    case LBRACKET:
    case RBRACKET:
    case LBRACE:
    case RBRACE: 
	fprintf(listing, "%-20s\t%-20s\n", tokenString, tokenString); break;
    
    case ERROR: 
	fprintf(listing, "%-20s\t%-20s\n", "ERROR", tokenString); break;
    case ENDFILE: 
	fprintf(listing, "%-20s\t\n", "EOF"); break;
    case COMMENT_ERROR: 
	fprintf(listing, "%-20s\t%-20s\n", "ERROR", "Comment Error"); break;
    
    default: /* should never happen */
      fprintf(listing,"Unknown token: %d\n",token);
  }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode * newStmtNode(StmtKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
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
TreeNode * newExpNode(ExpKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->type = Void;
  }
  return t;
}

/* Function newDeclNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode * newDeclNode(DeclKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = DeclK;
    t->kind.decl = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function newParamNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode * newParamNode(ParamKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ParamK;
    t->kind.type = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function newTypeNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode * newTypeNode(TypeKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = TypeK;
    t->kind.type = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString(char * s)
{ int n;
  char * t;
  if (s==NULL) return NULL;
  n = strlen(s)+1;
  t = malloc(n);
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else strcpy(t,s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{ int i;
  for (i=0;i<indentno;i++)
    fprintf(listing," ");
}

static void printOp(TokenType op)
{ switch (op)
    { 
    case PLUS: fprintf(listing,"+\n"); break;
    case MINUS: fprintf(listing,"-\n"); break;
    case TIMES: fprintf(listing,"*\n"); break;
    case OVER: fprintf(listing,"/\n"); break;
    case LT: fprintf(listing,"<\n"); break;
    case LTEQ: fprintf(listing,"<=\n"); break;
    case GT: fprintf(listing,">\n"); break;
    case GTEQ: fprintf(listing,">=\n"); break;
    case EQ: fprintf(listing,"==\n"); break;
    case NOTEQ: fprintf(listing,"!=\n"); break;
    default: /* should never happen */
      fprintf(listing,"Unknown op: %d\n", op);
    }
}

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree( TreeNode * tree )
{ int i;
  INDENT;
  while (tree != NULL) {
    printSpaces();
    if (tree->nodekind==StmtK)
    { switch (tree->kind.stmt) {
        case CompK:
          fprintf(listing,"Compound Statement\n");
          break;
        case IfK:
          fprintf(listing,"If\n");
          break;
        case IterK:
          fprintf(listing,"While\n");
          break;
        case RetK:
          fprintf(listing,"Return\n");
          break;
        default:
          fprintf(listing,"Unknown StmtNode kind\n");
          break;
      }
    }
    else if (tree->nodekind==ExpK)
    { switch (tree->kind.exp) {
        case AssignK:
          fprintf(listing,"Op: =\n");
          break;
        case OpK:
          fprintf(listing,"Op: ");
          printOp(tree->attr.op);
          break;
        case ConstK:
          fprintf(listing,"Const: %d\n",tree->attr.val);
          break;
        case IdK:
          fprintf(listing,"Id: %s\n", tree->attr.name);
          break;
        case IdArrK:
          fprintf(listing,"Array Id: %s\n", tree->attr.name);
          break;
        case CallK:
          fprintf(listing,"Call Precedure: %s\n", tree->attr.name);
          break;
        default:
          fprintf(listing,"Unknown ExpNode kind\n");
          break;
      }
    }
    else if (tree->nodekind==DeclK)
    { switch (tree->kind.decl) {
        case FuncK:
          fprintf(listing,"Function Declaration: %s\n", tree->attr.name);
          break;
        case VarK:
          fprintf(listing,"Variable Declaration: %s\n", tree->attr.name);
          break;
        case VarArrK:
          fprintf(listing, "Array Variable Declaration: name = %s, size = %d\n", 
                  tree->attr.arrAttr.name, tree->attr.arrAttr.len);
          break;
        default:
          fprintf(listing,"Unknown DeclNode kind\n");
          break;
      }
    }
    else if (tree->nodekind==ParamK)
    { switch (tree->kind.param) {
        case ArrParamK:
          fprintf(listing,"Array Parameter: name = %s\n", tree->attr.arrAttr.name);
          break;
        case NonArrParamK:
          if (tree->attr.typeSpec == VOID)
            fprintf(listing,"Parameter: (void)\n");
          else
            fprintf(listing,"Parameter: %s\n",tree->attr.name);
          break;
        default:
          fprintf(listing,"Unknown ParamNode kind\n");
          break;
      }
    }
    else if (tree->nodekind==TypeK)
    { 
      switch (tree->attr.typeSpec)
        {
        case INT:
          fprintf(listing, "Type: int\n");
          break;
        case VOID:
          fprintf(listing, "Type: void\n");
          break;
        }
    }
    else fprintf(listing,"Unknown node kind\n");
    for (i=0;i<MAXCHILDREN;i++)
         printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}
