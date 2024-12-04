/*
 * expr_tree.h
 * 
 * A dynamically allocated tree to handle arbitrary expressions
 *
 * Author: <Uwase Pauline>
 */

#ifndef _EXPR_TREE_H_
#define _EXPR_TREE_H_


#include <stdio.h>
#include <string.h>

typedef struct _expr_tree_node * ExprTree;

typedef enum {
  VALUE,
  UNARY_NEGATE,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_POWER
} ExprNodeType;


/*
 * Create a value node on the tree. A value node is always a leaf.
 *
 * Parameters:
 *   value    The value for the leaf node
 * 
 * Returns:
 *   The new tree, which will consist of a single leaf node
 * 
 * It is the responsibility of the caller to call ET_free on a tree
 * that contains this leaf.
 */
ExprTree ET_value(double value);


/*
 * Create an interior node on tree. An interior node always represents
 * an arithmetic operation.
 *
 * Parameters:
 *   op       The operator
 *   left     Left side of the operator
 *   right    Right side of the operator
 * 
 * Returns: The new tree, which will consist of an interior node with
 *   two children.
 * 
 * It is the responsibility of the caller to call ET_free on a tree
 * that contains this leaf
 */
ExprTree ET_node(ExprNodeType op, ExprTree left, ExprTree right);


/*
 * Destroy an ExprTree, calling free() on all malloc'd memory
 *
 * Parameters:
 *   tree     The tree; if NULL, no action will occur
 * 
 * Returns: None
 */
void ET_free(ExprTree tree);


/*
 * Return the number of nodes in the tree, including both leaf and
 * interior nodes in the count.
 *
 * Parameters:
 *   tree     The tree 
 * 
 * Returns: The number of nodes
 */
int ET_count(ExprTree tree);


/*
 * Return the maximum depth for the tree. A tree that contains just a
 * single leaf node has a depth of 1.
 *
 * Parameters:
 *   tree     The tree 
 * 
 * Returns: The maximum depth. 
 */
int ET_depth(ExprTree tree);


/*
 * Evaluate an ExprTree and return the resulting value
 *
 * Parameters:
 *   tree     The tree to compute
 * 
 * Returns: The computed value
 */
double ET_evaluate(ExprTree tree);


/*
 * Convert an ExprTree into a printable ASCII string stored in buf
 *
 * Parameters:
 *   tree     The tree
 *   buf      The buffer 
 *   buf_sz   Size of buffer, in bytes
 * 
 * If it takes more characters to represent tree than are allowed in
 * buf, this function places as many characters as will fit into buf,
 * followed by a '$' character to indicate the result was truncated.
 * Buf will always be terminated with a \0 and in no case will this
 * function write characters beyond the end of buf.
 * 
 * Returns: The number of characters written to buf, not counting the
 * \0 terminator.
 */
size_t ET_tree2string(ExprTree tree, char *buf, size_t buf_sz);


#endif /* _EXPR_TREE_H_ */
