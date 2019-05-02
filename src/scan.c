#include "globals.h"
#include "scan.h"

#define GENERATE_CASE(x) case x: return #x;

#ifdef NO_PARSE
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
#endif

char const *getTokenName(TokenType type) {
  #ifdef NO_PARSE
  return _tokenNames[type];
  #endif

  switch (type) {
    case ENDFILE: return "EOF";
    GENERATE_CASE(ERROR)
    GENERATE_CASE(BLKCOMMENT)
    GENERATE_CASE(ELSE)
    GENERATE_CASE(IF)
    GENERATE_CASE(INT)
    GENERATE_CASE(RETURN)
    GENERATE_CASE(VOID)
    GENERATE_CASE(WHILE)
    GENERATE_CASE(ID)
    GENERATE_CASE(NUM)
    case PLUS: return "+";
    case MINUS: return "-";
    case STAR: return "*";
    case SLASH: return "/";
    case LT: return "<";
    case LE: return "<=";
    case GT: return ">";
    case GE: return ">=";
    case EQ: return "==";
    case NE: return "!=";
    case ASSIGN: return "=";
    case SEMI: return ";";
    case COMMA: return ",";
    case LPAREN: return "(";
    case RPAREN: return ")";
    case LBRACKET: return "[";
    case RBRACKET: return "]";
    case LBRACE: return "{";
    case RBRACE: return "}";
    default:
      return "";
  }
}