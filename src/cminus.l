%{
  #include "globals.h"
  #include "util.h"
  #include "scan.h"
  #include "cminus.tab.h"
  char tokenString[MAXTOKENLEN+1];

  static enum _ErrorType {
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
    Error = TRUE;
    return ERROR;
  }
}
<<EOF>> { 
  return ENDFILE;
}

. {
  errorType = INVALID_TOKEN_ERROR;
  Error = TRUE;
  return ERROR;
}

%%

TokenType getToken(void) {
  TokenType currentToken = yylex();
  strncpy(tokenString, yytext, MAXTOKENLEN);
  _printToken(currentToken);
  return currentToken;
}

static char const *getErrorText(enum _ErrorType errorType) {
  if(errorType == COMMENT_ERROR) return "Comment Error";
  else if(errorType == INVALID_TOKEN_ERROR) return "Invalid Token Error";
  return "-- Unknown Error";
}

static void _printToken(TokenType currentToken) {
  if(TraceScan) {
    fprintf(listing, "\t%d\t\t", lineno);
    fprintf(listing, "%s\t\t", getTokenName(currentToken));
    if(currentToken == ERROR) {
      fprintf(listing, "%s\n", getErrorText(errorType));
    }
    else {
      fprintf(listing, "%s\n", tokenString);
    }
  }

  if(currentToken == ERROR) {
    fprintf(listing, "Lexical error at line %d\n", lineno);
    fprintf(listing, "%s\n", getErrorText(errorType));
  }
}