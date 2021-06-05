//
// Created by gyorgy on 17/05/2021.
//
#include <test.h>
#include <apint.h>
#include <apfp.h>
#include <apbar.h>

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

            ASSERT_EQUAL_UL(apint_test[0]->limbs[1], 1llu);
    });

    TEST_CASE(shift left by more than 64 bits, {
            apint_setlimb(apint_test[0], 0, 1);
            apint_setlimb(apint_test[0], 1, 0);
            apint_shiftl(apint_test[0], 65);

            ASSERT_EQUAL_UL(apint_test[0]->limbs[1], 2llu);
    });

    TEST_CASE(shift right less than 64 bits, {
            apint_setlimb(apint_test[0], 0, 0);
            apint_setlimb(apint_test[0], 1, 1);
            apint_shiftr(apint_test[0], 1);

            ASSERT_EQUAL_UL(apint_test[0]->limbs[0], 0x8000000000000000llu);
    });

    TEST_CASE(shift right by more than 64 bits, {
            apint_setlimb(apint_test[0], 0, 0);
            apint_setlimb(apint_test[0], 1, 2);
            apint_shiftr(apint_test[0], 65);

            ASSERT_EQUAL_UL(apint_test[0]->limbs[0], 1llu);
    });

    TEST_CASE( addition with positive numbers, {
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

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 3llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, 1);
    });

    TEST_CASE(addition with both numbers negative, {
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

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 3llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, -1);
    });

    TEST_CASE(subtraction with positive numbers a-b a>b, {
            apint_setlimb(apint_test[0], 3, 4);
            apint_setlimb(apint_test[0], 2, 6);
            apint_setlimb(apint_test[0], 1, 2);
            apint_setlimb(apint_test[0], 0, 5);
            apint_test[0]->sign = 1;

            apint_setlimb(apint_test[1], 3, 3);
            apint_setlimb(apint_test[1], 2, 2);
            apint_setlimb(apint_test[1], 1, 1);
            apint_setlimb(apint_test[1], 0, 0);
            apint_test[1]->sign = 1;

            apint_sub(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 4llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 5llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, 1);
    });

    TEST_CASE(subtraction with positive numbers a-b a<b, {

            apint_setlimb(apint_test[0], 3, 3);
            apint_setlimb(apint_test[0], 2, 2);
            apint_setlimb(apint_test[0], 1, 1);
            apint_setlimb(apint_test[0], 0, 0);
            apint_test[0]->sign = 1;

            apint_setlimb(apint_test[1], 3, 4);
            apint_setlimb(apint_test[1], 2, 6);
            apint_setlimb(apint_test[1], 1, 2);
            apint_setlimb(apint_test[1], 0, 5);
            apint_test[1]->sign = 1;

            apint_sub(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 4llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 5llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, -1);
    });

    TEST_CASE(subtraction with positive - negative number a-b a>b, {
            apint_setlimb(apint_test[0], 3, 4);
            apint_setlimb(apint_test[0], 2, 6);
            apint_setlimb(apint_test[0], 1, 2);
            apint_setlimb(apint_test[0], 0, 5);
            apint_test[0]->sign = 1;

            apint_setlimb(apint_test[1], 3, 3);
            apint_setlimb(apint_test[1], 2, 2);
            apint_setlimb(apint_test[1], 1, 1);
            apint_setlimb(apint_test[1], 0, 0);
            apint_test[1]->sign = -1;

            apint_sub(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 7llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 8llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 5llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, 1);
    });

    TEST_CASE(subtraction with negative - positive number a-b a>b, {
            apint_setlimb(apint_test[0], 3, 4);
            apint_setlimb(apint_test[0], 2, 6);
            apint_setlimb(apint_test[0], 1, 2);
            apint_setlimb(apint_test[0], 0, 5);
            apint_test[0]->sign = -1;

            apint_setlimb(apint_test[1], 3, 3);
            apint_setlimb(apint_test[1], 2, 2);
            apint_setlimb(apint_test[1], 1, 1);
            apint_setlimb(apint_test[1], 0, 0);
            apint_test[1]->sign = 1;

            apint_sub(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 7llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 8llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 5llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, -1);
    });

    TEST_CASE(subtraction with borrow, {
            apint_setlimb(apint_test[0], 3, 1);
            apint_setlimb(apint_test[0], 2, 0);
            apint_setlimb(apint_test[0], 1, 0);
            apint_setlimb(apint_test[0], 0, 0);
            apint_test[0]->sign = 1;

            apint_setlimb(apint_test[1], 3, 0);
            apint_setlimb(apint_test[1], 2, 0);
            apint_setlimb(apint_test[1], 1, 0);
            apint_setlimb(apint_test[1], 0, 1);
            apint_test[1]->sign = 1;

            unsigned char borrow = apint_sub(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 18446744073709551615llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 18446744073709551615llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 18446744073709551615llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, 1);
            ASSERT_EQUAL_I(borrow, 0);
    });

    TEST_CASE(addition with positive numbers, {
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

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 3llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, 1);
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

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 3llu);
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

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 4llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 2llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 2llu);

            ASSERT_EQUAL_I(carry, 1);
    });

    TEST_CASE( multiply, {
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
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 2llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 4llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 6llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 8llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 4), 6llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 5), 4llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 6), 2llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 7), 0llu);
    });

    TEST_CASE(portable multiply, {
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
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 2llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 4llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 6llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 8llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 4), 6llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 5), 4llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 6), 2llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 7), 0llu);
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

    TEST_CASE(Add two numbers, {
            // Need to re-precision
            apfp_free(apfp_test[0]);
            apfp_free(apfp_test[1]);
            apfp_free(apfp_test[2]);

            apfp_init(apfp_test[0], 128);
            apfp_init(apfp_test[1], 128);
            apfp_init(apfp_test[2], 128);

            // 1 * 2^-127
            apfp_set_mant_msb(apfp_test[0]);
            apfp_set_exp(apfp_test[0], -254);

            // 16703571626015105435307505830654230989 * 2^-249
            apfp_set_exp(apfp_test[1], -253);
            apfp_set_mant(apfp_test[1], 1, 0xc90fdaa22168c234);
            apfp_set_mant(apfp_test[1], 0, 0xc4c6628b80dc1cd0);

            apfp_add(apfp_test[2], apfp_test[1], apfp_test[0]);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 1), 0x8487ed5110b4611allu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 0x62633145c06e0e68llu);

            //Check value of exponent and sign
            ASSERT_EQUAL_L(apfp_test[2]->exp, -252l);
    });

    TEST_CASE(apfp_add addition with positive numbers 1.000...00 X 2^10 + 1.000...00 X 2^10, {
            // Check mantissa first
            // Then check expected exponent
            apfp_set_mant(apfp_test[0], 3, 9223372036854775808llu);
            apfp_set_mant(apfp_test[0], 2, 0);
            apfp_set_mant(apfp_test[0], 1, 0);
            apfp_set_mant(apfp_test[0], 0, 0);
            apfp_test[0]->mant->sign = 1;
            apfp_test[0]->exp = 10;

            apfp_set_mant(apfp_test[1], 3, 9223372036854775808llu);
            apfp_set_mant(apfp_test[1], 2, 0);
            apfp_set_mant(apfp_test[1], 1, 0);
            apfp_set_mant(apfp_test[1], 0, 0);
            apfp_test[1]->mant->sign = 1;
            apfp_test[1]->exp = 10;

            apfp_add(apfp_test[2], apfp_test[0], apfp_test[1]);

            //Check value of mantissa
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 3), 9223372036854775808llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 2), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 1), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 0llu);

            //Check value of exponent and sign
            ASSERT_EQUAL_L(apfp_test[2]->exp, 11lu);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, 1);
    });

    TEST_CASE(apfp_add addition with positive numbers 1.111...11 X 2^10 + 1.111...11 X 2^10, {
            // Check mantissa first
            // Then check expected exponent
            apfp_set_mant(apfp_test[0], 3, 18446744073709551615llu);
            apfp_set_mant(apfp_test[0], 2, 18446744073709551615llu);
            apfp_set_mant(apfp_test[0], 1, 18446744073709551615llu);
            apfp_set_mant(apfp_test[0], 0, 18446744073709551615llu);
            apfp_test[0]->mant->sign = 1;
            apfp_test[0]->exp = 10;

            apfp_set_mant(apfp_test[1], 3, 18446744073709551615llu);
            apfp_set_mant(apfp_test[1], 2, 18446744073709551615llu);
            apfp_set_mant(apfp_test[1], 1, 18446744073709551615llu);
            apfp_set_mant(apfp_test[1], 0, 18446744073709551615llu);
            apfp_test[1]->mant->sign = 1;
            apfp_test[1]->exp = 10;

            apfp_add(apfp_test[2], apfp_test[0], apfp_test[1]);

            //Check value of mantissa
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 3), 18446744073709551615llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 2), 18446744073709551615llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 1), 18446744073709551615llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 18446744073709551615llu);

            //Check value of exponent and sign
            ASSERT_EQUAL_L(apfp_test[2]->exp, 11lu);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, 1);
    });

    TEST_CASE(apfp_add addition with positive numbers 1.000...01 X 2^256 + 1.000...00 X 2^1, {
            // Check mantissa first
            // Then check expected exponent
            apfp_set_mant(apfp_test[0], 3, 9223372036854775808llu);
            apfp_set_mant(apfp_test[0], 2, 0);
            apfp_set_mant(apfp_test[0], 1, 0);
            apfp_set_mant(apfp_test[0], 0, 1);
            apfp_test[0]->mant->sign = 1;
            apfp_test[0]->exp = 256;

            apfp_set_mant(apfp_test[1], 3, 9223372036854775808llu);
            apfp_set_mant(apfp_test[1], 2, 0);
            apfp_set_mant(apfp_test[1], 1, 0);
            apfp_set_mant(apfp_test[1], 0, 0);
            apfp_test[1]->mant->sign = 1;
            apfp_test[1]->exp = 1;

            apfp_add(apfp_test[2], apfp_test[0], apfp_test[1]);

            //Check value of mantissa
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 3), 9223372036854775808llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 2), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 1), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 2llu);

            //Check value of exponent and sign
            ASSERT_EQUAL_L(apfp_test[2]->exp, 256lu);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, 1);
    });

    TEST_CASE(apfp_add addition with positive and negative number 1.000...00 X 2^256 + -1.000...00 X 2^192, {
            // Check mantissa first
            // Then check expected exponent
            apfp_set_mant_msb(apfp_test[0]);
            apfp_test[0]->mant->sign = 1;
            apfp_test[0]->exp = 256;

            apfp_set_mant_msb(apfp_test[1]);
            apfp_test[1]->mant->sign = -1;
            apfp_test[1]->exp = 192;

            apfp_add(apfp_test[2], apfp_test[0], apfp_test[1]);

            //Check value of mantissa
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 3), 18446744073709551615llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 2), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 1), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 0llu);

            //Check value of exponent and sign
            ASSERT_EQUAL_L(apfp_test[2]->exp, 255lu);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, 1);
    });

    //Subtraction code
    TEST_CASE(subtraction with positive numbers a-b a>b, {
            // Check mantissa first
            // Then check expected exponent
            apfp_set_mant_msb(apfp_test[0]);
            apfp_test[0]->mant->sign = 1;
            apfp_test[0]->exp = 256;

            apfp_set_mant_msb(apfp_test[1]);
            apfp_test[1]->mant->sign = 1;
            apfp_test[1]->exp = 192;

            apfp_sub(apfp_test[2], apfp_test[0], apfp_test[1]);

            //Check value of mantissa
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 3), 18446744073709551615llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 2), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 1), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 0llu);

            //Check value of exponent and sign
            ASSERT_EQUAL_L(apfp_test[2]->exp, 255lu);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, 1);
    });

    TEST_CASE(subtraction with positive numbers a-b a<b, {

            apfp_set_mant(apfp_test[0], 3, 9223372036854775808llu);
            apfp_set_mant(apfp_test[0], 2, 0);
            apfp_set_mant(apfp_test[0], 1, 0);
            apfp_set_mant(apfp_test[0], 0, 0);
            apfp_test[0]->mant->sign = 1;
            apfp_test[0]->exp = 192;

            apfp_set_mant(apfp_test[1], 3, 9223372036854775808llu);
            apfp_set_mant(apfp_test[1], 2, 0);
            apfp_set_mant(apfp_test[1], 1, 0);
            apfp_set_mant(apfp_test[1], 0, 0);
            apfp_test[1]->mant->sign = 1;
            apfp_test[1]->exp = 256;

            apfp_sub(apfp_test[2], apfp_test[0], apfp_test[1]);

            //Check value of mantissa
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 3), 18446744073709551615llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 2), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 1), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 0llu);

            //Check value of exponent and sign
            ASSERT_EQUAL_L(apfp_test[2]->exp, 255lu);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, -1);
    });

    TEST_CASE(subtraction with positive numbers a-b a>b 2pi - pi, {

            apfp_set_mant(apfp_test[0], 3, 0xC90FDAA22168C234);
            apfp_set_mant(apfp_test[0], 2, 0xC4C6628B80DC1CD1);
            apfp_set_mant(apfp_test[0], 1, 0);
            apfp_set_mant(apfp_test[0], 0, 0);
            apfp_test[0]->mant->sign = 1;
            apfp_test[0]->exp = -125;

            apfp_set_mant(apfp_test[1], 3, 0xC90FDAA22168C234);
            apfp_set_mant(apfp_test[1], 2, 0xC4C6628B80DC1CD1);
            apfp_set_mant(apfp_test[1], 1, 0);
            apfp_set_mant(apfp_test[1], 0, 0);
            apfp_test[1]->mant->sign = 1;
            apfp_test[1]->exp = -126;

            apfp_sub(apfp_test[2], apfp_test[0], apfp_test[1]);

            //Check value of mantissa
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 3), 0xC90FDAA22168C234llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 2), 0xC4C6628B80DC1CD1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 1), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 0llu);

            //Check value of exponent and sign
            ASSERT_EQUAL_L(apfp_test[2]->exp, -126l);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, 1);
    });


    TEST_CASE(subtraction with positive - negative number a-b a>b, {
            apfp_set_mant(apfp_test[0], 3, 9223372036854775808llu);
            apfp_set_mant(apfp_test[0], 2, 0);
            apfp_set_mant(apfp_test[0], 1, 0);
            apfp_set_mant(apfp_test[0], 0, 1);
            apfp_test[0]->mant->sign = 1;
            apfp_test[0]->exp = 256;

            apfp_set_mant(apfp_test[1], 3, 9223372036854775808llu);
            apfp_set_mant(apfp_test[1], 2, 0);
            apfp_set_mant(apfp_test[1], 1, 0);
            apfp_set_mant(apfp_test[1], 0, 0);
            apfp_test[1]->mant->sign = -1;
            apfp_test[1]->exp = 1;

            apfp_sub(apfp_test[2], apfp_test[0], apfp_test[1]);

            //Check value of mantissa
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 3), 9223372036854775808llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 2), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 1), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 2llu);

            //Check value of exponent and sign
            ASSERT_EQUAL_L(apfp_test[2]->exp, 256lu);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, 1);
    });


    TEST_CASE(subtraction with negative - positive number a-b a>b, {
            apfp_set_mant(apfp_test[0], 3, 9223372036854775808llu);
            apfp_set_mant(apfp_test[0], 2, 0);
            apfp_set_mant(apfp_test[0], 1, 0);
            apfp_set_mant(apfp_test[0], 0, 1);
            apfp_test[0]->mant->sign = -1;
            apfp_test[0]->exp = 256;

            apfp_set_mant(apfp_test[1], 3, 9223372036854775808llu);
            apfp_set_mant(apfp_test[1], 2, 0);
            apfp_set_mant(apfp_test[1], 1, 0);
            apfp_set_mant(apfp_test[1], 0, 0);
            apfp_test[1]->mant->sign = 1;
            apfp_test[1]->exp = 1;

            apfp_sub(apfp_test[2], apfp_test[0], apfp_test[1]);

            //Check value of mantissa
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 3), 9223372036854775808llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 2), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 1), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 2llu);

            //Check value of exponent and sign
            ASSERT_EQUAL_L(apfp_test[2]->exp, 256lu);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, -1);
    });

    TEST_CASE(subtract with differing signs and overflow, {
            apfp_set_mant(apfp_test[0], 3, 0xFFFFFFFFFFFFFFFF);
            apfp_set_exp(apfp_test[0], 255);
            apfp_set_pos(apfp_test[0]);

            apfp_set_mant_msb(apfp_test[1]);
            apfp_set_exp(apfp_test[1], 192);
            apfp_set_neg(apfp_test[1]);

            apfp_sub(apfp_test[2], apfp_test[0], apfp_test[1]);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 3), 9223372036854775808llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 2), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 1), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 0llu);

            //Check value of exponent and sign
            ASSERT_EQUAL_L(apfp_test[2]->exp, 256lu);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, 1);

    });

    TEST_CASE(pi * pi, {
            apfp_set_mant(apfp_test[0], 3, 0x6487ED5110B4611A);
            apfp_set_mant(apfp_test[0], 2, 0x62633145C06E0E68);
            apfp_set_mant(apfp_test[0], 1, 0x948127044533E63A);
            apfp_set_mant(apfp_test[0], 0, 0x0105DF531D89CD91);
            apfp_test[0]->mant->sign = 1;
            apfp_test[0]->exp = -253;

            apfp_set_mant(apfp_test[1], 3, 0x6487ED5110B4611A);
            apfp_set_mant(apfp_test[1], 2, 0x62633145C06E0E68);
            apfp_set_mant(apfp_test[1], 1, 0x948127044533E63A);
            apfp_set_mant(apfp_test[1], 0, 0x0105DF531D89CD91);
            apfp_test[1]->mant->sign = 1;
            apfp_test[1]->exp = -253;

            apfp_print_msg("pi is:", apfp_test[0]);

            // Have to reinit result holder
            apfp_free(apfp_test[2]);
            apfp_init(apfp_test[2], 512);
            apfp_mul(apfp_test[2], apfp_test[1], apfp_test[0]);

            apfp_print_msg("pi * pi =", apfp_test[2]);
            ASSERT_EQUAL_L(apfp_test[2]->exp, -506l);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 7), 0x277A79937C8BBCB4llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 6), 0x95B89B36602306B1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 5), 0xC2159A8FF834288Allu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 4), 0x19A0884094F1CDA3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 3), 0xBC5658F0D63B5677llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 2), 0xE42CA89707EA23AEllu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 1), 0x8A103EDE33E3D523llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 0x68906CC684438C21llu);
    });

});

apbar_t apbar_test[3];

static void apbar_test_setup() {
    apbar_init(apbar_test[0], 128);
    apbar_init(apbar_test[1], 128);
    apbar_init(apbar_test[2], 128);
}

static void apbar_test_teardown() {
    apbar_free(apbar_test[0]);
    apbar_free(apbar_test[1]);
    apbar_free(apbar_test[2]);
}

TEST_GROUP(ball_arithmetic, {
    WITH_SETUP(apbar_test_setup);
    WITH_TEARDOWN(apbar_test_teardown);

    TEST_CASE(add pi and pi, {
            apbar_set_midpt_mant(apbar_test[0], 1, 0xC90FDAA22168C234);
            apbar_set_midpt_mant(apbar_test[0], 0, 0xC4C6628B80DC1CD1);
            apbar_set_midpt_exp(apbar_test[0], -126);
            apbar_set_rad(apbar_test[0], 536870912, -156);
            apbar_test[0]->midpt->mant->sign=1;

            apbar_set_midpt_mant(apbar_test[1], 1, 0xC90FDAA22168C234);
            apbar_set_midpt_mant(apbar_test[1], 0, 0xC4C6628B80DC1CD1);
            apbar_set_midpt_exp(apbar_test[1], -126);
            apbar_set_rad(apbar_test[1], 536870912, -156);
            apbar_test[1]->midpt->mant->sign=1;

            apbar_print_msg("pi is:", apbar_test[0]);

            apbar_add(apbar_test[2], apbar_test[1], apbar_test[0], 128);

            apbar_print_msg("pi + pi is:", apbar_test[2]);

            // Expected value (128 bit): 267257146016241686964920093290467695825 * 2^-125) +/- (536870913 * 2^-154)
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 1), 0xC90FDAA22168C234llu);
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 0), 0xC4C6628B80DC1CD1llu);
            ASSERT_EQUAL_L(apbar_get_midpt_exp(apbar_test[2]), -125l);

            ASSERT_EQUAL_UL(apbar_test[2]->rad->mant, 2llu);
            ASSERT_EQUAL_L(apbar_test[2]->rad->exp, -126l);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, 1);
    });

    TEST_CASE(subtract pi and -pi, {
            apbar_set_midpt_mant(apbar_test[0], 0, 0xC4C6628B80DC1CD1);
            apbar_set_midpt_mant(apbar_test[0], 1, 0xC90FDAA22168C234);
            apbar_set_midpt_exp(apbar_test[0], -126);
            apbar_set_rad(apbar_test[0], 536870912, -156);
            apbar_test[0]->midpt->mant->sign=1;

            apbar_set_midpt_mant(apbar_test[1], 0, 0xC4C6628B80DC1CD1);
            apbar_set_midpt_mant(apbar_test[1], 1, 0xC90FDAA22168C234);
            apbar_set_midpt_exp(apbar_test[1], -126);
            apbar_set_rad(apbar_test[1], 536870912, -156);
            apbar_test[1]->midpt->mant->sign=-1;

            apbar_print_msg("pi is:", apbar_test[0]);

            apbar_sub(apbar_test[2], apbar_test[1], apbar_test[0], 128);

            apbar_print_msg("pi - -pi is:", apbar_test[2]);

            // Expected value (128 bit): 267257146016241686964920093290467695825 * 2^-125) +/- (536870913 * 2^-154)
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 1), 0xC90FDAA22168C234llu);
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 0), 0xC4C6628B80DC1CD1llu);
            ASSERT_EQUAL_L(apbar_get_midpt_exp(apbar_test[2]), -125l);

            ASSERT_EQUAL_UL(apbar_test[2]->rad->mant, 2llu);
            ASSERT_EQUAL_L(apbar_test[2]->rad->exp, -126l);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, 1);
    });

  /*  TEST_CASE(add pi and -pi, {
            apbar_set_midpt_mant(apbar_test[0], 0, 0xC4C6628B80DC1CD1);
            apbar_set_midpt_mant(apbar_test[0], 1, 0xC90FDAA22168C234);
            apbar_set_midpt_exp(apbar_test[0], -126);
            apbar_set_rad(apbar_test[0], 536870912, -156);
            apbar_test[0]->midpt->mant->sign=1;

            apbar_set_midpt_mant(apbar_test[1], 0, 0xC4C6628B80DC1CD1);
            apbar_set_midpt_mant(apbar_test[1], 1, 0xC90FDAA22168C234);
            apbar_set_midpt_exp(apbar_test[1], -126);
            apbar_set_rad(apbar_test[1], 536870912, -156);
            apbar_test[0]->midpt->mant->sign=-1;

            apbar_print_msg("pi is:", apbar_test[0]);

            apbar_add(apbar_test[2], apbar_test[1], apbar_test[0], 128);

            apbar_print_msg("pi + pi is:", apbar_test[2]);

            // Expected value (128 bit): 267257146016241686964920093290467695825 * 2^-125) +/- (536870913 * 2^-154)
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 1), 0llu);
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 0), 0llu);
            ASSERT_EQUAL_L(apbar_get_midpt_exp(apbar_test[2]), -0llu);

            ASSERT_EQUAL_UL(apbar_test[2]->rad->mant, 2llu);
            ASSERT_EQUAL_L(apbar_test[2]->rad->exp, -126l);
    });*/

    TEST_CASE(subtract 2*pi and pi, {
            apbar_set_midpt_mant(apbar_test[0], 1, 0xC90FDAA22168C234llu);
            apbar_set_midpt_mant(apbar_test[0], 0, 0xC4C6628B80DC1CD1llu);
            apbar_set_midpt_exp(apbar_test[0], -125l);
            apbar_set_rad(apbar_test[0], 536870912, -155);
            apbar_test[0]->midpt->mant->sign = 1;

            apbar_set_midpt_mant(apbar_test[1], 1, 0xC90FDAA22168C234llu);
            apbar_set_midpt_mant(apbar_test[1], 0, 0xC4C6628B80DC1CD1llu);
            apbar_set_midpt_exp(apbar_test[1], -126l);
            apbar_set_rad(apbar_test[1], 536870912, -156l);
            apbar_test[1]->midpt->mant->sign = 1;

            apbar_print_msg("2*pi is:", apbar_test[0]);
            apbar_print_msg("pi is:", apbar_test[1]);

            apbar_sub(apbar_test[2], apbar_test[0], apbar_test[1], 128);

            apbar_print_msg("2*pi - pi is:", apbar_test[2]);
            //printf("\n done with apbar subtract \n");
            // Expected value (128 bit): 267257146016241686964920093290467695825 * 2^-125) +/- (536870913 * 2^-154)
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 1), 0xC90FDAA22168C234llu);
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 0), 0xC4C6628B80DC1CD1llu);
            ASSERT_EQUAL_L(apbar_get_midpt_exp(apbar_test[2]), -126l);

            ASSERT_EQUAL_UL(apbar_test[2]->rad->mant, 2llu);
            ASSERT_EQUAL_L(apbar_test[2]->rad->exp, -126l);
            //printf("\n done with apbar subtract");
    });

    TEST_CASE(mul pi with pi, {
            apbar_set_midpt_mant(apbar_test[0], 0, 0xC4C6628B80DC1CD1);
            apbar_set_midpt_mant(apbar_test[0], 1, 0xC90FDAA22168C234);
            apbar_set_midpt_exp(apbar_test[0], -126);
            apbar_set_rad(apbar_test[0], 536870912, -156);

            apbar_set_midpt_mant(apbar_test[1], 0, 0xC4C6628B80DC1CD1);
            apbar_set_midpt_mant(apbar_test[1], 1, 0xC90FDAA22168C234);
            apbar_set_midpt_exp(apbar_test[1], -126);
            apbar_set_rad(apbar_test[1], 536870912, -156);

            // reinitialize result holder, because it needs to be twice as large
            apbar_free(apbar_test[2]);
            apbar_init(apbar_test[2], 256);

            apbar_mul(apbar_test[2], apbar_test[1], apbar_test[0], 128);

            apbar_print_msg("pi * pi is:", apbar_test[2]);
    });
});

void run_test_suite() {
    RUN_TEST_GROUP(apint);
    RUN_TEST_GROUP(apfp);
    RUN_TEST_GROUP(ball_arithmetic);
}

int main(int argc, char *argv[]) {
    run_test_suite();
}
