//
// Created by gyorgy on 02/05/2021.
//

#ifndef BALL_ARITHMETIC_TEST_H
#define BALL_ARITHMETIC_TEST_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define RESET_COLOUR "\033[0m"
#define GREEN(TEXT) "\033[0;32m" #TEXT RESET_COLOUR
#define RED(TEXT) "\033[0;31m" #TEXT RESET_COLOUR

typedef void (*helper_t)(void);
helper_t setup = NULL;
helper_t teardown = NULL;

/* --- MACROS --- */
#define BOOL_TO_STR(VAL) VAL == true ? "true" : "false"

#define BAIL goto out

#define ASSERT_EQUAL_I(ACTUAL, EXPECTED)                                       \
if ((EXPECTED) != (ACTUAL)) {                                                  \
    printf("\t---- %d expected got %d at line %d", EXPECTED, ACTUAL, __LINE__);\
    printf(" (ASSERT_EQUAL_I(" #ACTUAL ", " #EXPECTED "))\n");                 \
    did_pass = false;                                                          \
    BAIL;                                                                      \
}

#define ASSERT_EQUAL_L(ACTUAL, EXPECTED)                                       \
if ((EXPECTED) != (ACTUAL)) {                                                  \
    printf("\t---- %ld expected got %ld at line %d", EXPECTED, ACTUAL, __LINE__);\
    printf(" (ASSERT_EQUAL_L(" #ACTUAL ", " #EXPECTED "))\n");                 \
    did_pass = false;                                                          \
    BAIL;                                                                      \
}

#define ASSERT_LARGER_OR_EQUAL_UL(LHS, RHS)                                    \
if ((LHS) < (RHS)) {                                                           \
    printf("\t---- %llu smaller than %llu at line %d", LHS, RHS, __LINE__);    \
    printf("(ASSERT_LARGER_OR_EQUAL_UL(" #LHS ", " #RHS ")\n");                \
    did_pass = false;                                                          \
    BAIL;                                                                      \
}

#define ASSERT_EQUAL_UL(ACTUAL, EXPECTED)                                      \
if ((EXPECTED) != (ACTUAL)) {                                                  \
    printf("\t---- %llu expected got %llu at line ", EXPECTED, ACTUAL);        \
    printf("%d (ASSERT_EQUAL_UL(" #ACTUAL ", " #EXPECTED ")\n", __LINE__);     \
    did_pass = false;                                                          \
    BAIL;                                                                      \
}

#define ASSERT_NOT_EQUAL_UL(ACTUAL, EXPECTED)                                  \
if ((EXPECTED) == (ACTUAL)) {                                                  \
    printf("\t---- Assertion not equal failed for: ");                         \
    printf(#ACTUAL " and " #EXPECTED " on line: %d\n", __LINE__);              \
    did_pass = false;                                                          \
    BAIL;                                                                      \
}

#define ASSERT_TRUE(EXPR)                                                      \
if (!(EXPR)) {                                                                 \
    printf("\t---- Expression " #EXPR " is false (expected true)");            \
    printf(" at line %d\n", __LINE__);                                         \
    did_pass = false;                                                          \
    BAIL;                                                                      \
}

#define ASSERT_NOT_NULL(EXPR)                                                  \
if(!(EXPR)){                                                                   \
    printf("\t---- Expression is null (expected non-null) at line %d\n", __LINE__); \
    did_pass = false;                                                          \
    BAIL;                                                                      \
}

#define TEST_GROUP(NAME, CODE)                                                 \
void test_##NAME(void);                                                        \
void test_##NAME(void) {                                                       \
    for (int i = 0; i < 80; i++) printf("=");                                  \
    printf("\n");                                                              \
    printf("Running test group: " #NAME "\n");                                 \
    for (int i = 0; i < 80; i++) printf("-");                                  \
    printf("\n");                                                              \
    uint32_t case_no = 0;                                                      \
    uint32_t pass_no = 0;                                                      \
    CODE                                                                       \
    printf("cases: %u pass: %u fail: %u\n", case_no, pass_no, case_no-pass_no);\
    setup = NULL;                                                              \
    teardown = NULL;                                                           \
}

#define WITH_SETUP(FUNC) setup = FUNC

#define WITH_TEARDOWN(FUNC) teardown = FUNC

#define TEST_CASE(NAME, CODE) {                                                \
    __label__ out;                                                             \
    if (setup != NULL) setup();                                                \
    case_no++;                                                                 \
    bool did_pass = true;                                                      \
    printf("  Case %s\n", #NAME ":");                                          \
    CODE                                                                       \
    out:                                                                       \
    printf("%90s\n", did_pass ? GREEN([PASS]): RED([FAIL]));                   \
    for (int i = 0; i < 80; i++) printf("-");                                  \
    printf("\n");                                                              \
    if (did_pass) pass_no++;                                                   \
    if (teardown != NULL) teardown();                                          \
}

#define RUN_TEST_GROUP(NAME) test_##NAME()

/* --- Function definitions --- */

void run_test_suite(void);

#endif //BALL_ARITHMETIC_TEST_H
