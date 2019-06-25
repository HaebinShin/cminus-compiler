%{
  #define YYPARSER

  #include "globals.h"
  #include "util.h"
  #include "scan.h"
  #include "parse.h"

  static char *savedName;
  static int savedLineno;
  static TreeNode *savedTree;

  static int yyerror(char const *message);
  static int yylex(void);
%}

%union {
  TreeNode *treeNode;
  TokenType tok;
  char *identifier;
  int number;
}

%type <treeNode>
  program declaration-list declaration var-declaration fun-declaration
  params param-list param local-declarations statement-list
  statement expression-stmt compound-stmt selection-stmt iteration-stmt return-stmt
  expression simple-expression additive-expression var term factor call args arg-list
  type-specifier

%type <tok>
  relop addop mulop

%token <number> NUM
%token <identifier> ID
%token <tok> ELSE IF INT RETURN VOID WHILE
%token <tok> PLUS MINUS STAR SLASH LT LE GT GE EQ NE ASSIGN SEMI COMMA
%token <tok> LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
%token <tok> LEX_ERROR

%right THEN ELSE

%%
program: { savedTree = NULL; } declaration-list {
  savedTree = $2;
}
;
declaration-list: declaration-list declaration {
  TreeNode *t = $1;
  if(t != NULL) {
    while(t->sibling != NULL) t = t->sibling;
    t->sibling = $2;
    $$ = $1;
  }
  else $$ = $2;
}
| declaration {
  $$ = $1;
}
;
declaration: var-declaration { $$ = $1; } | fun-declaration { $$ = $1; }
;
var-declaration: type-specifier ID SEMI {
  $$ = newDeclNode(VarDeclK);
  $$->child[0] = $1;
  $$->attr.name = $2;
  $$->type = $1->type;
}
| type-specifier ID LBRACKET NUM RBRACKET SEMI {
  $$ = newDeclNode(VarDeclK);
  $$->child[0] = $1;
  $$->child[0]->attr.val = $4;
  $$->attr.name = $2;
  $$->type = $1->type;
}
;
type-specifier: INT { $$ = newTypeNode(); $$->type = IntK; }
| VOID { $$ = newTypeNode(); $$->type = VoidK; }
;
fun-declaration: type-specifier ID LPAREN params RPAREN compound-stmt {
  $$ = newDeclNode(FunDeclK);
  $$->child[0] = $1;
  $$->attr.name = $2;
  $$->child[1] = $4;
  $$->child[2] = $6;
  $$->type = $1->type;
}
;
params: param-list {
  $$ = $1;
} | VOID {
  $$ = newParamNode();
  $$->nChildren = 0;
}
;
param-list: param-list COMMA param {
  TreeNode *t = $1;
  if(t != NULL) {
    while(t->sibling != NULL) t = t->sibling;
    t->sibling = $3;
    $$ = $1;
  }
  else $$ = $3;
} | param {
  $$ = $1;
}
;
param: type-specifier ID {
  $$ = newParamNode();
  $$->child[0] = $1;
  $$->attr.name = $2;
  $$->type = $1->type;
} | type-specifier ID LBRACKET RBRACKET {
  $$ = newParamNode();
  $$->child[0] = $1;
  $$->child[0]->attr.val = 0;
  $$->attr.name = $2;
  $$->type = $1->type;
}
;
compound-stmt: LBRACE local-declarations statement-list RBRACE {
  $$ = newStmtNode(CompdK);
  $$->child[0] = $2;
  $$->child[1] = $3;
}
;
local-declarations: local-declarations var-declaration {
  TreeNode *t = $1;
  if(t != NULL) {
    while(t->sibling != NULL) t = t->sibling;
    t->sibling = $2;
    $$ = $1;
  }
  else $$ = $2;
} | /* empty */ {
  $$ = NULL;
}
;
statement-list: statement-list statement {
  TreeNode *t = $1;
  if(t != NULL) {
    while(t->sibling != NULL) t = t->sibling;
    t->sibling = $2;
    $$ = $1;
  }
  else $$ = $2;
} | /* empty */ {
  $$ = NULL;
};
statement: expression-stmt { $$ = $1; }
  | compound-stmt { $$ = $1; }
  | selection-stmt { $$ = $1; }
  | iteration-stmt { $$ = $1; }
  | return-stmt { $$ = $1; }
;
expression-stmt: expression SEMI {
  $$ = $1;
} | SEMI {
  $$ = NULL;
}
;
selection-stmt: IF LPAREN expression RPAREN statement {
  $$ = newStmtNode(SelectK);
  $$->child[0] = $3;
  $$->child[1] = $5;
  $$->nChildren = 2;
} %prec THEN | IF LPAREN expression RPAREN statement ELSE statement {
  $$ = newStmtNode(SelectK);
  $$->child[0] = $3;
  $$->child[1] = $5;
  $$->child[2] = $7;
}
;
iteration-stmt: WHILE LPAREN expression RPAREN statement {
  $$ = newStmtNode(IterK);
  $$->child[0] = $3;
  $$->child[1] = $5;
}
;
return-stmt: RETURN SEMI {
  $$ = newStmtNode(RetK);
  $$->nChildren = 0;
} | RETURN expression SEMI {
  $$ = newStmtNode(RetK);
  $$->child[0] = $2;
}
;
expression: var ASSIGN expression {
  $$ = newExprNode(OpExprK);
  $$->child[0] = $1;
  $$->child[1] = $3;
  $$->attr.op = ASSIGN;
} | simple-expression {
  $$ = $1;
}
;
var: ID {
  $$ = newExprNode(VarK);
  $$->nChildren = 0;
  $$->attr.name = $1;
} | ID LBRACKET expression RBRACKET {
  $$ = newExprNode(OpExprK);
  $$->child[0] = newExprNode(VarK);
  $$->child[0]->nChildren = 0;
  $$->child[0]->attr.name = $1;
  $$->child[1] = $3;
  $$->attr.op = LBRACKET;
}
;
simple-expression: additive-expression relop additive-expression {
  $$ = newExprNode(OpExprK);
  $$->child[0] = $1;
  $$->child[1] = $3;
  $$->attr.op = $2;
} | additive-expression { $$ = $1; }
;
relop: LE { $$ = LE; } | LT { $$ = LT; } | GT { $$ = GT; } | GE { $$ = GE; }
  | EQ { $$ = EQ; } | NE { $$ = NE; }
;
additive-expression: additive-expression addop term {
  $$ = newExprNode(OpExprK);
  $$->child[0] = $1;
  $$->child[1] = $3;
  $$->attr.op = $2;
} | term { $$ = $1; }
;
addop: PLUS { $$ = PLUS; } | MINUS { $$ = MINUS; }
;
term: term mulop factor {
  $$ = newExprNode(OpExprK);
  $$->child[0] = $1;
  $$->child[1] = $3;
  $$->attr.op = $2;
} | factor { $$ = $1; }
;
mulop: STAR { $$ = STAR; } | SLASH { $$ = SLASH; }
;
factor: LPAREN expression RPAREN { $$ = $2; }
  | var { $$ = $1; } | call { $$ = $1; } 
  | NUM {
    $$ = newExprNode(ConstK);
    $$->attr.val = atoi(tokenString);
  }
;
call: ID LPAREN args RPAREN {
  $$ = newExprNode(CallK);
  $$->child[0] = $3;
  $$->attr.name = $1;
}
;
args: arg-list { $$ = $1; } | /* empty */ { $$ = NULL; };
arg-list: arg-list COMMA expression {
  TreeNode *t = $1;
  if(t != NULL) {
    while(t->sibling != NULL) t = t->sibling;
    t->sibling = $3;
    $$ = $1;
  }
  else $$ = $3;
} | expression {
  $$ = $1;
}
;
%%

static int yyerror(char const *message) {
  if(yychar == LEX_ERROR) printf("Syntax error due to Lexical error\n");
  else {
    fprintf(listing, "Syntax error at line %d: %s\n", lineno, message);
    fprintf(listing, "Current token: ");
    printToken(yychar, tokenString);
    Error = TRUE;
  }
  return 0;
}

static int yylex(void) {
  TokenType tok = getToken();
  if(Error) return LEX_ERROR;
  return tok;
}

TreeNode *parse(void) {
  yyparse();
  return savedTree;
}
