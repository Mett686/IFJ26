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
  static bool generateDEFVAR = true;
  if (node == NULL) {
    fprintf(stderr, "codegen: encountered a null AST node\n");
    return;
  }
  Frame_t prevFrame = frame;

  switch (node->node_type) {
    case NODE_PROGRAM:
      // koren definuje zabudovane funkce, ".IFJcode26" vypisuje codeGenerate
      // jinak se chova jako def list

      // TODO

    case NODE_DEF_LIST:
      // levy je nejaky def (? konstanty promenne)
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
      int argcnt = 0;
      fprintf(file, "LABEL %s$%d\n", node->name, argcnt);
      fprintf(file, "CREATEFRAME\nPUSHFRAME\n");
      generate_node(node->left);
      generate_node(node->right);
      // return?
      fprintf(file, "POPFRAME\nRETURN\n");
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
    case NODE_STATEMENT:
      // id funkce nebo prirazeni
      // jen levy podstrom arg_list, expression
      // TODO
      break;
    case NODE_VAL_DEF:
      // id
      // levy expression
    case NODE_VAR_DEF:
      // id
      // levy expression
      if(generateDEFVAR) fprintf(file, "DEFVAR %s@%s$%d\n", FR_S(frame), node->name, 0); //scope todo
      break;
    case NODE_IF:
      // levy exp
      // pravy telo (statemet lsit)
      // next je else !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      char* exp = node->left->name;

      // vyhodnoceni podminky
      generate_node(node->left);
      fprintf(file, "PUSHS 0\n");
      // if
      fprintf(file, "JUMPIFEQS $IF$$else\n"); //TODO replace with ↓
      //fprintf(file, "JUMPIFEQS %s$IF$%d$else\n", functname, ifcnt);
      fprintf(file, "CLEARS\n");
      //telo if
      generate_node(node->right);
      fprintf(file, "JUMP $IF$$end\n"); //TODO replace with ↓
      //fprintf(file, "JUMP %s$IF$%d$end\n", functname, ifcnt);
      //telo else
      fprintf(file, "LABEL $IF$$else\n"); //TODO replace with ↓
      //fprintf(file, "LABEL %s$IF$%d$else\n", functname, ifcnt); 
      fprintf(file, "CLEARS\n");
      generate_node(node->next);
      //konec a vycisteni stacku
      fprintf(file, "LABEL $IF$$end\n"); //TODO replace with ↓
      //fprintf(file, "LABEL %s$IF$%d$end\n", functname, ifcnt);
      break;
    case NODE_ELSE:
      // telo levy podstrom (statement list) ????????? proc ne statement lsit
      break;
    case NODE_WHILE_LOOP:
      // levy exp
      // pravy telo (statemet lsit)
      
      // pregenerace promennych
      //defvarPregen(node->right);
      generateDEFVAR = false;

      fprintf(file, "LABEL $while$$start\n"); //TODO replace with ↓
      //fprintf(file, "LABEL %s$while$%d$start\n", func, whilcnt); 
      // vyhodnoceni podminky
      generate_node(node->left);
      fprintf(file, "PUSHS 0\n");
      // podmineny skok
      fprintf(file, "JUMPIFEQS $while$$end\n"); //TODO replace with ↓
      //fprintf(file, "JUMPIFEQS %s$while$%d$end\n", func, whilecnt);
      fprintf(file, "CLEARS\n");
      // telo while bez defvar
      generate_node(node->right);
      fprintf(file, "JUMP $while$$start\n"); //TODO replace with ↓
      //fprintf(file, "JUMP %s$while$%d$start\n", func, whilcnt);
      // konec
      fprintf(file, "LABEL $while$$end\n"); //TODO replace with ↓
      //fprintf(file, "LABEL %s$while$%d$end\n", func, whilecnt); 
      fprintf(file, "CLEARS\n");
      generateDEFVAR = true;

      break;
    case NODE_ARG_LIST:
      // levy exp, return ???????????????????????????
      // pravy arg_list
      generate_node(node->left);
      generate_node(node->right);
      break;
    case NODE_RETURN:
      // levy exp (musi doplnovat semAn)
      break;
    case NODE_EXP:
      // todo
      // synove val var exp
      break;
  }

  frame = prevFrame;
  return;
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
