// Implementace překladače imperativního jazyka IFJ26
// codegen.c
// Generátor výstupního kódu

// Autor: Martin Knotek (xknotem00)
// Autor: Filip Maxa (xmaxafi00)


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "stdFunc.h"
#include "ast.h"

#define FRAME_S(frame) \
  (frame == GF ? "GF" : (frame == TF ? "TF" : "LF"))

#define TYPE_S(type) \
  (type == TYPE_DOUBLE ? "float" : "int")

static FILE* file;

static void generate_node(const AST_Node* node)
{
  static Frame_t frame = GF;
  static unsigned scopecnt = 0;
  static unsigned cycleDepth = 0;
  static unsigned whilecnt = 0;
  static unsigned ifcnt = 0;
  static char* funcName;

  if (node == NULL) {
    return;
  }
  Frame_t prevFrame = frame;

  switch (node->node_type) {
    case NODE_PROGRAM:
      // koren definuje zabudovane funkce, ".IFJcode26" vypisuje codeGenerate
      // jinak se chova jako def list
      fprintf(file, STD_GLOBALS);
      fprintf(file, "CALL MAIN\nEXIT int@0\n");
      fprintf(file, STD_FUNC_STR0); // tisk standartnich funkci
      fprintf(file, STD_FUNC_STR1); // tisk standartnich funkci
      
    case NODE_DEF_LIST:
      // levy je nejaky def
      // pravy je node_def_list
      generate_node(node->left);
      generate_node(node->right);
      break;
    case NODE_FUNC_DEF_MAIN:
      // definuje main funkci a jinak stejne jako func def
      fprintf(file, "LABEL MAIN\n");
    case NODE_FUNC_DEF:
      // definuje funkci
      // volanim na levy podstrom zapise parametry
      // volanim na pravy podstrom zapise telo
      funcName = node->name;
      scopecnt++;
      fprintf(file, "LABEL %s\n", funcName);
      fprintf(file, "CREATEFRAME\nPUSHFRAME\n");
      fprintf(stderr, "generating: %s with %d params\n", funcName, node->symbol->paramCount);
      if(node->symbol->paramCount) generate_node(node->left);
      generate_node(node->right);
      break;
    case NODE_FUNC_CALL:
      // push args and call fun
      generate_node(node->left);
      fprintf(file, "CALL %s\n", node->name);
      break;
    case NODE_ASSIGN:
      generate_node(node->left);
      fprintf(file, "POPS LF@%s$%d\n", node->name, node->symbol->parent->tableId);
      break;
    case NODE_PARAM:
      // parametr
      // levy dalsi param
      fprintf(file, "DEFVAR LF@%s$%d\n", node->name, node->symbol->parent->tableId);
      fprintf(file, "POPS LF@%s$%d\n", node->name, node->symbol->parent->tableId);
      generate_node(node->left);
      break;
    case NODE_BODY:
      // telo obsahuje prvni prikaz vlevo a dalsi prikazy vpravo
      generate_node(node->left);
      generate_node(node->right);
      break;
    case NODE_STATEMENT_LIST:
      // levy podstrom statement, val, var, if, while, exp, return
      // pravy statement_list
      generate_node(node->left);
      generate_node(node->right);
      break;
    case NODE_RETURN:
      // levy exp
      generate_node(node->left);
      fprintf(file, "POPFRAME\nRETURN\n");
      break;
    case NODE_VAL_DEF:
      // id
      // levy expression
      if(!cycleDepth) fprintf(file, "DEFVAR LF@%s$%d\n", node->name, node->symbol->parent->tableId);
      generate_node(node->left);
      fprintf(file, "POPS LF@%s$%d\n", node->name, node->symbol->parent->tableId);
      break;
    case NODE_VAR_DEF:
      // id
      // levy expression
      if(!cycleDepth) fprintf(file, "DEFVAR LF@%s$%d\n", node->name, node->symbol->parent->tableId);
      generate_node(node->left);
      fprintf(file, "POPS LF@%s$%d\n", node->name, node->symbol->parent->tableId);
      break;
    case NODE_IF:
      // levy exp
      // pravy telo (statemet lsit)
      // next je else

      // vyhodnoceni podminky
      scopecnt++;
      generate_node(node->left); // melo by na stack hodit navratovou vec
      fprintf(file, "PUSHS bool@true\n");
      // if
      unsigned myIfCnt = ifcnt;
      ifcnt++;
      fprintf(file, "JUMPIFNEQS %s$IF$%d$else\n", funcName, myIfCnt);
      //telo if
      generate_node(node->right);
      fprintf(file, "JUMP %s$IF$%d$end\n", funcName, myIfCnt);
      //telo else
      fprintf(file, "LABEL %s$IF$%d$else\n", funcName, myIfCnt); 
      generate_node(node->next);
      //konec a vycisteni stacku
      fprintf(file, "LABEL %s$IF$%d$end\n", funcName, myIfCnt);
      scopecnt++;

      break;
    case NODE_ELSE:
      // telo levy podstrom statement list
      scopecnt++;
      generate_node(node->left);

      break;
    case NODE_WHILE_LOOP:
      // levy exp
      // pravy telo (statemet lsit)
      
      // pregenerace promennych
      unsigned myWhileCnt = whilecnt;
      whilecnt++;
      if (cycleDepth == 0) defvarPregen(node->right);
      cycleDepth++;
      scopecnt++;

      fprintf(file, "LABEL %s$while$%d$start\n", funcName, myWhileCnt); 
      // vyhodnoceni podminky
      generate_node(node->left);
      fprintf(file, "PUSHS bool@true\n");
      scopecnt++;
      // podmineny skok
      fprintf(file, "JUMPIFNEQS %s$while$%d$end\n", funcName, myWhileCnt);
      // telo while bez defvar
      generate_node(node->right);
      fprintf(file, "JUMP %s$while$%d$start\n", funcName, myWhileCnt);
      // konec
      fprintf(file, "LABEL %s$while$%d$end\n", funcName, myWhileCnt); 
      cycleDepth--;
      scopecnt++;
      break;
    case NODE_ARG_LIST:
      // levy exp
      // pravy arg_list
      generate_node(node->right);
      generate_node(node->left);
      break;
    case NODE_EXP:
      // synove val var exp
      if(node->operator_type == OT_UNARY_MINUS) fprintf(file, "PUSHS %s@0\n", TYPE_S(node->node_type));
      generate_node(node->left);
      generate_node(node->right);
      expGenerate(node);
      break;
  }

  frame = prevFrame;
  return;
}

void defvarPregen(AST_Node* node) {
  if (node == NULL) return;
  switch (node->node_type) {
    case NODE_VAL_DEF:
    case NODE_VAR_DEF:
      fprintf(file, "DEFVAR LF@%s$%d\n", node->name, node->symbol->parent->tableId);
      break;
    case NODE_FUNC_DEF:
    case NODE_FUNC_DEF_MAIN:
      return;
    default:
      break;
  }

  defvarPregen(node->left);
  defvarPregen(node->right);
  defvarPregen(node->next);
}

void expGenerate(const AST_Node* node) {
  switch (node->operator_type) {
    case OT_UNARY_PLUS:
      fprintf(stderr, "codegen: invalid OT type in expression!\n");
      exit(99);
      break;
    case OT_UNARY_MINUS:
      fprintf(file, "SUBS\n");
      break;
    case OT_PLUS:
      fprintf(file, "ADDS\n");
      break;
    case OT_MINUS:
      fprintf(file, "SUBS\n");
      break;
    case OT_DIVIDE:
      fprintf(file, "DIVS\n");
      break;
    case OT_IDIVIDE:
      fprintf(file, "IDIVS\n");
      break;
    case OT_EQUAL:
      fprintf(file, "EQS\n");
      break;
    case OT_NOT_EQUAL:
      fprintf(file, "EQS\n");
      fprintf(file, "NOTS\n");
      break;
    case OT_LESS:
      fprintf(file, "LTS\n");
      break;
    case OT_GREATER:
      fprintf(file, "GTS\n");
      break;
    case OT_LESS_EQUAL:
      fprintf(file, "GTS\nNOTS\n");
      break;
    case OT_GREATER_EQUAL:
      fprintf(file, "LTS\nNOTS\n");
      break;
    case OT_TIMES:
      fprintf(file, "MULS\n");
      break;
    case OT_CONCAT:
      fprintf(file, "POPS GF@CONCAT$$$0\nPOPS GF@CONCAT$$$1\n");
      fprintf(file, "CONCAT GF@CONCAT$$$0 GF@CONCAT$$$1 GF@CONCAT$$$0\n");
      fprintf(file, "PUSHS GF@CONCAT$$$0\n");
      break;
    case OT_CONV:
      fprintf(file, "INT2FLOATS\n");
      break;
    case OT_PUSH:
      if(node->name) {
        fprintf(file, "PUSHS LF@%s$%d\n", node->name, node->symbol->parent->tableId);
      } else {
        switch (node->data_type) {
          case TYPE_INT:
            fprintf(file, "PUSHS int@%lld\n", node->int_value);
            break;
          case TYPE_DOUBLE:
            fprintf(file, "PUSHS float@%a\n", node->double_value);
            break;
          case TYPE_STRING:
            fprintf(file, "PUSHS string@%s\n", node->string_value);
            break;
          case TYPE_BOOL:
            fprintf(file, "PUSHS bool@%s\n", (node->bool_value ? "true" : "false"));
            break;
          default:
            break;
        }
      }
      break;
    // Boolthen TODO
    case OT_AND:
    case OT_OR:
      break;
  }
}

bool codeGenerate(FILE *output, const AST_Node *root)
{
  if (output == NULL) {
    fprintf(stderr, "codegen: output stream is required\n");
    return false;
  }
  if (root == NULL) {
    fprintf(stderr, "codegen: AST root is required\n");
    return false;
  }
  file = output;

  if (fputs(".IFJcode26\n", output) == EOF) {
    fprintf(stderr, "codegen: failed to write output: %s\n",
            errno != 0 ? strerror(errno) : "unknown output error");
    return false;
  }

  generate_node(root);
  if (ferror(output)) {
    fprintf(stderr, "codegen: failed while writing output: %s\n",
            errno != 0 ? strerror(errno) : "unknown output error");
    return false;
  }
  return true;
}
