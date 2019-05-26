#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 8

#define ENDFILE 0
#define ERROR 1
#define BLKCOMMENT 2
typedef int TokenType;

extern FILE *source; /* source code text file */
extern FILE *listing; /* listing output text file */
extern FILE *code; /* code text file for TM simulator */

extern int lineno; /* source line number for listing */

/**** Syntax tree for parsing ****/

typedef enum { DeclK, ParamK, StmtK, ExprK, TypeK } NodeKind;
typedef enum { VarDeclK, FunDeclK } DeclKind;
typedef enum { /*ExprStmtK, */CompdK, SelectK, IterK, RetK } StmtKind;
typedef enum { VarK, OpExprK, CallK, ConstK } ExprKind;

/* ExpType is used for type checking */
typedef enum { VoidK, IntK } TypeKind;

#define MAXCHILDREN 7

typedef struct treeNode {
    int nChildren;
    struct treeNode *child[MAXCHILDREN];
    struct treeNode *sibling;
    int lineno;
    NodeKind nodekind;
    union { DeclKind decl; StmtKind stmt; ExprKind expr; } kind;
    union {
        TokenType op;
        int val;
        char *name;
        void *scope_ref;
    } attr;
    TypeKind type;
} TreeNode;

#ifndef YYPARSER
#include "cminus.tab.h"
#endif

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* EchoSource = TRUE causes the source program to
 * be echoed to the listing file with line numbers
 * during parsing
 */
extern int EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern int TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern int Error; 
#endif 