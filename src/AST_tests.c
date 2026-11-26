/*
- ast_tests.c -- testy tvaru AST generovaneho parserom (parser.c), BEZ scanneru
 *
- AI-generated (Claude), prompt: "teraz potrebujem testy na to ci spravne generujem
- strom ... poslem ti vsetky tokeny ktore budu nato potrebne a ty z toho vytvoris testy"
 *
- Princip
- -------
-  - getToken() je nahradeny atrapou: kazdy test je pole tokenov {typ, text}.
-    Text sa ulozi do token.string (mena premennych, funkcii, hodnoty literalov).
-  - parser(NULL, table) vrati AST. Strom sa vypise do kratkeho textoveho zapisu
-    (nizsie) a porovna s ocakavanym retazcom. Pri zhode PASS, inak sa vypise
-    expected/actual.
-  - Kazdy test bezi vo vlastnom procese (fork), timeout 2 s.
 *
- Preklad a spustenie (rovnaky zoznam suborov ako parser_tests.c, bez scanner.c):
-   gcc -std=c11 -Wall -Wextra -g -fsanitize=address -I. -o test_ast ast_tests.c \
-       parser.c stack.c htab.c abtDecoding.c codegen.c
-   ASAN_OPTIONS=detect_leaks=0:exitcode=77 ./test_ast
-   ./test_ast expr_       # len testy s "expr_" v nazve
-   ./test_ast -v          # nepotlaci stderr parsera
 *
- !!! PREDPOKLAD: create_node() nuluje uzol a nastavuje data_type = TYPE_NULL !!!
-   (calloc + node->data_type = TYPE_NULL; pozor, TYPE_INT je 0). Bez toho cita
-   test neinicializovanu pamat (name, data_type) a moze padnut.
 *
- Tvar stromu (podla komentarov v ast.h)
- --------------------------------------
-  PROGRAM/DEF_LIST: left = definicia, right = dalsi DEF_LIST (koncovy prazdny uzol sa ignoruje)
-  FUNC_DEF/FUNC_DEF_MAIN: name, data_type = navratovy typ, left = retaz PARAM (cez left),
-                          right = STATEMENT_LIST
-  PARAM: name, data_type
-  STATEMENT_LIST: left = prikaz, right = dalsi STATEMENT_LIST. Ak je left uzol NODE_BODY,
-                  zoznam sa cita cez BODY (BODY.left = prvy prikaz, BODY.right = dalsi).
-  VAL_DEF/VAR_DEF: name, data_type (TYPE_NULL ak typ chyba), left = vyraz
-  ASSIGN: name, left = vyraz
-  FUNC_CALL: name, left = ARG_LIST (left = vyraz, right = dalsi ARG_LIST)
-  RETURN: left = vyraz (NULL pre samostatny return)
-  IF: left = podmienka, right = STATEMENT_LIST (then), next = ELSE (left = STATEMENT_LIST)
-  WHILE_LOOP: left = podmienka, right = STATEMENT_LIST
-  EXP: list = literal/id (name = text tokenu), inak operator_type + left (+ right)
 *
- Predpoklady, ktore ast.h nehovori (uprav testy, ak mas v kode iny zamer):
-   - vyraz je v left uzlu priamo ako koren vyrazu (bez dalsieho obalovacieho EXP),
-   - volanie funkcie na pravej strane (val/var/assign/return) je uzol FUNC_CALL v left,
-   - samostatny vyraz alebo samostatne id ako prikaz je uzol EXP v STATEMENT_LIST.left,
-   - hodnoty literalov (int_value ...) sa netestuju, len name == text tokenu,
-   - operator_type listov sa netestuje (OT_ID je v ast.h, ale parser ho nenastavuje).
 *
- Zapis stromu
- ------------
-  (program DEF...)
-  (def NAME :Typ [p:Typ ...] STMT...)   (main NAME :Typ [...] STMT...)   Typ: Int Double String Unit none
-  (val NAME :Typ EXPR)  (var ...)  (assign NAME EXPR)  (call NAME ARG...)  (return [EXPR])
-  (if COND (then STMT...) [(else STMT...)])   (while COND STMT...)   EXPR ako prikaz
-  EXPR: text listu | (+ a b) (- a b) (* a b) (/ a b) (== a b) (!= a b) (< a b) (<= a b)
-        (> a b) (>= a b) | (neg a) (pos a) | (call f ARG...)
-  "_" = chybajuci (NULL) potomok, "?" = chybajuce meno, "<node N>" = necakany typ uzla,
-  "!right=..." / "!next=..." = potomok, ktory ast.h pre dany uzol neuvadza.
 */

#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "htab.h"
#include "parser.h"
#include "scanner.h"

/* ------------------------------------------------------------------ */
/* Atrapa scanneru                                                     */
/* ------------------------------------------------------------------ */

typedef struct {
    int type;
    const char *text;
} tk_t;

static const tk_t *g_toks;
static size_t g_n, g_pos;

token_t getToken(FILE *stream)
{
    (void)stream;
    token_t t;
    memset(&t, 0, sizeof t);
    if (g_pos < g_n) {
        t.type = g_toks[g_pos].type;
        if (g_toks[g_pos].text) t.string = strdup(g_toks[g_pos].text);
        g_pos++;
    } else {
        t.type = TKN_EOF;
    }
    return t;
}

/* ------------------------------------------------------------------ */
/* Vypis stromu do textu                                               */
/* ------------------------------------------------------------------ */

static const char *type_str(Data_Type t)
{
    switch (t) {
    case TYPE_INT:    return "Int";
    case TYPE_DOUBLE: return "Double";
    case TYPE_STRING: return "String";
    case TYPE_UNIT:   return "Unit";
    case TYPE_NULL:   return "none";
    default:          return "?";
    }
}

static const char *op_str(Operator_type o)
{
    switch (o) {
    case OT_UNARY_PLUS:    return "pos";
    case OT_UNARY_MINUS:   return "neg";
    case OT_PLUS:          return "+";
    case OT_MINUS:         return "-";
    case OT_DIVIDE:        return "/";
    case OT_EQUAL:         return "==";
    case OT_NOT_EQUAL:     return "!=";
    case OT_LESS:          return "<";
    case OT_GREATER:       return ">";
    case OT_LESS_EQUAL:    return "<=";
    case OT_GREATER_EQUAL: return ">=";
    case OT_TIMES:         return "*";
    case OT_AND:           return "&&";
    case OT_OR:            return "||";
    case OT_ID:            return "id";
    default:               return "?op";
    }
}

static void dump_expr(FILE *o, const AST_Node *n);
static void dump_stmts(FILE *o, const AST_Node *list);

static void dump_call(FILE *o, const AST_Node *n)
{
    fprintf(o, "(call %s", n->name ? n->name : "?");
    for (const AST_Node *a = n->left; a; a = a->right) {
        if (a->left) {
            fputc(' ', o);
            dump_expr(o, a->left);
        }
    }
    fputc(')', o);
}

static void dump_expr(FILE *o, const AST_Node *n)
{
    if (!n) { fputs("_", o); return; }
    if (n->node_type == NODE_FUNC_CALL) { dump_call(o, n); return; }
    if (n->node_type != NODE_EXP) { fprintf(o, "<node %d>", (int)n->node_type); return; }
    if (!n->left && !n->right) { fputs(n->name ? n->name : "?", o); return; }

    int unary = n->operator_type == OT_UNARY_PLUS || n->operator_type == OT_UNARY_MINUS;
    fprintf(o, "(%s ", op_str(n->operator_type));
    dump_expr(o, n->left);
    if (!unary || n->right) {
        fputc(' ', o);
        dump_expr(o, n->right);
    }
    fputc(')', o);
}

/* potomkovia, ktorych ast.h pre dany uzol neuvadza (pomaha odhalit left/right zamenu) */
static void dump_extra(FILE *o, const AST_Node *n)
{
    if (n->right) { fputs(" !right=", o); dump_expr(o, n->right); }
    if (n->next)  { fputs(" !next=", o);  dump_expr(o, n->next); }
}

static void dump_stmt(FILE *o, const AST_Node *s)
{
    switch (s->node_type) {
    case NODE_VAL_DEF:
    case NODE_VAR_DEF:
        fprintf(o, "(%s %s :%s ", s->node_type == NODE_VAL_DEF ? "val" : "var",
                s->name ? s->name : "?", type_str(s->data_type));
        dump_expr(o, s->left);
        dump_extra(o, s);
        fputc(')', o);
        break;
    case NODE_ASSIGN:
        fprintf(o, "(assign %s ", s->name ? s->name : "?");
        dump_expr(o, s->left);
        dump_extra(o, s);
        fputc(')', o);
        break;
    case NODE_FUNC_CALL:
        dump_call(o, s);
        break;
    case NODE_RETURN:
        fputs("(return", o);
        if (s->left) {
            fputc(' ', o);
            dump_expr(o, s->left);
        }
        dump_extra(o, s);
        fputc(')', o);
        break;
    case NODE_IF:
        fputs("(if ", o);
        dump_expr(o, s->left);
        fputs(" (then", o);
        dump_stmts(o, s->right);
        fputc(')', o);
        if (s->next) {
            if (s->next->node_type == NODE_ELSE) {
                fputs(" (else", o);
                dump_stmts(o, s->next->left);
                fputc(')', o);
            } else {
                fprintf(o, " !next=<node %d>", (int)s->next->node_type);
            }
        }
        fputc(')', o);
        break;
    case NODE_WHILE_LOOP:
        fputs("(while ", o);
        dump_expr(o, s->left);
        dump_stmts(o, s->right);
        fputc(')', o);
        break;
    case NODE_EXP:
        dump_expr(o, s);
        break;
    default:
        fprintf(o, "<node %d>", (int)s->node_type);
        break;
    }
}

/* Zoznam prikazov; kazdy prikaz sa vypise s medzerou pred sebou.
- Toleruje obal NODE_BODY a koncove prazdne uzly. */
static void dump_stmts(FILE *o, const AST_Node *list)
{
    const AST_Node *n = list;
    if (n && n->node_type == NODE_STATEMENT_LIST && n->left && n->left->node_type == NODE_BODY)
        n = n->left;
    for (; n; n = n->right) {
        if ((n->node_type == NODE_STATEMENT_LIST || n->node_type == NODE_BODY) && n->left) {
            fputc(' ', o);
            dump_stmt(o, n->left);
        }
    }
}

static void dump_def(FILE *o, const AST_Node *d)
{
    if (d->node_type != NODE_FUNC_DEF && d->node_type != NODE_FUNC_DEF_MAIN) {
        fprintf(o, "<node %d>", (int)d->node_type);
        return;
    }
    fprintf(o, "(%s %s :%s [", d->node_type == NODE_FUNC_DEF_MAIN ? "main" : "def",
            d->name ? d->name : "?", type_str(d->data_type));
    int first = 1;
    for (const AST_Node *p = d->left; p; p = p->left) {
        if (!p->name) continue; /* prazdny PARAM = ziadne parametre */
        fprintf(o, "%s%s:%s", first ? "" : " ", p->name, type_str(p->data_type));
        first = 0;
    }
    fputc(']', o);
    dump_stmts(o, d->right);
    fputc(')', o);
}

static void dump_program(FILE *o, const AST_Node *ast)
{
    if (!ast) { fputs("<null>", o); return; }
    fputs("(program", o);
    for (const AST_Node *n = ast; n; n = n->right) {
        if (n->left) {
            fputc(' ', o);
            dump_def(o, n->left);
        }
    }
    fputc(')', o);
}

/* ------------------------------------------------------------------ */
/* Pomocne makra pre tokeny                                            */
/* ------------------------------------------------------------------ */

#define TK(t)     { (t), NULL }
#define T_ID(s)   { TKN_ID, (s) }
#define T_NUM(s)  { TKN_INT, (s) }
#define T_DBL(s)  { TKN_DOUBLE, (s) }
#define T_STR(s)  { TKN_STRING, (s) }

#define T_NL      TK(TKN_NEWLINE)
#define T_IND     TK(TKN_INDENTATION)
#define T_DED     TK(TKN_DEDENTATION)
#define T_LP      TK(TKN_OPENING_PARENTHESES)
#define T_RP      TK(TKN_CLOSING_PARENTHESES)
#define T_ASG     TK(TKN_ASSING)
#define T_COMMA   TK(TKN_COMMA)
#define T_COLON   TK(TKN_COLON)
#define T_SEMI    TK(TKN_SEMICOLON)
#define T_PLUS    TK(TKN_PLUS)
#define T_MINUS   TK(TKN_MINUS)
#define T_TIMES   TK(TKN_TIMES)
#define T_DIV     TK(TKN_DIVISION_SLASH)
#define T_EQ      TK(TKN_EQUAL)
#define T_NE      TK(TKN_NOT_EQUAL)
#define T_LT      TK(TKN_LESS)
#define T_LE      TK(TKN_LESS_EQUAL)
#define T_GT      TK(TKN_GREATER)
#define T_GE      TK(TKN_GREATER_EQUAL)
#define T_VAL     TK(TKN_VAL)
#define T_VAR     TK(TKN_VAR)
#define T_IF      TK(TKN_IF)
#define T_THEN    TK(TKN_THEN)
#define T_ELSE    TK(TKN_ELSE)
#define T_WHILE   TK(TKN_WHILE)
#define T_DO      TK(TKN_DO)
#define T_RETURN  TK(TKN_RETURN)
#define T_INT_KW  TK(TKN_INT_KEYWORD)
#define T_DBL_KW  TK(TKN_DOUBLE_KEYWORD)
#define T_STR_KW  TK(TKN_STRING_KEYWORD)
#define T_UNIT_KW TK(TKN_UNIT_KEYWORD)
#define T_EOF     TK(TKN_EOF)

#define PRO        TK(TKN_IMPORT), TK(TKN_IFJ26dot_asterisk), T_NL
#define MAIN_OPEN  TK(TKN_AT_MAIN), TK(TKN_DEF), T_ID("main"), T_LP, T_RP, T_COLON, \
                   T_UNIT_KW, T_ASG, T_NL, T_IND
#define MAIN_CLOSE T_DED, T_EOF
#define MAIN_PROG(...) PRO, MAIN_OPEN, __VA_ARGS__, MAIN_CLOSE

/* print(x) NEWLINE */
#define CALL1(f, a) T_ID(f), T_LP, T_ID(a), T_RP, T_NL

/* ocakavany strom programu s jedinou funkciou main, ktorej telo su prikazy b */
#define M(b) "(program (main main :Unit [] " b "))"

typedef struct {
    const char *name;
    const char *expected; /* NULL = parser ma vstup odmietnut (exit 2) */
    const tk_t *toks;
    size_t n;
} test_t;

#define CASE(nm, ex, ...) \
    { nm, ex, (const tk_t[]){__VA_ARGS__}, sizeof((const tk_t[]){__VA_ARGS__}) / sizeof(tk_t) }

/* test jedneho prikazu v tele main */
#define ST(nm, stmts, ...) CASE(nm, M(stmts), MAIN_PROG(__VA_ARGS__))

/* test vyrazu: val r = <vyraz> */
#define EXPR(nm, ex, ...) \
    CASE(nm, M("(val r :none " ex ")"), MAIN_PROG(T_VAL, T_ID("r"), T_ASG, __VA_ARGS__, T_NL))

/* ------------------------------------------------------------------ */
/* Testy                                                               */
/* ------------------------------------------------------------------ */

static const test_t tests[] = {
    /* ===================== VYRAZY (val r = ...) ===================== */
    EXPR("expr_single_id", "a", T_ID("a")),
    EXPR("expr_single_literal", "5", T_NUM("5")),
    EXPR("expr_string_literal", "hi", T_STR("hi")),
    EXPR("expr_add", "(+ a b)", T_ID("a"), T_PLUS, T_ID("b")),
    EXPR("expr_sub", "(- a b)", T_ID("a"), T_MINUS, T_ID("b")),
    EXPR("expr_mul", "(* a b)", T_ID("a"), T_TIMES, T_ID("b")),
    EXPR("expr_div", "(/ a b)", T_ID("a"), T_DIV, T_ID("b")),
    EXPR("expr_lt", "(< a b)", T_ID("a"), T_LT, T_ID("b")),
    EXPR("expr_le", "(<= a b)", T_ID("a"), T_LE, T_ID("b")),
    EXPR("expr_gt", "(> a b)", T_ID("a"), T_GT, T_ID("b")),
    EXPR("expr_ge", "(>= a b)", T_ID("a"), T_GE, T_ID("b")),
    EXPR("expr_eq", "(== a b)", T_ID("a"), T_EQ, T_ID("b")),
    EXPR("expr_ne", "(!= a b)", T_ID("a"), T_NE, T_ID("b")),
    EXPR("expr_literals_first", "(* 2 3)", T_NUM("2"), T_TIMES, T_NUM("3")),
    EXPR("expr_double_literal", "(+ 1.5 a)", T_DBL("1.5"), T_PLUS, T_ID("a")),
    EXPR("expr_id_and_literal", "(+ a 1)", T_ID("a"), T_PLUS, T_NUM("1")),

    /* priority a asociativita */
    EXPR("expr_mul_then_add", "(+ (* a b) c)",
         T_ID("a"), T_TIMES, T_ID("b"), T_PLUS, T_ID("c")),
    EXPR("expr_add_then_mul", "(+ a (* b c))",
         T_ID("a"), T_PLUS, T_ID("b"), T_TIMES, T_ID("c")),
    EXPR("expr_left_assoc_sub", "(- (- a b) c)",
         T_ID("a"), T_MINUS, T_ID("b"), T_MINUS, T_ID("c")),
    EXPR("expr_left_assoc_div", "(/ (/ a b) c)",
         T_ID("a"), T_DIV, T_ID("b"), T_DIV, T_ID("c")),
    EXPR("expr_mul_div_chain", "(- a (/ (* b c) d))",
         T_ID("a"), T_MINUS, T_ID("b"), T_TIMES, T_ID("c"), T_DIV, T_ID("d")),
    EXPR("expr_relational_of_sums", "(< (+ a b) (* c d))",
         T_ID("a"), T_PLUS, T_ID("b"), T_LT, T_ID("c"), T_TIMES, T_ID("d")),
    EXPR("expr_equality_of_sum", "(== (+ a b) c)",
         T_ID("a"), T_PLUS, T_ID("b"), T_EQ, T_ID("c")),

    /* zatvorky */
    EXPR("expr_parens", "(* (+ a b) c)",
         T_LP, T_ID("a"), T_PLUS, T_ID("b"), T_RP, T_TIMES, T_ID("c")),
    EXPR("expr_parens_right", "(* a (+ b c))",
         T_ID("a"), T_TIMES, T_LP, T_ID("b"), T_PLUS, T_ID("c"), T_RP),
    EXPR("expr_nested_parens", "a", T_LP, T_LP, T_ID("a"), T_RP, T_RP),
    EXPR("expr_parens_both_sides", "(- (+ a b) (+ c d))",
         T_LP, T_ID("a"), T_PLUS, T_ID("b"), T_RP, T_MINUS,
         T_LP, T_ID("c"), T_PLUS, T_ID("d"), T_RP),

    /* unarne operatory */
    EXPR("expr_unary_minus", "(neg a)", T_MINUS, T_ID("a")),
    EXPR("expr_unary_binds_tighter", "(* (neg a) b)", T_MINUS, T_ID("a"), T_TIMES, T_ID("b")),
    EXPR("expr_unary_paren", "(neg (+ a b))",
         T_MINUS, T_LP, T_ID("a"), T_PLUS, T_ID("b"), T_RP),
    EXPR("expr_binary_then_unary", "(- a (neg b))", T_ID("a"), T_MINUS, T_MINUS, T_ID("b")),
    EXPR("expr_double_unary", "(neg (neg a))", T_MINUS, T_MINUS, T_ID("a")),

    /* chyba: retazenie relacnych operatorov */
    CASE("expr_chained_relational_rejected", NULL,
         MAIN_PROG(T_VAL, T_ID("r"), T_ASG, T_ID("a"), T_LT, T_ID("b"), T_LT, T_ID("c"), T_NL)),

    /* ===================== PRIKAZY ===================== */
    /* val / var */
    ST("stmt_val_typed", "(val x :Int 5)",
       T_VAL, T_ID("x"), T_COLON, T_INT_KW, T_ASG, T_NUM("5"), T_NL),
    ST("stmt_val_untyped", "(val x :none 5)", T_VAL, T_ID("x"), T_ASG, T_NUM("5"), T_NL),
    ST("stmt_val_double", "(val d :Double 2.5)",
       T_VAL, T_ID("d"), T_COLON, T_DBL_KW, T_ASG, T_DBL("2.5"), T_NL),
    ST("stmt_val_typed_expression", "(val x :Int (+ a 1))",
       T_VAL, T_ID("x"), T_COLON, T_INT_KW, T_ASG, T_ID("a"), T_PLUS, T_NUM("1"), T_NL),
    ST("stmt_var_untyped", "(var x :none 1)", T_VAR, T_ID("x"), T_ASG, T_NUM("1"), T_NL),
    ST("stmt_var_string", "(var s :String hi)",
       T_VAR, T_ID("s"), T_COLON, T_STR_KW, T_ASG, T_STR("hi"), T_NL),

    /* priradenie */
    ST("stmt_assign_literal", "(var x :none 1) (assign x 5)",
       T_VAR, T_ID("x"), T_ASG, T_NUM("1"), T_NL,
       T_ID("x"), T_ASG, T_NUM("5"), T_NL),
    ST("stmt_assign_id", "(var x :none 1) (assign x y)",
       T_VAR, T_ID("x"), T_ASG, T_NUM("1"), T_NL,
       T_ID("x"), T_ASG, T_ID("y"), T_NL),
    ST("stmt_assign_expression", "(var x :none 1) (assign x (+ x 1))",
       T_VAR, T_ID("x"), T_ASG, T_NUM("1"), T_NL,
       T_ID("x"), T_ASG, T_ID("x"), T_PLUS, T_NUM("1"), T_NL),
    ST("stmt_assign_unary", "(var x :none 1) (assign x (neg y))",
       T_VAR, T_ID("x"), T_ASG, T_NUM("1"), T_NL,
       T_ID("x"), T_ASG, T_MINUS, T_ID("y"), T_NL),
    ST("stmt_assign_paren", "(var x :none 1) (assign x (* (+ a b) c))",
       T_VAR, T_ID("x"), T_ASG, T_NUM("1"), T_NL,
       T_ID("x"), T_ASG, T_LP, T_ID("a"), T_PLUS, T_ID("b"), T_RP, T_TIMES, T_ID("c"), T_NL),

    /* volanie funkcie ako prikaz */
    ST("stmt_call_noargs", "(call f)", T_ID("f"), T_LP, T_RP, T_NL),
    ST("stmt_call_one_arg", "(call print x)", CALL1("print", "x")),
    ST("stmt_call_three_args", "(call f a 1 b)",
       T_ID("f"), T_LP, T_ID("a"), T_COMMA, T_NUM("1"), T_COMMA, T_ID("b"), T_RP, T_NL),
    ST("stmt_call_string_arg", "(call print hello)",
       T_ID("print"), T_LP, T_STR("hello"), T_RP, T_NL),
    /* argumenty ako vyrazy (rozsirenie FUNEXP) */
    ST("stmt_call_expr_args", "(call f (+ a 1) b)",
       T_ID("f"), T_LP, T_ID("a"), T_PLUS, T_NUM("1"), T_COMMA, T_ID("b"), T_RP, T_NL),

    /* volanie funkcie na pravej strane */
    ST("stmt_val_call", "(val r :none (call f x))",
       T_VAL, T_ID("r"), T_ASG, T_ID("f"), T_LP, T_ID("x"), T_RP, T_NL),
    ST("stmt_val_call_noargs", "(val r :none (call f))",
       T_VAL, T_ID("r"), T_ASG, T_ID("f"), T_LP, T_RP, T_NL),
    ST("stmt_assign_call", "(var r :none 0) (assign r (call f y))",
       T_VAR, T_ID("r"), T_ASG, T_NUM("0"), T_NL,
       T_ID("r"), T_ASG, T_ID("f"), T_LP, T_ID("y"), T_RP, T_NL),

    /* return */
    ST("stmt_return_expr", "(return (+ a 1))", T_RETURN, T_ID("a"), T_PLUS, T_NUM("1"), T_NL),
    ST("stmt_return_id", "(return a)", T_RETURN, T_ID("a"), T_NL),
    ST("stmt_return_literal", "(return 5)", T_RETURN, T_NUM("5"), T_NL),
    ST("stmt_return_bare", "(return)", T_RETURN, T_NL),
    ST("stmt_return_call", "(return (call f x))",
       T_RETURN, T_ID("f"), T_LP, T_ID("x"), T_RP, T_NL),

    /* samostatny vyraz ako prikaz */
    ST("stmt_expr_id", "a", T_ID("a"), T_NL),
    ST("stmt_expr_binary", "(+ a 1)", T_ID("a"), T_PLUS, T_NUM("1"), T_NL),
    ST("stmt_expr_paren", "(+ a 1)", T_LP, T_ID("a"), T_PLUS, T_NUM("1"), T_RP, T_NL),

    /* viac prikazov za sebou */
    ST("stmt_semicolon", "(val a :none 1) (val b :none 2)",
       T_VAL, T_ID("a"), T_ASG, T_NUM("1"), T_SEMI,
       T_VAL, T_ID("b"), T_ASG, T_NUM("2"), T_NL),
    ST("stmt_order_preserved", "(val a :none 1) (val b :none 2) (call print a)",
       T_VAL, T_ID("a"), T_ASG, T_NUM("1"), T_NL,
       T_VAL, T_ID("b"), T_ASG, T_NUM("2"), T_NL,
       CALL1("print", "a")),

    /* ===================== if / while ===================== */
    ST("if_else_calls", "(if (< a 1) (then (call print x)) (else (call print y)))",
       T_IF, T_ID("a"), T_LT, T_NUM("1"), T_THEN, T_NL, T_IND, CALL1("print", "x"), T_DED,
       T_ELSE, T_NL, T_IND, CALL1("print", "y"), T_DED),
    ST("if_condition_expression",
       "(if (== (+ a b) c) (then (call print x)) (else (call print y)))",
       T_IF, T_ID("a"), T_PLUS, T_ID("b"), T_EQ, T_ID("c"), T_THEN, T_NL, T_IND,
       CALL1("print", "x"), T_DED,
       T_ELSE, T_NL, T_IND, CALL1("print", "y"), T_DED),
    ST("if_multiple_statements",
       "(if (> a 0) (then (val b :none 1) (call print b)) (else (call print a)))",
       T_IF, T_ID("a"), T_GT, T_NUM("0"), T_THEN, T_NL, T_IND,
       T_VAL, T_ID("b"), T_ASG, T_NUM("1"), T_NL, CALL1("print", "b"), T_DED,
       T_ELSE, T_NL, T_IND, CALL1("print", "a"), T_DED),
    ST("if_followed_by_statement",
       "(if (< a 1) (then (call print x)) (else (call print y))) (call print z)",
       T_IF, T_ID("a"), T_LT, T_NUM("1"), T_THEN, T_NL, T_IND, CALL1("print", "x"), T_DED,
       T_ELSE, T_NL, T_IND, CALL1("print", "y"), T_DED,
       CALL1("print", "z")),
    ST("if_returns", "(if (< a 1) (then (return 1)) (else (return 2)))",
       T_IF, T_ID("a"), T_LT, T_NUM("1"), T_THEN, T_NL, T_IND, T_RETURN, T_NUM("1"), T_NL, T_DED,
       T_ELSE, T_NL, T_IND, T_RETURN, T_NUM("2"), T_NL, T_DED),
    ST("if_nested",
       "(if (< a 1) (then (if (< b 2) (then (call print x)) (else (call print y)))) "
       "(else (call print z)))",
       T_IF, T_ID("a"), T_LT, T_NUM("1"), T_THEN, T_NL, T_IND,
       T_IF, T_ID("b"), T_LT, T_NUM("2"), T_THEN, T_NL, T_IND, CALL1("print", "x"), T_DED,
       T_ELSE, T_NL, T_IND, CALL1("print", "y"), T_DED,
       T_DED,
       T_ELSE, T_NL, T_IND, CALL1("print", "z"), T_DED),
    ST("while_basic", "(while (> a 0) (assign a (- a 1)))",
       T_WHILE, T_ID("a"), T_GT, T_NUM("0"), T_DO, T_NL, T_IND,
       T_ID("a"), T_ASG, T_ID("a"), T_MINUS, T_NUM("1"), T_NL, T_DED),
    ST("while_two_statements_then_next",
       "(while (> a 0) (call print a) (assign a (- a 1))) (call print a)",
       T_WHILE, T_ID("a"), T_GT, T_NUM("0"), T_DO, T_NL, T_IND,
       CALL1("print", "a"),
       T_ID("a"), T_ASG, T_ID("a"), T_MINUS, T_NUM("1"), T_NL, T_DED,
       CALL1("print", "a")),
    ST("while_with_if",
       "(while (< i 3) (if (== i 1) (then (call print i)) (else (call print i))))",
       T_WHILE, T_ID("i"), T_LT, T_NUM("3"), T_DO, T_NL, T_IND,
       T_IF, T_ID("i"), T_EQ, T_NUM("1"), T_THEN, T_NL, T_IND, CALL1("print", "i"), T_DED,
       T_ELSE, T_NL, T_IND, CALL1("print", "i"), T_DED,
       T_DED),

    /* ===================== FUNKCIE A PROGRAM ===================== */
    CASE("prog_only_prolog", "(program)", PRO, T_EOF),

    CASE("def_params_one_line_body",
         "(program (def f :Int [a:Int b:Double] a) (main main :Unit [] (call print x)))",
         PRO,
         TK(TKN_DEF), T_ID("f"), T_LP, T_ID("a"), T_COLON, T_INT_KW, T_COMMA,
         T_ID("b"), T_COLON, T_DBL_KW, T_RP, T_COLON, T_INT_KW, T_ASG, T_ID("a"), T_NL,
         MAIN_OPEN, CALL1("print", "x"), MAIN_CLOSE),
    CASE("def_no_params_no_type",
         "(program (def g :none [] 1) (main main :Unit [] (call print x)))",
         PRO,
         TK(TKN_DEF), T_ID("g"), T_LP, T_RP, T_ASG, T_NUM("1"), T_NL,
         MAIN_OPEN, CALL1("print", "x"), MAIN_CLOSE),
    CASE("def_no_return_type_expression",
         "(program (def inc :none [a:Int] (+ a 1)) (main main :Unit [] (call print x)))",
         PRO,
         TK(TKN_DEF), T_ID("inc"), T_LP, T_ID("a"), T_COLON, T_INT_KW, T_RP, T_ASG,
         T_ID("a"), T_PLUS, T_NUM("1"), T_NL,
         MAIN_OPEN, CALL1("print", "x"), MAIN_CLOSE),
    CASE("def_three_string_params",
         "(program (def s :String [a:String b:String c:String] a) "
         "(main main :Unit [] (call print x)))",
         PRO,
         TK(TKN_DEF), T_ID("s"), T_LP,
         T_ID("a"), T_COLON, T_STR_KW, T_COMMA,
         T_ID("b"), T_COLON, T_STR_KW, T_COMMA,
         T_ID("c"), T_COLON, T_STR_KW, T_RP, T_COLON, T_STR_KW, T_ASG, T_ID("a"), T_NL,
         MAIN_OPEN, CALL1("print", "x"), MAIN_CLOSE),
    CASE("def_block_body_return",
         "(program (def h :Int [a:Int] (return a)) (main main :Unit [] (call print x)))",
         PRO,
         TK(TKN_DEF), T_ID("h"), T_LP, T_ID("a"), T_COLON, T_INT_KW, T_RP, T_COLON, T_INT_KW,
         T_ASG, T_NL, T_IND, T_RETURN, T_ID("a"), T_NL, T_DED,
         MAIN_OPEN, CALL1("print", "x"), MAIN_CLOSE),
    CASE("def_block_body_two_statements",
         "(program (def p :Unit [] (val a :none 1) (call print a)) "
         "(main main :Unit [] (call print x)))",
         PRO,
         TK(TKN_DEF), T_ID("p"), T_LP, T_RP, T_COLON, T_UNIT_KW, T_ASG, T_NL, T_IND,
         T_VAL, T_ID("a"), T_ASG, T_NUM("1"), T_NL, CALL1("print", "a"), T_DED,
         MAIN_OPEN, CALL1("print", "x"), MAIN_CLOSE),
    CASE("main_annotation_own_line_and_name",
         "(program (main run :Unit [] (call print x)))",
         PRO,
         TK(TKN_AT_MAIN), T_NL, TK(TKN_DEF), T_ID("run"), T_LP, T_RP, T_COLON, T_UNIT_KW,
         T_ASG, T_NL, T_IND, CALL1("print", "x"), T_DED, T_EOF),
    CASE("main_first_then_function",
         "(program (main main :Unit [] (call print x)) (def g :none [] 1))",
         PRO, MAIN_OPEN, CALL1("print", "x"), T_DED,
         TK(TKN_DEF), T_ID("g"), T_LP, T_RP, T_ASG, T_NUM("1"), T_NL, T_EOF),
    CASE("three_functions_order_preserved",
         "(program (def f :none [] 1) (def g :none [] 2) (main main :Unit [] (call print x)))",
         PRO,
         TK(TKN_DEF), T_ID("f"), T_LP, T_RP, T_ASG, T_NUM("1"), T_NL,
         TK(TKN_DEF), T_ID("g"), T_LP, T_RP, T_ASG, T_NUM("2"), T_NL,
         MAIN_OPEN, CALL1("print", "x"), MAIN_CLOSE),
};

/* ------------------------------------------------------------------ */
/* Spustanie                                                           */
/* ------------------------------------------------------------------ */

/* exit: 0 = zhoda, 1 = strom sa lisi, 3 = mal byt odmietnuty, ale prijaty */
static int run_child(const test_t *t)
{
    g_toks = t->toks;
    g_n = t->n;
    g_pos = 0;
    alarm(2);

    htab_t *table = htabCreate(16);
    AST_Node *ast = parser(NULL, table);

    if (t->expected == NULL) return 3;

    char *buf = NULL;
    size_t len = 0;
    FILE *o = open_memstream(&buf, &len);
    if (!o) return 99;
    dump_program(o, ast);
    fclose(o);

    if (strcmp(buf, t->expected) == 0) return 0;
    printf("      expected: %s\n      actual:   %s\n", t->expected, buf);
    fflush(stdout);
    return 1;
}

int main(int argc, char **argv)
{
    const char *filter = NULL;
    int verbose = 0, pass = 0, fail = 0;
    size_t count = sizeof tests / sizeof tests[0];

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) verbose = 1;
        else filter = argv[i];
    }

    for (size_t i = 0; i < count; i++) {
        const test_t *t = &tests[i];
        if (filter && !strstr(t->name, filter)) continue;

        int fd[2];
        if (pipe(fd) != 0) { perror("pipe"); return 99; }
        fflush(stdout);
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); return 99; }
        if (pid == 0) {
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            if (!verbose) freopen("/dev/null", "w", stderr);
            _exit(run_child(t));
        }
        close(fd[1]);
        int status = 0;
        waitpid(pid, &status, 0);

        char out[4096];
        size_t used = 0;
        ssize_t r;
        while (used < sizeof out - 1 && (r = read(fd[0], out + used, sizeof out - 1 - used)) > 0)
            used += (size_t)r;
        out[used] = '\0';
        close(fd[0]);

        const char *why = NULL;
        char tmp[64];
        if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            if (sig == SIGALRM) why = "zacykleni (timeout 2 s)";
            else { snprintf(tmp, sizeof tmp, "pad, signal %d", sig); why = tmp; }
        } else {
            int code = WEXITSTATUS(status);
            if (code == 0) why = NULL;
            else if (code == 1) why = "strom sa nezhoduje";
            else if (code == 2) why = t->expected ? "parser odmietol vstup (exit 2)" : NULL;
            else if (code == 3) why = "neplatny vstup bol prijaty";
            else if (code == 77) why = "chyba AddressSanitizer (spusti s -v)";
            else { snprintf(tmp, sizeof tmp, "neocekavany exit kod %d", code); why = tmp; }
        }

        if (why == NULL) {
            pass++;
            printf("PASS  %s\n", t->name);
        } else {
            fail++;
            printf("FAIL  %-36s %s\n", t->name, why);
            if (used > 0) fputs(out, stdout);
        }
    }

    printf("\nVysledek: %d proslo, %d selhalo\n", pass, fail);
    return fail ? 1 : 0;
}