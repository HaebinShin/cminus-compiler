%{
  #include "globals.h"
  #include "util.h"
  #include "scan.h"
  #include "cminus.tab.h"
  char tokenString[MAXTOKENLEN+1];

  static enum {
      NO_ERROR, INVALID_TOKEN_ERROR, COMMENT_ERROR
  } errorType = NO_ERROR;

  static void _printToken(TokenType);
%}

letter [A-Za-z]
number {digit}+
digit [0-9]
identifier {letter}+
newline     \n
whitespace  [ \t]+

%%

{newline} { ++lineno; }
{whitespace} {/* skip whitespace */}
else return ELSE;
if return IF;
int return INT;
return return RETURN;
void return VOID;
while return WHILE;
"+" return PLUS;
"-" return MINUS;
"*" return STAR;
"/" return SLASH;
"<" return LT;
"<=" return LE;
">" return GT;
">=" return GE;
"==" return EQ;
"!=" return NE;
"=" return ASSIGN;
";" return SEMI;
"," return COMMA;
"(" return LPAREN;
")" return RPAREN;
"[" return LBRACKET;
"]" return RBRACKET;
"{" return LBRACE;
"}" return RBRACE;
{identifier} { yylval.identifier = copyString(yytext); return ID; }
{number} { yylval.number = atoi(yytext); return NUM; }
"/*" {
  int ch;
  int flag = 0;
  while((ch = input()) != EOF) {
    if(ch == '*') {
      flag = 1;
    }
    else if(flag == 1 && ch == '/') {
      break;
    }
    else {
      flag = 0;
      if(ch == '\n') ++lineno;
    }
  }
  if(ch == EOF) {
    errorType = COMMENT_ERROR;
    _printToken(ERROR);
  }
}
<<EOF>> { 
  return ENDFILE;
}

. {
  errorType = INVALID_TOKEN_ERROR;
  return ERROR;
}

%%

TokenType getToken(void) {
  TokenType currentToken = yylex();
  strncpy(tokenString, yytext, MAXTOKENLEN);
  _printToken(currentToken);
  return currentToken;
}
static void _printToken(TokenType currentToken) {
  if(TraceScan) {
    fprintf(listing, "\t%d\t\t", lineno);
    fprintf(listing, "%s\t\t", getTokenName(currentToken));
    if(currentToken == ERROR) {
      if(errorType == COMMENT_ERROR) {
        fprintf(listing, "Comment Error\n");
      }
      else if(errorType == INVALID_TOKEN_ERROR) {
        fprintf(listing, "Invalid Token Error\n");
      }
      else {
        fprintf(listing, "-- Unknown Error\n");
      }
    }
    else if(currentToken == BLKCOMMENT) {
      fprintf(listing, "\n");
    }
    else {
      fprintf(listing, "%s\n", tokenString);
    }
  }

FUNC_END:
  ;
}