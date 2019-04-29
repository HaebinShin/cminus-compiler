%{
  #include "globals.h"
  int extern yylex(void);
  static void yyerror(char const *s);
%}
%%
nothing: ' '
%%

TreeNode *parse(void) {
  return NULL;
}

static void yyerror(char const *s) {
  fprintf (stderr, "%s\n", s);
}