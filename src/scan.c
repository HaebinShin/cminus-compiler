#include "globals.h"
#include "scan.h"

static char const *_tokenNames[] = {
  /* book-keeping tokens */
  "EOF", "ERROR", "BLKCOMMENT",
  /* reserved words */
  "ELSE", "IF", "INT", "RETURN", "VOID", "WHILE",
  /* multicharacter tokens */
  "ID", "NUM",
  /* special symbols */
  "+", "-", "*", "/", "<", "<=", ">", ">=", "==", "!=", "=", ";", ",",
  "(", ")", "[", "]", "{", "}"
};

char const **tokenNames = _tokenNames;