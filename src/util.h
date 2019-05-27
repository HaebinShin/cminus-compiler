/****************************************************/
/* File: util.h                                     */
/* Utility functions for the C- compiler          */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _UTIL_H_
#define _UTIL_H_

enum AnalyzeError {
    MAIN_FUNCTION_NOT_EXISTS,
    MAIN_FUNCTION_MUST_APPEAR_LAST,
    MAIN_FUNCTION_PAARM_TYPE_MUST_BE_VOID,
    MAIN_FUNCTION_RETURN_TYPE_MUST_BE_VOID,
    SYMBOL_REDIFINITION,
    VARIABLE_HAS_INCOMPLETE_TYPE,
    PARAMETER_HAS_INCOMPLETE_TYPE,
    IDENTIFIER_NOT_FOUND,
    ZERO_SIZED_ARRAY_DECLARATION,
    ARRAY_SUBSCRIPT_TYPE_ERROR,
    SUBSCRIPTED_VALUE_TYPE_ERROR,
    EXPRESSION_IS_NOT_ASSIGNABLE,
    INCOMPATIBLE_ASSIGNMENT_ERROR,
    INVALID_OPERANDS_BINARY_OPERATION,
    TOO_MANY_ARGUMENTS_ERROR,
    TOO_FEW_ARGUMENTS_ERROR,
    INCOMPATIBLE_PARAMETER_PASSING,
    STATEMENT_EXPRESSION_TYPE_ERROR,
    RETURN_TYPE_MISMATCH_ERROR
};

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
void printToken(TokenType, char const*);

TreeNode *newDeclNode(DeclKind);

TreeNode *newParamNode(void);

TreeNode *newTypeNode(void);

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode* newStmtNode(StmtKind);

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode* newExprNode(ExprKind);

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char* copyString(char*);

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode*);

void analyzeErrorMsg(enum AnalyzeError err, int lineno, char const *msg);

#endif