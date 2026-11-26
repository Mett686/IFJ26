// codegen.c
// Autor: Martin Knotek, FIT
// Autor: Filip Maxa, FIT


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "codegen.h"
#include "stdFunc.h"

#define FR_S(frame) \
  (frame == GF ? "GF" : (frame == TF ? "TF" : "LF"))

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

  switch (node->node_type) { // TODO assign, statement, program, cteni z tabulky
    case NODE_PROGRAM:
      // koren definuje zabudovane funkce, ".IFJcode26" vypisuje codeGenerate
      // jinak se chova jako def list

      fprintf(file, STD_FUNC_STR); // tisk standartnich funkci

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
      int argcnt = 0;
      fprintf(file, "LABEL %s$%d\n", funcName, argcnt);
      fprintf(file, "CREATEFRAME\nPUSHFRAME\n");
      generate_node(node->left);
      generate_node(node->right);
      break;
    case NODE_PARAM:
      // parametr
      // levy dalsi param
      fprintf(file, "DEFVAR LF@%s", node->name);
      fprintf(file, "POPS LF@%s", node->name);
      generate_node(node->left);
      break;
    case NODE_STATEMENT_LIST:
      // levy podstrom statement, val, var, if, while, exp, return
      // pravy statement_list
      generate_node(node->left);
      generate_node(node->right);
      break;
    case NODE_STATEMENT: // TODO nerikali jsme ze bude rovnou arg_list?
      // id funkce nebo prirazeni
      // jen levy podstrom arg_list, expression
      // TODO
      break;
    case NODE_RETURN:
      // levy exp (musi doplnovat semAn)
      // TODO je to spravne?
      generate_node(node->left);
      fprintf(file, "POPFRAME\nRETURN\n");
      break;
    case NODE_VAL_DEF: // todo zapis do tabulky
      // id
      // levy expression
      if(!cycleDepth) fprintf(file, "DEFVAR %s@%s$VAL$%d\n", FR_S(frame), node->name, 0); //scope todo
      break;
    case NODE_VAR_DEF:
      // id
      // levy expression
      if(!cycleDepth) fprintf(file, "DEFVAR %s@%s$VAR$%d\n", FR_S(frame), node->name, 0); //scope todo
      break;
    case NODE_IF:
      // levy exp
      // pravy telo (statemet lsit)
      // next je else

      // vyhodnoceni podminky
      scopecnt++;
      generate_node(node->left); // melo by na stack hodit navratovou vec
      fprintf(file, "PUSHS 0\n");
      // if
      unsigned myIfCnt = ifcnt;
      ifcnt++;
      fprintf(file, "JUMPIFEQS %s$IF$%d$else\n", funcName, myIfCnt);
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
      cycleDepth++;
      scopecnt++;
      if(!cycleDepth) defvarPregen(node->right, scopecnt);

      fprintf(file, "LABEL %s$while$%d$start\n", funcName, myWhileCnt); 
      // vyhodnoceni podminky
      generate_node(node->left);
      fprintf(file, "PUSHS 0\n");
      scopecnt++;
      // podmineny skok
      fprintf(file, "JUMPIFEQS %s$while$%d$end\n", funcName, myWhileCnt);
      fprintf(file, "CLEARS\n");
      // telo while bez defvar
      generate_node(node->right);
      fprintf(file, "JUMP %s$while$%d$start\n", funcName, myWhileCnt);
      // konec
      fprintf(file, "LABEL %s$while$%d$end\n", funcName, myWhileCnt); 
      fprintf(file, "CLEARS\n");
      cycleDepth--;
      scopecnt++;

      break;
    case NODE_ARG_LIST:
      // levy exp
      // pravy arg_list
      generate_node(node->left);
      generate_node(node->right);
      break;
    case NODE_EXP:
      // todo
      // synove val var exp
      generate_node(node->left);
      generate_node(node->right);
      expGenerate(node);
      break;
  }

  frame = prevFrame;
  return;
}

unsigned defvarPregen(AST_Node* node, unsigned scopecnt) {
  if(!node) return scopecnt;
  switch (node->node_type)
  {
  case NODE_VAL_DEF:
    fprintf(file, "DEFVAR LF@%s$VAL$%d\n", node->name, scopecnt);
    break;
  case NODE_VAR_DEF:
    fprintf(file, "DEFVAR LF@%s$VAR$%d\n", node->name, scopecnt);
    break;
  case NODE_FUNC_DEF:
  case NODE_FUNC_DEF_MAIN:
  case NODE_IF:
  case NODE_ELSE:
  case NODE_WHILE_LOOP:
    scopecnt++;
  default:
    break;
  }
  
  scopecnt = defvarPregen(node->left, scopecnt);
  scopecnt = defvarPregen(node->right, scopecnt);
  scopecnt = defvarPregen(node->next, scopecnt);
  return scopecnt;
}

void expGenerate(AST_Node* node) {
  switch (node->operator_type) { // TODO unarni, DIV vs IDIV, pretypovani, nacteni promene na stack
    case OT_PLUS:
      fprintf(file, "ADDS\n");
      break;
    case OT_MINUS:
      fprintf(file, "SUBS\n");
      break;
    case OT_DIVIDE:
      fprintf(file, "DIVS\n");
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
