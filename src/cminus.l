%{
  #include "globals.h"
  #include "scan.h"
  #include "cminus.tab.h"
  char tokenString[MAXTOKENLEN+1];

  static enum {
      NO_ERROR, INVALID_TOKEN_ERROR, COMMENT_ERROR
  } errorType = NO_ERROR;
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
{identifier} return ID;
{number} return NUM;
"/*" {
  int ch;
  int flag = 0;
  while((ch = input()) != EOF) {
    if(ch == '*') {
      flag = 1;
    }
    else if(flag == 1 && ch == '/') {
      return BLKCOMMENT;
    }
    else {
      flag = 0;
      if(ch == '\n') ++lineno;
    }
  }
  if(ch == EOF) {
    errorType = COMMENT_ERROR;
    return ERROR;
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

/* obsolete */
TokenType getToken(void) {
  static int firstTime = TRUE;
  TokenType currentToken;
  if(firstTime) {
    firstTime = FALSE;
    ++lineno;
    yyin = source;
    yyout = listing;

    if(TraceScan) {
      fprintf(listing, "line number\t\t");
      fprintf(listing, "token\t\t");
      fprintf(listing, "lexeme\n");
      fprintf(listing, "-----------------------------------------------------\n");
    }
  }
  currentToken = yylex();
  if(currentToken == BLKCOMMENT) goto FUNC_END;
  strncpy(tokenString, yytext, MAXTOKENLEN);
  if(TraceScan) {
    fprintf(listing, "\t%d\t\t", lineno);
    fprintf(listing, "%s\t\t", getTokenName(currentToken));
    if(currentToken != ERROR) {
      fprintf(listing, "%s\n", tokenString);
    }
    else if(errorType == COMMENT_ERROR) {
      fprintf(listing, "Comment Error\n");
    }
    else if(errorType == INVALID_TOKEN_ERROR) {
      fprintf(listing, "Invalid Token Error\n");
    }
    else {
      fprintf(listing, "-- Unknown Error\n");
    }
  }

FUNC_END:
  return currentToken;
}