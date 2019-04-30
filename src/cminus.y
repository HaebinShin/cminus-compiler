%{
  #define YYPARSER

  #include "globals.h"
  #include "util.h"
  #include "scan.h"
  #include "parse.h"

  #define YYSTYPE TreeNode *
  static char *savedName;
  static int saveLineno;
  static TreeNode *savedTree;

  static int yyerror(char const *message);
  static int yylex(void);
%}

%token ELSE IF INT RETURN VOID WHILE
%token NUM
%token ID
%token PLUS MINUS STAR SLASH LT LE GT GE EQ NE ASSIGN SEMI COMMA
%token LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE

%%
program: declaration-list;
declaration-list: declaration-list declaration | declaration;
declaration: var-declaration | fun-declaration;
var-declaration: type-specifier ID SEMI | type-specifier ID LBRACKET NUM RBRACKET SEMI;
type-specifier: INT | VOID;
fun-declaration: type-specifier ID LPAREN params RPAREN compound-stmt;
params: param-list | VOID;
param-list: param-list COMMA param | param;
param: type-specifier ID | type-specifier ID LBRACKET RBRACKET;
compound-stmt: LBRACE local-declarations statement-list RBRACE;
local-declarations: local-declarations var-declaration | /* empty */;
statement-list: statement-list statement | /* empty */;
statement: expression-stmt | compound-stmt | selection-stmt | iteration-stmt | return-stmt;
expression-stmt: expression SEMI | SEMI;
selection-stmt: IF LPAREN expression RPAREN statement
  | IF LPAREN expression RPAREN statement ELSE statement;
iteration-stmt: WHILE LPAREN expression RPAREN statement;
return-stmt: RETURN SEMI | RETURN expression SEMI;
expression: var ASSIGN expression | simple-expression;
var: ID | ID LBRACKET expression RBRACKET;
simple-expression: additive-expression relop additive-expression | additive-expression;
relop: LE | LT | GT | GE | EQ | NE;
additive-expression: additive-expression addop term | term;
addop: PLUS | MINUS;
term: term mulop factor | factor;
mulop: STAR | SLASH;
factor: LPAREN expression RPAREN | var | call | NUM;
call: ID LPAREN args RPAREN;
args: arg-list | /* empty */;
arg-list: arg-list COMMA expression | expression;
%%

static int yyerror(char const *message) {
  fprintf(listing, "Syntax error at line %d: %s\n", lineno, message);
  fprintf(listing, "Current token: ");
  printToken(yychar, tokenString);
  Error = TRUE;
  return 0;
}

static int yylex(void) {
  return getToken();
}

TreeNode *parse(void) {
  yyparse();
  return savedTree;
}
