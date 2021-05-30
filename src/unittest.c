//
// Created by gyorgy on 17/05/2021.
//
#include <test.h>
#include <apint.h>
#include <apfp.h>

apint_t apint_test[3];

static void apint_test_setup(void) {
    apint_init(apint_test[0], 256);
    apint_init(apint_test[1], 256);
    apint_init(apint_test[2], 512);
}

static void apint_test_teardown(void) {
    apint_free(apint_test[0]);
    apint_free(apint_test[1]);
    apint_free(apint_test[2]);
}

TEST_GROUP(apint, {
    WITH_SETUP(apint_test_setup);
    WITH_TEARDOWN(apint_test_teardown);

    TEST_CASE(shift left less than 64 bits, {
            apint_setlimb(apint_test[0], 0, 0x8000000000000000);
            apint_setlimb(apint_test[0], 1, 0);
            apint_shiftl(apint_test[0], 1);

            ASSERT_EQUAL_UL(apint_test[0]->limbs[1], 1ull);
    });

    TEST_CASE(shift left by more than 64 bits, {
            apint_setlimb(apint_test[0], 0, 1);
            apint_setlimb(apint_test[0], 1, 0);
            apint_shiftl(apint_test[0], 65);

            ASSERT_EQUAL_UL(apint_test[0]->limbs[1], 2ull);
    });

    TEST_CASE(shift right less than 64 bits, {
            apint_setlimb(apint_test[0], 0, 0);
            apint_setlimb(apint_test[0], 1, 1);
            apint_shiftr(apint_test[0], 1);

            ASSERT_EQUAL_UL(apint_test[0]->limbs[0], 0x8000000000000000ull);
    });

    TEST_CASE(shift right by more than 64 bits, {
            apint_setlimb(apint_test[0], 0, 0);
            apint_setlimb(apint_test[0], 1, 2);
            apint_shiftr(apint_test[0], 65);

            ASSERT_EQUAL_UL(apint_test[0]->limbs[0], 1ull);
    });

    TEST_CASE(apint_add addition with positive numbers, {
            apint_setlimb(apint_test[0], 0, 1);
            apint_setlimb(apint_test[0], 1, 1);
            apint_setlimb(apint_test[0], 2, 1);
            apint_setlimb(apint_test[0], 3, 1);
            apint_test[0]->sign = 1;

            apint_setlimb(apint_test[1], 0, 2);
            apint_setlimb(apint_test[1], 1, 2);
            apint_setlimb(apint_test[1], 2, 2);
            apint_setlimb(apint_test[1], 3, 2);
            apint_test[1]->sign = 1;
            apint_add(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 3ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 3ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 3ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 3ull);
            ASSERT_EQUAL_I(apint_test[2]->sign, 1);
    });

    TEST_CASE(apint_add addition with both numbers negative, {
            apint_setlimb(apint_test[0], 0, 1);
            apint_setlimb(apint_test[0], 1, 1);
            apint_setlimb(apint_test[0], 2, 1);
            apint_setlimb(apint_test[0], 3, 1);
            apint_test[0]->sign = -1;

            apint_setlimb(apint_test[1], 0, 2);
            apint_setlimb(apint_test[1], 1, 2);
            apint_setlimb(apint_test[1], 2, 2);
            apint_setlimb(apint_test[1], 3, 2);
            apint_test[1]->sign = -1;
            apint_add(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 3ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 3ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 3ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 3ull);
            ASSERT_EQUAL_I(apint_test[2]->sign, -1);
    });

    TEST_CASE(apint_add addition with positive numbers and overflow, {
            apint_setlimb(apint_test[0], 0, UINT64_MAX);
            apint_setlimb(apint_test[0], 1, 1);
            apint_setlimb(apint_test[0], 2, UINT64_MAX);
            apint_setlimb(apint_test[0], 3, UINT64_MAX - 1);
            apint_test[0]->sign = 1;

            apint_setlimb(apint_test[1], 0, 2);
            apint_setlimb(apint_test[1], 1, 2);
            apint_setlimb(apint_test[1], 2, 3);
            apint_setlimb(apint_test[1], 3, 3);
            apint_test[0]->sign = 1;

            char carry = apint_plus(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 1ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 4ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 2ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 2ull);

            ASSERT_EQUAL_I(carry, 1);
    });


    TEST_CASE(portable addition with positive numbers, {
            apint_setlimb(apint_test[0], 0, 1);
            apint_setlimb(apint_test[0], 1, 1);
            apint_setlimb(apint_test[0], 2, 1);
            apint_setlimb(apint_test[0], 3, 1);

            apint_setlimb(apint_test[1], 0, 2);
            apint_setlimb(apint_test[1], 1, 2);
            apint_setlimb(apint_test[1], 2, 2);
            apint_setlimb(apint_test[1], 3, 2);

            apint_plus_portable(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 3ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 3ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 3ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 3ull);
    });

    TEST_CASE(portable addition with positive numbers and overflow, {
            apint_setlimb(apint_test[0], 0, UINT64_MAX);
            apint_setlimb(apint_test[0], 1, 1);
            apint_setlimb(apint_test[0], 2, UINT64_MAX);
            apint_setlimb(apint_test[0], 3, UINT64_MAX - 1);

            apint_setlimb(apint_test[1], 0, 2);
            apint_setlimb(apint_test[1], 1, 2);
            apint_setlimb(apint_test[1], 2, 3);
            apint_setlimb(apint_test[1], 3, 3);

            char carry = apint_plus_portable(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 1ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 4ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 2ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 2ull);

            ASSERT_EQUAL_I(carry, 1);
    });

    TEST_CASE(apint multiply, {
            apint_setlimb(apint_test[0], 0, 1);
            apint_setlimb(apint_test[0], 1, 1);
            apint_setlimb(apint_test[0], 2, 1);
            apint_setlimb(apint_test[0], 3, 1);

            apint_setlimb(apint_test[1], 0, 2);
            apint_setlimb(apint_test[1], 1, 2);
            apint_setlimb(apint_test[1], 2, 2);
            apint_setlimb(apint_test[1], 3, 2);

            apint_mul(apint_test[2], apint_test[0], apint_test[1]);

            // Calculated with: https://defuse.ca/big-number-calculator.htm
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 2ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 4ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 6ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 8ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 4), 6ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 5), 4ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 6), 2ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 7), 0ull);
    });

    TEST_CASE(portable apint multiply, {
            apint_setlimb(apint_test[0], 0, 1);
            apint_setlimb(apint_test[0], 1, 1);
            apint_setlimb(apint_test[0], 2, 1);
            apint_setlimb(apint_test[0], 3, 1);

            apint_setlimb(apint_test[1], 0, 2);
            apint_setlimb(apint_test[1], 1, 2);
            apint_setlimb(apint_test[1], 2, 2);
            apint_setlimb(apint_test[1], 3, 2);

            apint_mul_portable(apint_test[2], apint_test[0], apint_test[1]);

            // Calculated with: https://defuse.ca/big-number-calculator.htm
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 2ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 4ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 6ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 8ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 4), 6ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 5), 4ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 6), 2ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 7), 0ull);
    });
});

apfp_t apfp_test[3];

static void apfp_test_setup() {
    apfp_init(apfp_test[0], 256);
    apfp_init(apfp_test[1], 256);
    apfp_init(apfp_test[2], 256);
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

    TEST_CASE(apfp_add addition with positive numbers, {
            // Check mantissa first
            // Then check expected exponent
            apfp_set_mant(apfp_test[0], 0, 0);
            apfp_set_mant(apfp_test[0], 1, 0);
            apfp_set_mant(apfp_test[0], 2, 0);
            apfp_set_mant(apfp_test[0], 3, 9223372036854775808);
            apfp_test[0]->mant->sign = 1;
            apfp_test[0]->exp = 10;

            apfp_set_mant(apfp_test[1], 0, 0);
            apfp_set_mant(apfp_test[1], 1, 0);
            apfp_set_mant(apfp_test[1], 2, 0);
            apfp_set_mant(apfp_test[1], 3, 9223372036854775808);
            apfp_test[1]->mant->sign = 1;
            apfp_test[1]->exp = 10;

            apfp_add(apfp_test[2], apfp_test[0], apfp_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 0);

/*            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 3ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 3ull);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 3ull);
            ASSERT_EQUAL_I(apint_test[2]->sign, 1);
*/
    });

})

void run_test_suite() {
    RUN_TEST_GROUP(apint);
    RUN_TEST_GROUP(apfp);
}

int main(int argc, char *argv[]) {
    run_test_suite();
}
