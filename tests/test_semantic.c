#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/htab.h"
#include "../src/abtDecoding.h"

#define HTAB_TEST_BUCKETS 503

// --- Test Helpers ---

static AST_Node* make_node(Node_type type, Data_Type dtype, const char* name,
                           Operator_type op, AST_Node* left, AST_Node* right, AST_Node* next) {
    AST_Node* n = calloc(1, sizeof(AST_Node));
    assert(n != NULL);
    n->node_type = type;
    n->data_type = dtype;
    n->operator_type = op;
    n->left = left;
    n->right = right;
    n->next = next;
    if (name != NULL) {
        n->name = strdup(name);
        assert(n->name != NULL);
    }
    return n;
}

static AST_Node* make_leaf_exp(Data_Type dtype, const char* var_name) {
    return make_node(NODE_EXP, dtype, var_name, OT_PLUS, NULL, NULL, NULL);
}

static AST_Node* make_bin_exp(Operator_type op, AST_Node* left, AST_Node* right) {
    return make_node(NODE_EXP, TYPE_NULL, NULL, op, left, right, NULL);
}

static AST_Node* make_param(const char* name, Data_Type dtype, AST_Node* next_param) {
    return make_node(NODE_PARAM, dtype, name, OT_PLUS, next_param, NULL, NULL);
}

static AST_Node* make_arg_node(AST_Node* expr, AST_Node* next_arg) {
    return make_node(NODE_ARG_LIST, TYPE_NULL, NULL, OT_PLUS, expr, next_arg, NULL);
}

static AST_Node* make_func_def(const char* name, Data_Type ret_type, AST_Node* params, AST_Node* body) {
    return make_node(NODE_FUNC_DEF, ret_type, name, OT_PLUS, params, body, NULL);
}

static AST_Node* make_main_def(const char* name, Data_Type ret_type, AST_Node* params, AST_Node* body) {
    return make_node(NODE_FUNC_DEF_MAIN, ret_type, name, OT_PLUS, params, body, NULL);
}

// Wraps one or more top-level function definitions into NODE_PROGRAM -> NODE_DEF_LIST chain
static AST_Node* make_program(AST_Node* funcs[], int func_count) {
    AST_Node* prog = make_node(NODE_PROGRAM, TYPE_NULL, NULL, OT_PLUS, NULL, NULL, NULL);
    AST_Node* curr = prog;
    for (int i = 0; i < func_count; i++) {
        curr->left = funcs[i];
        curr->right = make_node(NODE_DEF_LIST, TYPE_NULL, NULL, OT_PLUS, NULL, NULL, NULL);
        curr = curr->right;
    }
    return prog;
}

// Wraps statements into a NODE_STATEMENT_LIST chain
static AST_Node* make_stmt_list(AST_Node* stmts[], int stmt_count) {
    if (stmt_count <= 0) {
        return make_node(NODE_STATEMENT_LIST, TYPE_NULL, NULL, OT_PLUS, NULL, NULL, NULL);
    }
    AST_Node* head = make_node(NODE_STATEMENT_LIST, TYPE_NULL, NULL, OT_PLUS, NULL, NULL, NULL);
    AST_Node* curr = head;
    for (int i = 0; i < stmt_count; i++) {
        curr->left = stmts[i];
        curr->right = make_node(NODE_STATEMENT_LIST, TYPE_NULL, NULL, OT_PLUS, NULL, NULL, NULL);
        curr = curr->right;
    }
    return head;
}

// Verifies that an AST pointer was wrapped in a conversion node of expected_type
static void assert_wrapped_with(AST_Node* wrapper, AST_Node* original_child,
                              Node_type expected_node_type, Data_Type expected_child_dtype) {
    assert(wrapper != NULL);
    assert(wrapper != original_child);
    assert(wrapper->node_type == expected_node_type);
    assert(wrapper->left == original_child);
    assert(wrapper->right == NULL);
    assert(wrapper->next == NULL);
    assert(wrapper->data_type == expected_child_dtype);
}

static void run_expect_exit(const char* test_name, int expected_code, void (*fn)(void)) {
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        freopen("/dev/null", "w", stderr);
        fn();
        _exit(0);
    } else {
        int status = 0;
        assert(waitpid(pid, &status, 0) == pid);
        assert(WIFEXITED(status));
        int actual_code = WEXITSTATUS(status);
        if (actual_code != expected_code) {
            fprintf(stderr, "[FAIL] %s: expected exit code %d, got %d\n",
                    test_name, expected_code, actual_code);
            exit(1);
        }
        printf("  PASS: %s (exit %d)\n", test_name, actual_code);
    }
}

static void run_expect_pass(const char* test_name, void (*fn)(void)) {
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        fn();
        _exit(0);
    } else {
        int status = 0;
        assert(waitpid(pid, &status, 0) == pid);
        assert(WIFEXITED(status));
        int code = WEXITSTATUS(status);
        if (code != 0) {
            fprintf(stderr, "[FAIL] %s: expected clean pass (exit 0), got exit %d\n",
                    test_name, code);
            exit(1);
        }
        printf("  PASS: %s\n", test_name);
    }
}

// --- 1. @main & Program Structure Tests (1-5) ---

static void body_main_ok(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_01_valid_main_unit(void) { run_expect_pass("01_valid_main_unit", body_main_ok); }

static void body_missing_main(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* f = make_func_def("foo", TYPE_INT, NULL, NULL);
    AST_Node* funcs[] = { f };
    semanticCheck(make_program(funcs, 1), t);
}
void test_02_missing_main_exit_3(void) { run_expect_exit("02_missing_main_exit_3", 3, body_missing_main); }

static void body_duplicate_main(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* m1 = make_main_def("main1", TYPE_UNIT, NULL, NULL);
    AST_Node* m2 = make_main_def("main2", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { m1, m2 };
    semanticCheck(make_program(funcs, 2), t);
}
void test_03_duplicate_main_exit_4(void) { run_expect_exit("03_duplicate_main_exit_4", 4, body_duplicate_main); }

static void body_main_with_param(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* p = make_param("x", TYPE_INT, NULL);
    AST_Node* m = make_main_def("main", TYPE_UNIT, p, NULL);
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_04_main_with_param_exit_5(void) { run_expect_exit("04_main_with_param_exit_5", 5, body_main_with_param); }

static void body_main_non_unit(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* m = make_main_def("main", TYPE_INT, NULL, NULL);
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_05_main_non_unit_exit_6(void) { run_expect_exit("05_main_non_unit_exit_6", 6, body_main_non_unit); }

// --- 2. Builtin Functions Tests (6-12) ---

static void body_builtin_print_any(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* orig_arg = make_leaf_exp(TYPE_STRING, NULL);
    AST_Node* arg = make_arg_node(orig_arg, NULL);
    AST_Node* call = make_node(NODE_FUNC_CALL, TYPE_NULL, "print", OT_PLUS, arg, NULL, NULL);
    AST_Node* stmts[] = { call };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(call->data_type == TYPE_UNIT);
    assert(arg->left == orig_arg); // TYPE_ANY must NOT wrap argument
}
void test_06_builtin_print_string_wrapper(void) { run_expect_pass("06_builtin_print_string", body_builtin_print_any); }

static void body_builtin_toint_from_int_promoted(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* orig_arg = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* arg = make_arg_node(orig_arg, NULL);
    AST_Node* call = make_node(NODE_FUNC_CALL, TYPE_NULL, "toInt", OT_PLUS, arg, NULL, NULL);
    AST_Node* stmts[] = { call };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(call->data_type == TYPE_INT);
    assert_wrapped_with(arg->left, orig_arg, NODE_EXP, TYPE_INT);
}
void test_07_builtin_toint_int_arg(void) { run_expect_pass("07_builtin_toint_int_arg", body_builtin_toint_from_int_promoted); }

static void body_builtin_toint_wrong_type(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* arg = make_arg_node(make_leaf_exp(TYPE_STRING, NULL), NULL);
    AST_Node* call = make_node(NODE_FUNC_CALL, TYPE_NULL, "toInt", OT_PLUS, arg, NULL, NULL);
    AST_Node* stmts[] = { call };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_08_builtin_toint_string_exit_5(void) { run_expect_exit("08_builtin_toint_string_exit_5", 5, body_builtin_toint_wrong_type); }

static void body_builtin_substr_ok(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* a3 = make_arg_node(make_leaf_exp(TYPE_INT, NULL), NULL);
    AST_Node* a2 = make_arg_node(make_leaf_exp(TYPE_INT, NULL), a3);
    AST_Node* a1 = make_arg_node(make_leaf_exp(TYPE_STRING, NULL), a2);
    AST_Node* call = make_node(NODE_FUNC_CALL, TYPE_NULL, "substr", OT_PLUS, a1, NULL, NULL);
    AST_Node* stmts[] = { call };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(call->data_type == TYPE_STRING);
}
void test_09_builtin_substr_ok(void) { run_expect_pass("09_builtin_substr_ok", body_builtin_substr_ok); }

static void body_builtin_too_few_args(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* a1 = make_arg_node(make_leaf_exp(TYPE_STRING, NULL), NULL);
    AST_Node* call = make_node(NODE_FUNC_CALL, TYPE_NULL, "ord", OT_PLUS, a1, NULL, NULL);
    AST_Node* stmts[] = { call };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_10_builtin_too_few_args_exit_5(void) { run_expect_exit("10_builtin_too_few_args_exit_5", 5, body_builtin_too_few_args); }

static void body_builtin_too_many_args(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* a2 = make_arg_node(make_leaf_exp(TYPE_INT, NULL), NULL);
    AST_Node* a1 = make_arg_node(make_leaf_exp(TYPE_INT, NULL), a2);
    AST_Node* call = make_node(NODE_FUNC_CALL, TYPE_NULL, "chr", OT_PLUS, a1, NULL, NULL);
    AST_Node* stmts[] = { call };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_11_builtin_too_many_args_exit_5(void) { run_expect_exit("11_builtin_too_many_args_exit_5", 5, body_builtin_too_many_args); }

static void body_undefined_func_call(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* call = make_node(NODE_FUNC_CALL, TYPE_NULL, "noSuchFunc", OT_PLUS, NULL, NULL, NULL);
    AST_Node* stmts[] = { call };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_12_undefined_func_exit_3(void) { run_expect_exit("12_undefined_func_exit_3", 3, body_undefined_func_call); }

// --- 3. User Functions & Calls Tests (13-17) ---

static void body_duplicate_func_def(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* f1 = make_func_def("helper", TYPE_INT, NULL, NULL);
    AST_Node* f2 = make_func_def("helper", TYPE_INT, NULL, NULL);
    AST_Node* m  = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f1, f2, m };
    semanticCheck(make_program(funcs, 3), t);
}
void test_13_duplicate_func_exit_4(void) { run_expect_exit("13_duplicate_func_exit_4", 4, body_duplicate_func_def); }

static void body_duplicate_builtin_name(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* f1 = make_func_def("print", TYPE_UNIT, NULL, NULL);
    AST_Node* m  = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f1, m };
    semanticCheck(make_program(funcs, 2), t);
}
void test_14_redefine_builtin_exit_4(void) { run_expect_exit("14_redefine_builtin_exit_4", 4, body_duplicate_builtin_name); }

static void body_forward_func_call_ok(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* a1 = make_arg_node(make_leaf_exp(TYPE_INT, NULL), NULL);
    AST_Node* call = make_node(NODE_FUNC_CALL, TYPE_NULL, "laterFunc", OT_PLUS, a1, NULL, NULL);
    AST_Node* stmts[] = { call };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* p = make_param("x", TYPE_INT, NULL);
    AST_Node* later = make_func_def("laterFunc", TYPE_STRING, p, NULL);
    AST_Node* funcs[] = { m, later };
    semanticCheck(make_program(funcs, 2), t);
    assert(call->data_type == TYPE_STRING);
}
void test_15_forward_func_call_ok(void) { run_expect_pass("15_forward_func_call_ok", body_forward_func_call_ok); }

static void body_user_func_arg_mismatch(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* p = make_param("x", TYPE_STRING, NULL);
    AST_Node* f = make_func_def("takeStr", TYPE_UNIT, p, NULL);
    AST_Node* a1 = make_arg_node(make_leaf_exp(TYPE_INT, NULL), NULL);
    AST_Node* call = make_node(NODE_FUNC_CALL, TYPE_NULL, "takeStr", OT_PLUS, a1, NULL, NULL);
    AST_Node* stmts[] = { call };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);
}
void test_16_user_func_arg_mismatch_exit_5(void) { run_expect_exit("16_user_func_arg_mismatch_exit_5", 5, body_user_func_arg_mismatch); }

static void body_duplicate_param_name(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* p2 = make_param("a", TYPE_INT, NULL);
    AST_Node* p1 = make_param("a", TYPE_INT, p2);
    AST_Node* f  = make_func_def("dupParam", TYPE_UNIT, p1, NULL);
    AST_Node* m  = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);
}
void test_17_duplicate_param_exit_4(void) { run_expect_exit("17_duplicate_param_exit_4", 4, body_duplicate_param_name); }

// --- 4. Variable & Value Definitions + Scoping (18-26) ---

static void body_val_infer_int(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* orig = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* v = make_node(NODE_VAL_DEF, TYPE_NULL, "x", OT_PLUS, orig, NULL, NULL);
    AST_Node* stmts[] = { v };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(v->data_type == TYPE_INT);
    assert(v->left == orig); // No conversion needed when inferred
}
void test_18_val_infer_int(void) { run_expect_pass("18_val_infer_int", body_val_infer_int); }

static void body_var_int_to_double_init(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* orig = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* v = make_node(NODE_VAR_DEF, TYPE_DOUBLE, "d", OT_PLUS, orig, NULL, NULL);
    AST_Node* stmts[] = { v };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(v->data_type == TYPE_DOUBLE);
    assert_wrapped_with(v->left, orig, NODE_EXP, TYPE_INT);
}
void test_19_var_int_to_double_init(void) { run_expect_pass("19_var_int_to_double_init", body_var_int_to_double_init); }

static void body_var_init_mismatch(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v = make_node(NODE_VAR_DEF, TYPE_INT, "n", OT_PLUS, make_leaf_exp(TYPE_STRING, NULL), NULL, NULL);
    AST_Node* stmts[] = { v };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_20_var_init_mismatch_exit_6(void) { run_expect_exit("20_var_init_mismatch_exit_6", 6, body_var_init_mismatch); }

static void body_var_null_init_type(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v = make_node(NODE_VAR_DEF, TYPE_NULL, "bad", OT_PLUS, make_leaf_exp(TYPE_NULL, NULL), NULL, NULL);
    AST_Node* stmts[] = { v };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_21_var_null_init_exit_6(void) { run_expect_exit("21_var_null_init_exit_6", 6, body_var_null_init_type); }

static void body_redef_same_scope(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v1 = make_node(NODE_VAR_DEF, TYPE_NULL, "x", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* v2 = make_node(NODE_VAR_DEF, TYPE_NULL, "x", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* stmts[] = { v1, v2 };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 2));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_22_redef_same_scope_exit_4(void) { run_expect_exit("22_redef_same_scope_exit_4", 4, body_redef_same_scope); }

static void body_shadow_in_inner_scope_ok(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v_outer = make_node(NODE_VAR_DEF, TYPE_NULL, "x", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* v_inner = make_node(NODE_VAL_DEF, TYPE_NULL, "x", OT_PLUS, make_leaf_exp(TYPE_STRING, NULL), NULL, NULL);
    AST_Node* inner_stmts[] = { v_inner };
    AST_Node* else_node = make_node(NODE_ELSE, TYPE_NULL, NULL, OT_PLUS, NULL, NULL, NULL);
    AST_Node* if_node = make_node(NODE_IF, TYPE_NULL, NULL, OT_PLUS,
                                  make_leaf_exp(TYPE_BOOL, NULL),
                                  make_stmt_list(inner_stmts, 1),
                                  else_node);
    AST_Node* stmts[] = { v_outer, if_node };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 2));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(v_inner->data_type == TYPE_STRING);
}
void test_23_shadow_in_inner_scope_ok(void) { run_expect_pass("23_shadow_in_inner_scope_ok", body_shadow_in_inner_scope_ok); }

static void body_undef_var_use(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* stmts[] = { make_leaf_exp(TYPE_NULL, "ghostVar") };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_24_undef_var_exit_3(void) { run_expect_exit("24_undef_var_exit_3", 3, body_undef_var_use); }

static void body_var_out_of_scope(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v_inner = make_node(NODE_VAR_DEF, TYPE_NULL, "scoped", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* inner_stmts[] = { v_inner };
    AST_Node* else_node = make_node(NODE_ELSE, TYPE_NULL, NULL, OT_PLUS, NULL, NULL, NULL);
    AST_Node* if_node = make_node(NODE_IF, TYPE_NULL, NULL, OT_PLUS,
                                  make_leaf_exp(TYPE_BOOL, NULL),
                                  make_stmt_list(inner_stmts, 1),
                                  else_node);
    AST_Node* stmts[] = { if_node, make_leaf_exp(TYPE_NULL, "scoped") };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 2));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_25_var_out_of_scope_exit_3(void) { run_expect_exit("25_var_out_of_scope_exit_3", 3, body_var_out_of_scope); }

static void body_func_used_as_var(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* f = make_func_def("myFunc", TYPE_INT, NULL, NULL);
    AST_Node* stmts[] = { make_leaf_exp(TYPE_NULL, "myFunc") };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);
}
void test_26_func_used_as_var_exit_3(void) { run_expect_exit("26_func_used_as_var_exit_3", 3, body_func_used_as_var); }

// --- 5. Assignment Tests (27-31) ---

static void body_assign_var_ok(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v = make_node(NODE_VAR_DEF, TYPE_NULL, "x", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* a = make_node(NODE_ASSIGN, TYPE_NULL, "x", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* stmts[] = { v, a };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 2));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_27_assign_var_ok(void) { run_expect_pass("27_assign_var_ok", body_assign_var_ok); }

static void body_assign_val_immutable(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v = make_node(NODE_VAL_DEF, TYPE_NULL, "c", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* a = make_node(NODE_ASSIGN, TYPE_NULL, "c", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* stmts[] = { v, a };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 2));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_28_assign_val_exit_7(void) { run_expect_exit("28_assign_val_exit_7", 7, body_assign_val_immutable); }

static void body_assign_param_immutable(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* p = make_param("p", TYPE_INT, NULL);
    AST_Node* a = make_node(NODE_ASSIGN, TYPE_NULL, "p", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* f_stmts[] = { a };
    AST_Node* f = make_func_def("f", TYPE_UNIT, p, make_stmt_list(f_stmts, 1));
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);
}
void test_29_assign_param_exit_7(void) { run_expect_exit("29_assign_param_exit_7", 7, body_assign_param_immutable); }

static void body_assign_undef(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* a = make_node(NODE_ASSIGN, TYPE_NULL, "missing", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* stmts[] = { a };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_30_assign_undef_exit_3(void) { run_expect_exit("30_assign_undef_exit_3", 3, body_assign_undef); }

static void body_assign_type_mismatch(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v = make_node(NODE_VAR_DEF, TYPE_NULL, "x", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* a = make_node(NODE_ASSIGN, TYPE_NULL, "x", OT_PLUS, make_leaf_exp(TYPE_STRING, NULL), NULL, NULL);
    AST_Node* stmts[] = { v, a };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 2));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_31_assign_type_mismatch_exit_6(void) { run_expect_exit("31_assign_type_mismatch_exit_6", 6, body_assign_type_mismatch); }

// --- 6. Expression & Operator Type Tests (32-41) ---

static void body_exp_plus_str_str(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* e = make_bin_exp(OT_PLUS, make_leaf_exp(TYPE_STRING, NULL), make_leaf_exp(TYPE_STRING, NULL));
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(e->data_type == TYPE_STRING);
    assert(e->operator_type == OT_CONCAT);
}
void test_32_exp_plus_str_str(void) { run_expect_pass("32_exp_plus_str_str", body_exp_plus_str_str); }

static void body_exp_plus_str_int_err(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* e = make_bin_exp(OT_PLUS, make_leaf_exp(TYPE_STRING, NULL), make_leaf_exp(TYPE_INT, NULL));
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_33_exp_plus_str_int_exit_6(void) { run_expect_exit("33_exp_plus_str_int_exit_6", 6, body_exp_plus_str_int_err); }

static void body_exp_div_int_double(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* lhs = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* rhs = make_leaf_exp(TYPE_DOUBLE, NULL);
    AST_Node* e = make_bin_exp(OT_DIVIDE, lhs, rhs);
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(e->data_type == TYPE_DOUBLE);
    assert(e->operator_type == OT_DIVIDE);
    assert_wrapped_with(e->left, lhs, NODE_EXP, TYPE_INT);
    assert(e->right == rhs);
}
void test_34_exp_div_int_double(void) { run_expect_pass("34_exp_div_int_double", body_exp_div_int_double); }

static void body_exp_times_str_err(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* e = make_bin_exp(OT_TIMES, make_leaf_exp(TYPE_STRING, NULL), make_leaf_exp(TYPE_INT, NULL));
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_35_exp_times_str_exit_6(void) { run_expect_exit("35_exp_times_str_exit_6", 6, body_exp_times_str_err); }

static void body_exp_less_str_ok(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* e = make_bin_exp(OT_LESS, make_leaf_exp(TYPE_STRING, NULL), make_leaf_exp(TYPE_STRING, NULL));
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(e->data_type == TYPE_BOOL);
}
void test_36_exp_less_str_ok(void) { run_expect_pass("36_exp_less_str_ok", body_exp_less_str_ok); }

static void body_exp_less_str_int_err(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* e = make_bin_exp(OT_LESS, make_leaf_exp(TYPE_STRING, NULL), make_leaf_exp(TYPE_INT, NULL));
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_37_exp_less_str_int_exit_6(void) { run_expect_exit("37_exp_less_str_int_exit_6", 6, body_exp_less_str_int_err); }

static void body_exp_eq_int_double_ok(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* e = make_bin_exp(OT_EQUAL, make_leaf_exp(TYPE_INT, NULL), make_leaf_exp(TYPE_DOUBLE, NULL));
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(e->data_type == TYPE_BOOL);
}
void test_38_exp_eq_int_double_ok(void) { run_expect_pass("38_exp_eq_int_double_ok", body_exp_eq_int_double_ok); }

static void body_exp_neq_int_str_err(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* e = make_bin_exp(OT_NOT_EQUAL, make_leaf_exp(TYPE_INT, NULL), make_leaf_exp(TYPE_STRING, NULL));
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_39_exp_neq_int_str_exit_6(void) { run_expect_exit("39_exp_neq_int_str_exit_6", 6, body_exp_neq_int_str_err); }

static void body_exp_and_bool_ok(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* e = make_bin_exp(OT_AND, make_leaf_exp(TYPE_BOOL, NULL), make_leaf_exp(TYPE_BOOL, NULL));
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(e->data_type == TYPE_BOOL);
}
void test_40_exp_and_bool_ok(void) { run_expect_pass("40_exp_and_bool_ok", body_exp_and_bool_ok); }

static void body_exp_or_int_err(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* e = make_bin_exp(OT_OR, make_leaf_exp(TYPE_INT, NULL), make_leaf_exp(TYPE_BOOL, NULL));
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_41_exp_or_int_exit_6(void) { run_expect_exit("41_exp_or_int_exit_6", 6, body_exp_or_int_err); }

// --- 7. Control Flow & Return Tests (42-50) ---

static void body_if_cond_non_bool(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* else_n = make_node(NODE_ELSE, TYPE_NULL, NULL, OT_PLUS, NULL, NULL, NULL);
    AST_Node* if_n = make_node(NODE_IF, TYPE_NULL, NULL, OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, else_n);
    AST_Node* stmts[] = { if_n };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_42_if_non_bool_exit_6(void) { run_expect_exit("42_if_non_bool_exit_6", 6, body_if_cond_non_bool); }

static void body_while_cond_non_bool(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* w_n = make_node(NODE_WHILE_LOOP, TYPE_NULL, NULL, OT_PLUS, make_leaf_exp(TYPE_STRING, NULL), NULL, NULL);
    AST_Node* stmts[] = { w_n };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_43_while_non_bool_exit_6(void) { run_expect_exit("43_while_non_bool_exit_6", 6, body_while_cond_non_bool); }

static void body_while_cond_bool_ok(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* w_n = make_node(NODE_WHILE_LOOP, TYPE_NULL, NULL, OT_PLUS, make_leaf_exp(TYPE_BOOL, NULL), NULL, NULL);
    AST_Node* stmts[] = { w_n };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_44_while_cond_bool_ok(void) { run_expect_pass("44_while_cond_bool_ok", body_while_cond_bool_ok); }

static void body_return_outside_func(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* ret = make_node(NODE_RETURN, TYPE_NULL, NULL, OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* prog = make_node(NODE_PROGRAM, TYPE_NULL, NULL, OT_PLUS, ret, NULL, NULL);
    prog->right = make_node(NODE_DEF_LIST, TYPE_NULL, NULL, OT_PLUS, m, NULL, NULL);
    semanticCheck(prog, t);
}
void test_45_return_outside_func_exit_10(void) { run_expect_exit("45_return_outside_func_exit_10", 10, body_return_outside_func); }

static void body_return_inferred_type_null(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* ret = make_node(NODE_RETURN, TYPE_NULL, NULL, OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* f_stmts[] = { ret };
    AST_Node* f = make_func_def("inferredWithRet", TYPE_NULL, NULL, make_stmt_list(f_stmts, 1));
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);
}
void test_46_return_inferred_null_exit_6(void) { run_expect_exit("46_return_inferred_null_exit_6", 6, body_return_inferred_type_null); }

static void body_return_unit_bare_ok(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* ret = make_node(NODE_RETURN, TYPE_NULL, NULL, OT_PLUS, NULL, NULL, NULL);
    AST_Node* f_stmts[] = { ret };
    AST_Node* f = make_func_def("unitRet", TYPE_UNIT, NULL, make_stmt_list(f_stmts, 1));
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);
}
void test_47_return_unit_bare_ok(void) { run_expect_pass("47_return_unit_bare_ok", body_return_unit_bare_ok); }

static void body_return_unit_with_expr_err(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* ret = make_node(NODE_RETURN, TYPE_NULL, NULL, OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* f_stmts[] = { ret };
    AST_Node* f = make_func_def("unitWithVal", TYPE_UNIT, NULL, make_stmt_list(f_stmts, 1));
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);
}
void test_48_return_unit_with_val_exit_6(void) { run_expect_exit("48_return_unit_with_val_exit_6", 6, body_return_unit_with_expr_err); }

static void body_return_int_in_double_fn_ok(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* orig = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* ret = make_node(NODE_RETURN, TYPE_NULL, NULL, OT_PLUS, orig, NULL, NULL);
    AST_Node* f_stmts[] = { ret };
    AST_Node* f = make_func_def("doubleFn", TYPE_DOUBLE, NULL, make_stmt_list(f_stmts, 1));
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);
    assert_wrapped_with(ret->left, orig, NODE_EXP, TYPE_INT);
}
void test_49_return_int_in_double_ok(void) { run_expect_pass("49_return_int_in_double_ok", body_return_int_in_double_fn_ok); }

static void body_return_mismatch_err(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* ret = make_node(NODE_RETURN, TYPE_NULL, NULL, OT_PLUS, make_leaf_exp(TYPE_STRING, NULL), NULL, NULL);
    AST_Node* f_stmts[] = { ret };
    AST_Node* f = make_func_def("intFn", TYPE_INT, NULL, make_stmt_list(f_stmts, 1));
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);
}
void test_50_return_mismatch_exit_6(void) { run_expect_exit("50_return_mismatch_exit_6", 6, body_return_mismatch_err); }

// --- 8. Implicit Conversion Wrapping (wrapWithNode) & Operator Rewriting (51-62) ---

static void body_wrap_plus_left_int_right_double(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* lhs = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* rhs = make_leaf_exp(TYPE_DOUBLE, NULL);
    AST_Node* e = make_bin_exp(OT_PLUS, lhs, rhs);
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(e->data_type == TYPE_DOUBLE);
    assert(e->operator_type == OT_PLUS);
    assert_wrapped_with(e->left, lhs, NODE_EXP, TYPE_INT);
    assert(e->right == rhs);
}
void test_51_wrap_plus_left_int_right_double(void) { run_expect_pass("51_wrap_plus_left_int_right_double", body_wrap_plus_left_int_right_double); }

static void body_wrap_plus_left_double_right_int(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* lhs = make_leaf_exp(TYPE_DOUBLE, NULL);
    AST_Node* rhs = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* e = make_bin_exp(OT_PLUS, lhs, rhs);
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(e->data_type == TYPE_DOUBLE);
    assert(e->left == lhs);
    assert_wrapped_with(e->right, rhs, NODE_EXP, TYPE_INT);
}
void test_52_wrap_plus_left_double_right_int(void) { run_expect_pass("52_wrap_plus_left_double_right_int", body_wrap_plus_left_double_right_int); }

static void body_wrap_minus_both_sides(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* l1 = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* r1 = make_leaf_exp(TYPE_DOUBLE, NULL);
    AST_Node* e1 = make_bin_exp(OT_MINUS, l1, r1);

    AST_Node* l2 = make_leaf_exp(TYPE_DOUBLE, NULL);
    AST_Node* r2 = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* e2 = make_bin_exp(OT_MINUS, l2, r2);

    AST_Node* stmts[] = { e1, e2 };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 2));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);

    assert(e1->data_type == TYPE_DOUBLE);
    assert_wrapped_with(e1->left, l1, NODE_EXP, TYPE_INT);
    assert(e1->right == r1);

    assert(e2->data_type == TYPE_DOUBLE);
    assert(e2->left == l2);
    assert_wrapped_with(e2->right, r2, NODE_EXP, TYPE_INT);
}
void test_53_wrap_minus_both_sides(void) { run_expect_pass("53_wrap_minus_both_sides", body_wrap_minus_both_sides); }

static void body_wrap_times_both_sides(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* l1 = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* r1 = make_leaf_exp(TYPE_DOUBLE, NULL);
    AST_Node* e1 = make_bin_exp(OT_TIMES, l1, r1);

    AST_Node* l2 = make_leaf_exp(TYPE_DOUBLE, NULL);
    AST_Node* r2 = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* e2 = make_bin_exp(OT_TIMES, l2, r2);

    AST_Node* stmts[] = { e1, e2 };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 2));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);

    assert(e1->data_type == TYPE_DOUBLE);
    assert_wrapped_with(e1->left, l1, NODE_EXP, TYPE_INT);
    assert(e1->right == r1);

    assert(e2->data_type == TYPE_DOUBLE);
    assert(e2->left == l2);
    assert_wrapped_with(e2->right, r2, NODE_EXP, TYPE_INT);
}
void test_54_wrap_times_both_sides(void) { run_expect_pass("54_wrap_times_both_sides", body_wrap_times_both_sides); }

static void body_wrap_divide_double_left_int_right(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* lhs = make_leaf_exp(TYPE_DOUBLE, NULL);
    AST_Node* rhs = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* e = make_bin_exp(OT_DIVIDE, lhs, rhs);
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(e->data_type == TYPE_DOUBLE);
    assert(e->operator_type == OT_DIVIDE);
    assert(e->left == lhs);
    assert_wrapped_with(e->right, rhs, NODE_EXP, TYPE_INT);
}
void test_55_wrap_divide_double_left_int_right(void) { run_expect_pass("55_wrap_divide_double_left_int_right", body_wrap_divide_double_left_int_right); }

static void body_int_div_becomes_idivide_no_wrap(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* lhs = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* rhs = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* e = make_bin_exp(OT_DIVIDE, lhs, rhs);
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(e->data_type == TYPE_INT);
    assert(e->operator_type == OT_IDIVIDE);
    assert(e->left == lhs);
    assert(e->right == rhs);
}
void test_56_int_div_becomes_idivide_no_wrap(void) { run_expect_pass("56_int_div_becomes_idivide_no_wrap", body_int_div_becomes_idivide_no_wrap); }

static void body_double_div_double_no_wrap(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* lhs = make_leaf_exp(TYPE_DOUBLE, NULL);
    AST_Node* rhs = make_leaf_exp(TYPE_DOUBLE, NULL);
    AST_Node* e = make_bin_exp(OT_DIVIDE, lhs, rhs);
    AST_Node* stmts[] = { e };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(e->data_type == TYPE_DOUBLE);
    assert(e->operator_type == OT_DIVIDE);
    assert(e->left == lhs);
    assert(e->right == rhs);
}
void test_57_double_div_double_no_wrap(void) { run_expect_pass("57_double_div_double_no_wrap", body_double_div_double_no_wrap); }

static void body_wrap_user_func_double_arg_and_tostr_any(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* p2 = make_param("b", TYPE_DOUBLE, NULL);
    AST_Node* p1 = make_param("a", TYPE_INT, p2);
    AST_Node* f = make_func_def("mixArgs", TYPE_UNIT, p1, NULL);

    AST_Node* arg1_orig = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* arg2_orig = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* a2 = make_arg_node(arg2_orig, NULL);
    AST_Node* a1 = make_arg_node(arg1_orig, a2);
    AST_Node* call_mix = make_node(NODE_FUNC_CALL, TYPE_NULL, "mixArgs", OT_PLUS, a1, NULL, NULL);

    AST_Node* any_orig = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* any_arg = make_arg_node(any_orig, NULL);
    AST_Node* call_tostr = make_node(NODE_FUNC_CALL, TYPE_NULL, "toStr", OT_PLUS, any_arg, NULL, NULL);

    AST_Node* stmts[] = { call_mix, call_tostr };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 2));
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);

    assert(a1->left == arg1_orig); // Int -> Int: not wrapped
    assert_wrapped_with(a2->left, arg2_orig, NODE_EXP, TYPE_INT); // Int -> Double: wrapped
    assert(any_arg->left == any_orig); // TYPE_ANY: not wrapped
    assert(call_tostr->data_type == TYPE_STRING);
}
void test_58_wrap_user_func_arg_and_any(void) { run_expect_pass("58_wrap_user_func_arg_and_any", body_wrap_user_func_double_arg_and_tostr_any); }

static void body_wrap_val_double_from_int(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* orig = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* v = make_node(NODE_VAL_DEF, TYPE_DOUBLE, "c", OT_PLUS, orig, NULL, NULL);
    AST_Node* stmts[] = { v };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(v->data_type == TYPE_DOUBLE);
    assert_wrapped_with(v->left, orig, NODE_EXP, TYPE_INT);
}
void test_59_wrap_val_double_from_int(void) { run_expect_pass("59_wrap_val_double_from_int", body_wrap_val_double_from_int); }

static void body_wrap_assign_int_to_double_var(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v = make_node(NODE_VAR_DEF, TYPE_DOUBLE, "d", OT_PLUS, make_leaf_exp(TYPE_DOUBLE, NULL), NULL, NULL);
    AST_Node* rhs = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* a = make_node(NODE_ASSIGN, TYPE_NULL, "d", OT_PLUS, rhs, NULL, NULL);
    AST_Node* stmts[] = { v, a };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 2));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert_wrapped_with(a->left, rhs, NODE_EXP, TYPE_INT);
}
void test_60_wrap_assign_int_to_double_var(void) { run_expect_pass("60_wrap_assign_int_to_double_var", body_wrap_assign_int_to_double_var); }

static void body_assign_double_to_int_var_err(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v = make_node(NODE_VAR_DEF, TYPE_INT, "i", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* a = make_node(NODE_ASSIGN, TYPE_NULL, "i", OT_PLUS, make_leaf_exp(TYPE_DOUBLE, NULL), NULL, NULL);
    AST_Node* stmts[] = { v, a };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 2));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_61_assign_double_to_int_exit_6(void) { run_expect_exit("61_assign_double_to_int_exit_6", 6, body_assign_double_to_int_var_err); }

static void body_var_double_to_int_init_err(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v = make_node(NODE_VAR_DEF, TYPE_INT, "i", OT_PLUS, make_leaf_exp(TYPE_DOUBLE, NULL), NULL, NULL);
    AST_Node* stmts[] = { v };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_62_var_double_to_int_init_exit_6(void) { run_expect_exit("62_var_double_to_int_init_exit_6", 6, body_var_double_to_int_init_err); }

// --- 9. Explicit/Implicit Return & Edge Cases (63-74) ---

static void body_implicit_return_single_expr_wrapped(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* expr = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* f = make_func_def("singleExpr", TYPE_INT, NULL, expr);
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);

    assert(f->right != NULL);
    assert(f->right->node_type == NODE_RETURN);
    assert(f->right->left == expr);
    assert(f->right->data_type == TYPE_INT);
}
void test_63_implicit_return_single_expr_wrapped(void) { run_expect_pass("63_implicit_return_single_expr_wrapped", body_implicit_return_single_expr_wrapped); }

static void body_implicit_return_multi_stmt_wraps_only_last(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* s1 = make_node(NODE_VAR_DEF, TYPE_NULL, "x", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* s2 = make_node(NODE_ASSIGN, TYPE_NULL, "x", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* s3 = make_leaf_exp(TYPE_INT, "x");
    s1->next = s2;
    s2->next = s3;

    AST_Node* f = make_func_def("multiStmt", TYPE_INT, NULL, s1);
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);

    assert(f->right == s1);
    assert(s1->node_type == NODE_VAR_DEF);
    assert(s1->next == s2);
    assert(s2->node_type == NODE_ASSIGN);
    assert(s2->next != NULL);
    assert(s2->next->node_type == NODE_RETURN);
    assert(s2->next->left == s3);
    assert(s3->data_type == TYPE_INT);
}
void test_64_implicit_return_multi_stmt_only_last(void) { run_expect_pass("64_implicit_return_multi_stmt_only_last", body_implicit_return_multi_stmt_wraps_only_last); }

static void body_explicit_return_no_double_wrap(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* expr = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* ret = make_node(NODE_RETURN, TYPE_NULL, NULL, OT_PLUS, expr, NULL, NULL);
    AST_Node* f = make_func_def("explicitRet", TYPE_INT, NULL, ret);
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);

    // Expect a single NODE_RETURN above expr, never NODE_RETURN -> NODE_RETURN
    assert(f->right != NULL);
    assert(f->right->node_type == NODE_RETURN);
    assert(f->right->left != NULL);
    assert(f->right->left->node_type != NODE_RETURN);
}
void test_65_explicit_return_no_double_wrap(void) { run_expect_pass("65_explicit_return_no_double_wrap", body_explicit_return_no_double_wrap); }

static void body_null_ast_root_missing_main(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    semanticCheck(NULL, t);
}
void test_66_null_ast_root_exit_3(void) { run_expect_exit("66_null_ast_root_exit_3", 3, body_null_ast_root_missing_main); }

static void body_empty_def_list_nodes_ok(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* empty_def = make_node(NODE_DEF_LIST, TYPE_NULL, NULL, OT_PLUS, NULL, NULL, NULL);
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* prog = make_node(NODE_PROGRAM, TYPE_NULL, NULL, OT_PLUS, NULL, empty_def, NULL);
    empty_def->right = make_node(NODE_DEF_LIST, TYPE_NULL, NULL, OT_PLUS, m, NULL, NULL);
    semanticCheck(prog, t);
    assert(m->right == NULL); // Empty body stays NULL
}
void test_67_empty_def_list_nodes_ok(void) { run_expect_pass("67_empty_def_list_nodes_ok", body_empty_def_list_nodes_ok); }

static void body_if_else_scopes_isolated(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v_then = make_node(NODE_VAR_DEF, TYPE_NULL, "branchVar", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* v_else = make_node(NODE_VAR_DEF, TYPE_NULL, "branchVar", OT_PLUS, make_leaf_exp(TYPE_STRING, NULL), NULL, NULL);
    AST_Node* then_s[] = { v_then };
    AST_Node* else_s[] = { v_else };
    AST_Node* else_n = make_node(NODE_ELSE, TYPE_NULL, NULL, OT_PLUS, make_stmt_list(else_s, 1), NULL, NULL);
    AST_Node* if_n = make_node(NODE_IF, TYPE_NULL, NULL, OT_PLUS,
                                 make_leaf_exp(TYPE_BOOL, NULL),
                                 make_stmt_list(then_s, 1),
                                 else_n);
    AST_Node* stmts[] = { if_n };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
    assert(v_then->data_type == TYPE_INT);
    assert(v_else->data_type == TYPE_STRING);
}
void test_68_if_else_scopes_isolated_ok(void) { run_expect_pass("68_if_else_scopes_isolated_ok", body_if_else_scopes_isolated); }

static void body_else_var_leaks_after_if(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v_else = make_node(NODE_VAR_DEF, TYPE_NULL, "elseOnly", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* else_s[] = { v_else };
    AST_Node* else_n = make_node(NODE_ELSE, TYPE_NULL, NULL, OT_PLUS, make_stmt_list(else_s, 1), NULL, NULL);
    AST_Node* if_n = make_node(NODE_IF, TYPE_NULL, NULL, OT_PLUS,
                                 make_leaf_exp(TYPE_BOOL, NULL),
                                 make_stmt_list(NULL, 0),
                                 else_n);
    AST_Node* stmts[] = { if_n, make_leaf_exp(TYPE_NULL, "elseOnly") };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 2));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_69_else_var_out_of_scope_exit_3(void) { run_expect_exit("69_else_var_out_of_scope_exit_3", 3, body_else_var_leaks_after_if); }

static void body_while_var_out_of_scope(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* v_loop = make_node(NODE_VAR_DEF, TYPE_NULL, "loopVar", OT_PLUS, make_leaf_exp(TYPE_INT, NULL), NULL, NULL);
    AST_Node* loop_s[] = { v_loop };
    AST_Node* w_n = make_node(NODE_WHILE_LOOP, TYPE_NULL, NULL, OT_PLUS,
                                make_leaf_exp(TYPE_BOOL, NULL),
                                make_stmt_list(loop_s, 1),
                                NULL);
    AST_Node* stmts[] = { w_n, make_leaf_exp(TYPE_NULL, "loopVar") };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 2));
    AST_Node* funcs[] = { m };
    semanticCheck(make_program(funcs, 1), t);
}
void test_70_while_var_out_of_scope_exit_3(void) { run_expect_exit("70_while_var_out_of_scope_exit_3", 3, body_while_var_out_of_scope); }

static void body_nested_return_inside_if_ok(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* orig = make_leaf_exp(TYPE_INT, NULL);
    AST_Node* ret = make_node(NODE_RETURN, TYPE_NULL, NULL, OT_PLUS, orig, NULL, NULL);
    AST_Node* then_s[] = { ret };
    AST_Node* else_n = make_node(NODE_ELSE, TYPE_NULL, NULL, OT_PLUS, NULL, NULL, NULL);
    AST_Node* if_n = make_node(NODE_IF, TYPE_NULL, NULL, OT_PLUS,
                                 make_leaf_exp(TYPE_BOOL, NULL),
                                 make_stmt_list(then_s, 1),
                                 else_n);
    AST_Node* f_stmts[] = { if_n };
    AST_Node* f = make_func_def("retInIf", TYPE_DOUBLE, NULL, make_stmt_list(f_stmts, 1));
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);
    assert_wrapped_with(ret->left, orig, NODE_EXP, TYPE_INT);
}
void test_71_nested_return_inside_if_converts_ok(void) { run_expect_pass("71_nested_return_inside_if_converts_ok", body_nested_return_inside_if_ok); }

static void body_nested_return_inside_while_mismatch(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* ret = make_node(NODE_RETURN, TYPE_NULL, NULL, OT_PLUS, make_leaf_exp(TYPE_STRING, NULL), NULL, NULL);
    AST_Node* loop_s[] = { ret };
    AST_Node* w_n = make_node(NODE_WHILE_LOOP, TYPE_NULL, NULL, OT_PLUS,
                                make_leaf_exp(TYPE_BOOL, NULL),
                                make_stmt_list(loop_s, 1),
                                NULL);
    AST_Node* f_stmts[] = { w_n };
    AST_Node* f = make_func_def("retInWhile", TYPE_INT, NULL, make_stmt_list(f_stmts, 1));
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);
}
void test_72_nested_return_inside_while_exit_6(void) { run_expect_exit("72_nested_return_inside_while_exit_6", 6, body_nested_return_inside_while_mismatch); }

static void body_bare_return_in_non_unit_fn(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* ret = make_node(NODE_RETURN, TYPE_NULL, NULL, OT_PLUS, NULL, NULL, NULL);
    AST_Node* f_stmts[] = { ret };
    AST_Node* f = make_func_def("intBareRet", TYPE_INT, NULL, make_stmt_list(f_stmts, 1));
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, NULL);
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);
}
void test_73_bare_return_in_int_fn_exit_6(void) { run_expect_exit("73_bare_return_in_int_fn_exit_6", 6, body_bare_return_in_non_unit_fn); }

static void body_user_func_wrong_arg_count(void) {
    htab_t* t = htabCreate(HTAB_TEST_BUCKETS);
    AST_Node* p2 = make_param("b", TYPE_INT, NULL);
    AST_Node* p1 = make_param("a", TYPE_INT, p2);
    AST_Node* f = make_func_def("twoArgs", TYPE_INT, p1, NULL);

    AST_Node* a1 = make_arg_node(make_leaf_exp(TYPE_INT, NULL), NULL);
    AST_Node* call = make_node(NODE_FUNC_CALL, TYPE_NULL, "twoArgs", OT_PLUS, a1, NULL, NULL);
    AST_Node* stmts[] = { call };
    AST_Node* m = make_main_def("main", TYPE_UNIT, NULL, make_stmt_list(stmts, 1));
    AST_Node* funcs[] = { f, m };
    semanticCheck(make_program(funcs, 2), t);
}
void test_74_user_func_wrong_arg_count_exit_5(void) { run_expect_exit("74_user_func_wrong_arg_count_exit_5", 5, body_user_func_wrong_arg_count); }

int main(void) {
    printf("Starting Semantic Analyzer Tests (abtDecoding.c) - 74 cases...\n\n");

    // 1. @main & Program Structure
    test_01_valid_main_unit();
    test_02_missing_main_exit_3();
    test_03_duplicate_main_exit_4();
    test_04_main_with_param_exit_5();
    test_05_main_non_unit_exit_6();

    // 2. Builtins
    test_06_builtin_print_string_wrapper();
    test_07_builtin_toint_int_arg();
    test_08_builtin_toint_string_exit_5();
    test_09_builtin_substr_ok();
    test_10_builtin_too_few_args_exit_5();
    test_11_builtin_too_many_args_exit_5();
    test_12_undefined_func_exit_3();

    // 3. User Functions & Calls
    test_13_duplicate_func_exit_4();
    test_14_redefine_builtin_exit_4();
    test_15_forward_func_call_ok();
    test_16_user_func_arg_mismatch_exit_5();
    test_17_duplicate_param_exit_4();

    // 4. Variables & Scoping
    test_18_val_infer_int();
    test_19_var_int_to_double_init();
    test_20_var_init_mismatch_exit_6();
    test_21_var_null_init_exit_6();
    test_22_redef_same_scope_exit_4();
    test_23_shadow_in_inner_scope_ok();
    test_24_undef_var_exit_3();
    test_25_var_out_of_scope_exit_3();
    test_26_func_used_as_var_exit_3();

    // 5. Assignments
    test_27_assign_var_ok();
    test_28_assign_val_exit_7();
    test_29_assign_param_exit_7();
    test_30_assign_undef_exit_3();
    test_31_assign_type_mismatch_exit_6();

    // 6. Expressions & Operators
    test_32_exp_plus_str_str();
    test_33_exp_plus_str_int_exit_6();
    test_34_exp_div_int_double();
    test_35_exp_times_str_exit_6();
    test_36_exp_less_str_ok();
    test_37_exp_less_str_int_exit_6();
    test_38_exp_eq_int_double_ok();
    test_39_exp_neq_int_str_exit_6();
    test_40_exp_and_bool_ok();
    test_41_exp_or_int_exit_6();

    // 7. Control Flow & Returns
    test_42_if_non_bool_exit_6();
    test_43_while_non_bool_exit_6();
    test_44_while_cond_bool_ok();
    test_45_return_outside_func_exit_10();
    test_46_return_inferred_null_exit_6();
    test_47_return_unit_bare_ok();
    test_48_return_unit_with_val_exit_6();
    test_49_return_int_in_double_ok();
    test_50_return_mismatch_exit_6();

    // 8. Implicit Conversion Wrapping (wrapWithNode) & Operator Rewriting
    test_51_wrap_plus_left_int_right_double();
    test_52_wrap_plus_left_double_right_int();
    test_53_wrap_minus_both_sides();
    test_54_wrap_times_both_sides();
    test_55_wrap_divide_double_left_int_right();
    test_56_int_div_becomes_idivide_no_wrap();
    test_57_double_div_double_no_wrap();
    test_58_wrap_user_func_arg_and_any();
    test_59_wrap_val_double_from_int();
    test_60_wrap_assign_int_to_double_var();
    test_61_assign_double_to_int_exit_6();
    test_62_var_double_to_int_init_exit_6();

    // 9. Explicit/Implicit Return & Edge Cases
    test_63_implicit_return_single_expr_wrapped();
    test_64_implicit_return_multi_stmt_only_last();
    test_65_explicit_return_no_double_wrap();
    test_66_null_ast_root_exit_3();
    test_67_empty_def_list_nodes_ok();
    test_68_if_else_scopes_isolated_ok();
    test_69_else_var_out_of_scope_exit_3();
    test_70_while_var_out_of_scope_exit_3();
    test_71_nested_return_inside_if_converts_ok();
    test_72_nested_return_inside_while_exit_6();
    test_73_bare_return_in_int_fn_exit_6();
    test_74_user_func_wrong_arg_count_exit_5();

    printf("\nAll 74 semantic tests passed!\n");
    return 0;
}