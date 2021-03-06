#include "globals.h"

/* set NO_PARSE to TRUE to get scanner-only compiler */
#define NO_PARSE FALSE

/* set NO_ANALYZE to TRUE to get parser-only compiler */
#define NO_ANALYZE FALSE

/* set NO_CODE to TRUE to get a compiler that does not generate code */
#define NO_CODE FALSE

#include "util.h"
#if NO_PARSE
#include "scan.h"
#else
#include "parse.h"
#if !NO_ANALYZE
#include "analyze.h"
#if !NO_CODE
#include "cgen.h"
#endif
#endif
#endif

/* allocate global variables */
int lineno = 0;
FILE *source;
FILE *listing;
FILE *code;

/* allocate and set tracing flags */
int EchoSource = TRUE;
int TraceScan = FALSE;
int TraceParse = FALSE;
int TraceAnalyze = TRUE;
int TraceCode = FALSE;

int Error = FALSE;

int main(int argc, char *argv[]) {
  TreeNode *syntaxTree;
  char pgm[120]; /* source code file name */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <filename>\n", argv[0]);
    exit(1);
  }
  strcpy(pgm, argv[1]);
  if (strchr(pgm, '.') == NULL) {
    strcat(pgm, ".cm");
  }
  source = fopen(pgm, "r");
  if (source == NULL) {
    fprintf(stderr, "File %s not found\n", pgm);
    exit(1);
  }

  listing = stdout; /* send listing to screen */

  extern FILE *yyin, *yyout;
  ++lineno;
  yyin = source;
  yyout = listing;

  if(TraceScan) {
    fprintf(listing, "line number\t\t");
    fprintf(listing, "token\t\t");
    fprintf(listing, "lexeme\n");
    fprintf(listing, "-----------------------------------------------------\n");
  }

#if NO_PARSE
  while(getToken() != ENDFILE) { /* NOOP */ }
#else
  syntaxTree = parse();
  if(TraceParse) {
    fprintf(listing, "\nSyntax tree %s:\n", Error ? "(constructed sofar)" : "");
    printTree(syntaxTree);
  }
#if !NO_ANALYZE
  if(!Error) {
    fprintf(listing, "\nBuilding Symbol Table...\n");
    buildSymtab(syntaxTree);
    fprintf(listing, "\nChecking Types...\n");
    typeCheck(syntaxTree);
    fprintf(listing, "\nType Checking Finished\n");
  }
#if !NO_CODE
  if (!Error) {
    char *codefile;
    int fnlen = strcspn(pgm, ".");
    codefile = (char *)calloc(fnlen+4, sizeof(char));
    strncpy(codefile, pgm, fnlen);
    strcat(codefile, ".tm");
    code = fopen(codefile, "w");
    if(code == NULL) {
      printf("Unable to open %s\n", codefile);
      exit(1);
    }
    codeGen(syntaxTree, codefile);
    fclose(code);
  }
#endif
#endif
#endif
  /* TODO: teardown syntax tree*/
  fclose(source);
  return 0;
}
