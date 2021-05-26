//
// Created by gyorgy on 17/05/2021.
//
#include <test.h>
#include <apint.h>
#include <apfp.h>

apint_t apint_test;
apint_t apint_other;
apint_t apint_res;

static void apint_test_setup(void) {
    apint_init(apint_test, 256);
    apint_init(apint_other, 256);
    apint_init(apint_res, 512);
}

static void apint_test_teardown(void) {
    apint_free(apint_test);
    apint_free(apint_other);
    apint_free(apint_res);
}

TEST_GROUP(apint, {
    WITH_SETUP(apint_test_setup);
    WITH_TEARDOWN(apint_test_teardown);

    TEST_CASE(shift left less than 64 bits, {
            apint_setlimb(apint_test, 0, 0x8000000000000000);
            apint_setlimb(apint_test, 1, 0);
            apint_shiftl(apint_test, 1);

            ASSERT_EQUAL_UL(apint_test->limbs[1], 1ull);
    });

    TEST_CASE(shift left by more than 64 bits, {
            apint_setlimb(apint_test, 0, 1);
            apint_setlimb(apint_test, 1, 0);
            apint_shiftl(apint_test, 65);

            ASSERT_EQUAL_UL(apint_test->limbs[1], 2ull);
    });

    TEST_CASE(shift right less than 64 bits, {
            apint_setlimb(apint_test, 0, 0);
            apint_setlimb(apint_test, 1, 1);
            apint_shiftr(apint_test, 1);

            ASSERT_EQUAL_UL(apint_test->limbs[0], 0x8000000000000000ull);
    });

    TEST_CASE(shift right by more than 64 bits, {
            apint_setlimb(apint_test, 0, 0);
            apint_setlimb(apint_test, 1, 2);
            apint_shiftr(apint_test, 65);

            ASSERT_EQUAL_UL(apint_test->limbs[0], 1ull);
    });

    TEST_CASE(multiply, {
            apint_setlimb(apint_test, 0, 1);
            apint_setlimb(apint_test, 1, 1);
            apint_setlimb(apint_test, 2, 1);
            apint_setlimb(apint_test, 3, 1);

            apint_setlimb(apint_other, 0, 2);
            apint_setlimb(apint_other, 1, 2);
            apint_setlimb(apint_other, 2, 2);
            apint_setlimb(apint_other, 3, 2);

            apint_mul(apint_res, apint_test, apint_other);

            ASSERT_EQUAL_UL(apint_getlimb(apint_res, 0), 2ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_res, 1), 4ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_res, 2), 6ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_res, 3), 8ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_res, 4), 6ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_res, 5), 4ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_res, 6), 2ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_res, 7), 0ull);
    })
});

apfp_t apfp_test[3];

static void apfp_test_setup() {
    apfp_init(apfp_test[0], 128);
    apfp_init(apfp_test[1], 128);
    apfp_init(apfp_test[2], 128);
}

static void apfp_test_teardown() {
    apfp_free(apfp_test[0]);
    apfp_free(apfp_test[1]);
    apfp_free(apfp_test[2]);
}

TEST_GROUP(apfp, {
    WITH_SETUP(apfp_test_setup);
    WITH_TEARDOWN(apfp_test_teardown);

    TEST_CASE(set to double, {
        apfp_set_d(apfp_test[0], 2.71828);

        ASSERT_EQUAL_L(apfp_test[0]->exp, -51l);
        ASSERT_EQUAL_UL(apfp_test[0]->mant->limbs[0], 0x15BF0995AAF790ull);
    });
})

void run_test_suite() {
    RUN_TEST_GROUP(apint);
    RUN_TEST_GROUP(apfp);
}

int main(int argc, char *argv[]) {
    run_test_suite();
}
