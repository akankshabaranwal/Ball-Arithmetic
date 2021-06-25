//
// Created by gyorgy on 17/05/2021.
//
#include <test.h>
#include <apint.h>
#include <apfp.h>
#include <apbar.h>
#include <apbar2.h>

apint_t apint_test[3];

static void apint_test_setup(void) {
    apint_init(apint_test[0], 512);
    apint_init(apint_test[1], 512);
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

    TEST_CASE(shift right and copy less than 64 bits, {
            apint_setlimb(apint_test[0], 0, 0);
            apint_setlimb(apint_test[0], 1, 1);
            apint_shiftr_copy(apint_test[1], apint_test[0], 1);

            ASSERT_EQUAL_UL(apint_test[1]->limbs[0], 0x8000000000000000llu);
    });

    TEST_CASE(shift right and copy by more than 64 bits, {
            apint_setlimb(apint_test[0], 0, 0);
            apint_setlimb(apint_test[0], 1, 2);
            apint_shiftr_copy(apint_test[1], apint_test[0], 65);

            ASSERT_EQUAL_UL(apint_test[1]->limbs[0], 1llu);
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

            // Carry
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 4), 1llu);

    });

    TEST_CASE(portable subtraction with positive numbers a-b a>b, {
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

            apint_sub_portable(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 4llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 5llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, 1);
    });

    TEST_CASE(portable subtraction with positive numbers a-b a<b, {

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

            apint_sub_portable(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 4llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 5llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, -1);
    });

    TEST_CASE(portable subtraction with positive - negative number a-b a>b, {
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

            apint_sub_portable(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 7llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 8llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 5llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, 1);
    });

    TEST_CASE(portable subtraction with negative - positive number a-b a>b, {
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

            apint_sub_portable(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 7llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 8llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 5llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, -1);
    });

    TEST_CASE(portable subtraction with borrow, {
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

            unsigned char borrow = apint_sub_portable(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 18446744073709551615llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 18446744073709551615llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 18446744073709551615llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, 1);
            ASSERT_EQUAL_I(borrow, 0);
    });

    TEST_CASE(portable subtraction with max limb and borrow, {
            apint_setlimb(apint_test[0], 3, 1);
            apint_setlimb(apint_test[0], 2, 0);
            apint_setlimb(apint_test[0], 1, 0);
            apint_setlimb(apint_test[0], 0, 0);
            apint_test[0]->sign = 1;

            apint_setlimb(apint_test[1], 3, 0);
            apint_setlimb(apint_test[1], 2, UINT64_MAX);
            apint_setlimb(apint_test[1], 1, 1);
            apint_setlimb(apint_test[1], 0, 0);
            apint_test[1]->sign = 1;

            unsigned char borrow = apint_sub_portable(apint_test[2], apint_test[0], apint_test[1]);

            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), (apint_limb_t) UINT64_MAX);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 0llu);
            ASSERT_EQUAL_I(apint_test[2]->sign, 1);
            ASSERT_EQUAL_I(borrow, 0);
    });

    TEST_CASE(multiply, {
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

    TEST_CASE(multiply mantissa of pi with itself, {
            apint_setlimb(apint_test[0], 3, 0x6487ED5110B4611A);
            apint_setlimb(apint_test[0], 2, 0x62633145C06E0E68);
            apint_setlimb(apint_test[0], 1, 0x948127044533E63A);
            apint_setlimb(apint_test[0], 0, 0x0105DF531D89CD91);

            apint_setlimb(apint_test[1], 3, 0x6487ED5110B4611A);
            apint_setlimb(apint_test[1], 2, 0x62633145C06E0E68);
            apint_setlimb(apint_test[1], 1, 0x948127044533E63A);
            apint_setlimb(apint_test[1], 0, 0x0105DF531D89CD91);

            apint_mul(apint_test[2], apint_test[0], apint_test[1]);

            // Calculated with: https://defuse.ca/big-number-calculator.htm
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 0x68906cc684438c21llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 0x8a103ede33e3d523llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 0xe42ca89707ea23aellu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 0xbc5658f0d63b5677llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 4), 0x19a0884094f1cda3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 5), 0xc2159a8ff834288allu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 6), 0x95b89b36602306b1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 7), 0x277a79937c8bbcb4llu);
    });

    TEST_CASE(multiply propagate, {
            apint_setlimb(apint_test[0], 3, 0xffffffffffffffff);
            apint_setlimb(apint_test[0], 2, 0xffffffffffffffff);
            apint_setlimb(apint_test[0], 1, 0xffffffffffffffff);
            apint_setlimb(apint_test[0], 0, 0xffffffffffffffff);

            apint_setlimb(apint_test[1], 3, 0xffffffffffffffff);
            apint_setlimb(apint_test[1], 2, 0xffffffffffffffff);
            apint_setlimb(apint_test[1], 1, 0xffffffffffffffff);
            apint_setlimb(apint_test[1], 0, 0xffffffffffffffff);

            apint_mul(apint_test[2], apint_test[0], apint_test[1]);

            // Calculated with: https://defuse.ca/big-number-calculator.htm
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 0x0000000000000001llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 4), 0xfffffffffffffffellu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 5), 0xffffffffffffffffllu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 6), 0xffffffffffffffffllu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 7), 0xffffffffffffffffllu);
    });

    TEST_CASE(multiply mantissa of pi with itself karatsuba, {
            apint_setlimb(apint_test[0], 3, 0x6487ED5110B4611A);
            apint_setlimb(apint_test[0], 2, 0x62633145C06E0E68);
            apint_setlimb(apint_test[0], 1, 0x948127044533E63A);
            apint_setlimb(apint_test[0], 0, 0x0105DF531D89CD91);

            apint_setlimb(apint_test[1], 3, 0x6487ED5110B4611A);
            apint_setlimb(apint_test[1], 2, 0x62633145C06E0E68);
            apint_setlimb(apint_test[1], 1, 0x948127044533E63A);
            apint_setlimb(apint_test[1], 0, 0x0105DF531D89CD91);

            apint_mul_karatsuba(apint_test[2], apint_test[0], apint_test[1]);

            // Calculated with: https://defuse.ca/big-number-calculator.htm
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 0x68906cc684438c21llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 0x8a103ede33e3d523llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 0xe42ca89707ea23aellu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 0xbc5658f0d63b5677llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 4), 0x19a0884094f1cda3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 5), 0xc2159a8ff834288allu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 6), 0x95b89b36602306b1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 7), 0x277a79937c8bbcb4llu);
    });

    TEST_CASE(multiply propagate karatsuba, {
            apint_setlimb(apint_test[0], 3, 0xffffffffffffffff);
            apint_setlimb(apint_test[0], 2, 0xffffffffffffffff);
            apint_setlimb(apint_test[0], 1, 0xffffffffffffffff);
            apint_setlimb(apint_test[0], 0, 0xffffffffffffffff);

            apint_setlimb(apint_test[1], 3, 0xffffffffffffffff);
            apint_setlimb(apint_test[1], 2, 0xffffffffffffffff);
            apint_setlimb(apint_test[1], 1, 0xffffffffffffffff);
            apint_setlimb(apint_test[1], 0, 0xffffffffffffffff);

            apint_mul_karatsuba(apint_test[2], apint_test[0], apint_test[1]);

            // Calculated with: https://defuse.ca/big-number-calculator.htm
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 0x0000000000000001llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 4), 0xfffffffffffffffellu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 5), 0xffffffffffffffffllu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 6), 0xffffffffffffffffllu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 7), 0xffffffffffffffffllu);
    });

    TEST_CASE(multiply unrolled, {
            apint_setlimb(apint_test[0], 0, 1);
            apint_setlimb(apint_test[0], 1, 1);
            apint_setlimb(apint_test[0], 2, 1);
            apint_setlimb(apint_test[0], 3, 1);

            apint_setlimb(apint_test[1], 0, 2);
            apint_setlimb(apint_test[1], 1, 2);
            apint_setlimb(apint_test[1], 2, 2);
            apint_setlimb(apint_test[1], 3, 2);

            apint_mul_unroll(apint_test[2], apint_test[0], apint_test[1]);

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

    TEST_CASE(multiply mantissa of pi with itself unrolled, {
            apint_setlimb(apint_test[0], 3, 0x6487ED5110B4611A);
            apint_setlimb(apint_test[0], 2, 0x62633145C06E0E68);
            apint_setlimb(apint_test[0], 1, 0x948127044533E63A);
            apint_setlimb(apint_test[0], 0, 0x0105DF531D89CD91);

            apint_setlimb(apint_test[1], 3, 0x6487ED5110B4611A);
            apint_setlimb(apint_test[1], 2, 0x62633145C06E0E68);
            apint_setlimb(apint_test[1], 1, 0x948127044533E63A);
            apint_setlimb(apint_test[1], 0, 0x0105DF531D89CD91);

            apint_mul_unroll(apint_test[2], apint_test[0], apint_test[1]);

            // Calculated with: https://defuse.ca/big-number-calculator.htm
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 0x68906cc684438c21llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 0x8a103ede33e3d523llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 0xe42ca89707ea23aellu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 0xbc5658f0d63b5677llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 4), 0x19a0884094f1cda3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 5), 0xc2159a8ff834288allu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 6), 0x95b89b36602306b1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 7), 0x277a79937c8bbcb4llu);
    });

    TEST_CASE(multiply propagate unrolled, {
            apint_setlimb(apint_test[0], 3, 0xffffffffffffffff);
            apint_setlimb(apint_test[0], 2, 0xffffffffffffffff);
            apint_setlimb(apint_test[0], 1, 0xffffffffffffffff);
            apint_setlimb(apint_test[0], 0, 0xffffffffffffffff);

            apint_setlimb(apint_test[1], 3, 0xffffffffffffffff);
            apint_setlimb(apint_test[1], 2, 0xffffffffffffffff);
            apint_setlimb(apint_test[1], 1, 0xffffffffffffffff);
            apint_setlimb(apint_test[1], 0, 0xffffffffffffffff);

            apint_mul_unroll(apint_test[2], apint_test[0], apint_test[1]);

            // Calculated with: https://defuse.ca/big-number-calculator.htm
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 0x0000000000000001llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 4), 0xfffffffffffffffellu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 5), 0xffffffffffffffffllu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 6), 0xffffffffffffffffllu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 7), 0xffffffffffffffffllu);
    });

    TEST_CASE(multiply unrolled12, {
            apint_setlimb(apint_test[0], 0, 1);
            apint_setlimb(apint_test[0], 1, 1);
            apint_setlimb(apint_test[0], 2, 1);
            apint_setlimb(apint_test[0], 3, 1);

            apint_setlimb(apint_test[1], 0, 2);
            apint_setlimb(apint_test[1], 1, 2);
            apint_setlimb(apint_test[1], 2, 2);
            apint_setlimb(apint_test[1], 3, 2);

            apint_mul_unroll12(apint_test[2], apint_test[0], apint_test[1]);

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

    TEST_CASE(multiply mantissa of pi with itself unrolled12, {
            apint_setlimb(apint_test[0], 3, 0x6487ED5110B4611A);
            apint_setlimb(apint_test[0], 2, 0x62633145C06E0E68);
            apint_setlimb(apint_test[0], 1, 0x948127044533E63A);
            apint_setlimb(apint_test[0], 0, 0x0105DF531D89CD91);

            apint_setlimb(apint_test[1], 3, 0x6487ED5110B4611A);
            apint_setlimb(apint_test[1], 2, 0x62633145C06E0E68);
            apint_setlimb(apint_test[1], 1, 0x948127044533E63A);
            apint_setlimb(apint_test[1], 0, 0x0105DF531D89CD91);

            apint_mul_unroll12(apint_test[2], apint_test[0], apint_test[1]);

            // Calculated with: https://defuse.ca/big-number-calculator.htm
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 0x68906cc684438c21llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 0x8a103ede33e3d523llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 0xe42ca89707ea23aellu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 0xbc5658f0d63b5677llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 4), 0x19a0884094f1cda3llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 5), 0xc2159a8ff834288allu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 6), 0x95b89b36602306b1llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 7), 0x277a79937c8bbcb4llu);
    });

    TEST_CASE(multiply propagate unrolled12, {
            apint_setlimb(apint_test[0], 3, 0xffffffffffffffff);
            apint_setlimb(apint_test[0], 2, 0xffffffffffffffff);
            apint_setlimb(apint_test[0], 1, 0xffffffffffffffff);
            apint_setlimb(apint_test[0], 0, 0xffffffffffffffff);

            apint_setlimb(apint_test[1], 3, 0xffffffffffffffff);
            apint_setlimb(apint_test[1], 2, 0xffffffffffffffff);
            apint_setlimb(apint_test[1], 1, 0xffffffffffffffff);
            apint_setlimb(apint_test[1], 0, 0xffffffffffffffff);

            apint_mul_unroll12(apint_test[2], apint_test[0], apint_test[1]);

            // Calculated with: https://defuse.ca/big-number-calculator.htm
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 0), 0x0000000000000001llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 1), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 2), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 3), 0llu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 4), 0xfffffffffffffffellu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 5), 0xffffffffffffffffllu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 6), 0xffffffffffffffffllu);
            ASSERT_EQUAL_UL(apint_getlimb(apint_test[2], 7), 0xffffffffffffffffllu);
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
            // From arblib pi = 45471447111470790535029367847216232831674172166049053744846518889742361808273 * 2^-253
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

            apfp_mul(apfp_test[2], apfp_test[1], apfp_test[0]);

            // From arblib pi * pi = 35713191048373364904601842448597373027278304077961391237116093776389617195847 * 2^-251
            // But because of the alignment this needs to be adjusted (but if you print it you will see it's the same thing)
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 3), 0x9de9e64df22ef2d2llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 2), 0x56e26cd9808c1ac7llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 1), 0x8566a3fe0d0a228llu);
            ASSERT_EQUAL_UL(apint_getlimb(apfp_test[2]->mant, 0), 0x6682210253c7368ellu);

            ASSERT_EQUAL_L(apfp_test[2]->exp, -252l);
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

    TEST_CASE(add pi and 3.0, {
            apbar_set_midpt_mant(apbar_test[0], 1, 0xC000000000000000);
            apbar_set_midpt_exp(apbar_test[0], -126);
            apbar_set_rad(apbar_test[0], 0, 0);
            apbar_test[0]->midpt->mant->sign=1;

            apbar_set_midpt_mant(apbar_test[1], 1, 0xC90FDAA22168C234);
            apbar_set_midpt_mant(apbar_test[1], 0, 0xC4C6628B80DC1CD1);
            apbar_set_midpt_exp(apbar_test[1], -126);
            apbar_set_rad(apbar_test[1], 536870912, -156);
            apbar_test[1]->midpt->mant->sign=1;

            apbar_add(apbar_test[2], apbar_test[1], apbar_test[0], 128);
            apbar_print_msg("pi + 3 =", apbar_test[2]);

            // Expected value (128 bit): (32654307575434095910153190554018365901 * 2^-122) +/- (671088641 * 2^-154)
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 1), 0xc487ed5110b4611allu);
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 0), 0x62633145c06e0e68llu);
            ASSERT_EQUAL_L(apbar_get_midpt_exp(apbar_test[2]), -125l);

            // Unfortunately due to different methods of rounding we don't get the same result as arblib
            // We also cannot verify programmatically that this radius is within an error radius of arb
            // Please verify with this print:
            printf("Please verify rad is within limit: %llu * 2^%ld\n", apbar_test[2]->rad->mant, apbar_test[2]->rad->exp);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, 1);
    });

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

            printf("Please verify rad is within limit: %llu * 2^%ld\n", apbar_test[2]->rad->mant, apbar_test[2]->rad->exp);
            //ASSERT_EQUAL_UL(apbar_test[2]->rad->mant, 2llu);
            //ASSERT_EQUAL_L(apbar_test[2]->rad->exp, -126l);
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

            printf("Please verify rad is within limit: %llu * 2^%ld\n", apbar_test[2]->rad->mant, apbar_test[2]->rad->exp);
            //ASSERT_EQUAL_UL(apbar_test[2]->rad->mant, 2llu);
            //ASSERT_EQUAL_L(apbar_test[2]->rad->exp, -126l);
            ASSERT_EQUAL_I(apfp_test[2]->mant->sign, 1);
    });

//TODO: Ask if the difference of 1 bit in mantissa is okay.
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
            // (267257146016241686964920093290467695825 * 2^-126) +/- (805306371 * 2^-155)
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 1), 0xC90FDAA22168C234llu);
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 0), 0xC4C6628B80DC1CD2llu);
            ASSERT_EQUAL_L(apbar_get_midpt_exp(apbar_test[2]), -126l);

            printf("Please verify rad is within limit: %llu * 2^%ld\n", apbar_test[2]->rad->mant, apbar_test[2]->rad->exp);
            // ASSERT_EQUAL_UL(apbar_test[2]->rad->mant, 2llu);
            // ASSERT_EQUAL_L(apbar_test[2]->rad->exp, -126l);
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

            apbar_mul(apbar_test[2], apbar_test[1], apbar_test[0], 128);

            // apbar_print_msg("pi is:     ", apbar_test[0]);
            // apbar_print_msg("pi * pi is:", apbar_test[2]);

            // From arblib mid point needs to be: 9.86960440109
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 1), 0x9de9e64df22ef2d2llu);
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 0), 0x56e26cd9808c1ac6llu);
            ASSERT_EQUAL_L(apbar_get_midpt_exp(apbar_test[2]), -124l);

            // From arblib radius is: 958528343 * 2^-153
            printf("Please verify rad is within limit: %llu * 2^%ld\n", apbar_test[2]->rad->mant, apbar_test[2]->rad->exp);
            // ASSERT_EQUAL_UL(apbar_test[2]->rad->mant, 958528343llu);
            // ASSERT_EQUAL_L(apbar_test[2]->rad->exp, -153l);
    });

    TEST_CASE(mul pi with pi no exp, {
            apbar_set_midpt_mant(apbar_test[0], 0, 0xC4C6628B80DC1CD1);
            apbar_set_midpt_mant(apbar_test[0], 1, 0xC90FDAA22168C234);
            apbar_set_midpt_exp(apbar_test[0], -126);
            apbar_set_rad(apbar_test[0], 536870912, -156);

            apbar_set_midpt_mant(apbar_test[1], 0, 0xC4C6628B80DC1CD1);
            apbar_set_midpt_mant(apbar_test[1], 1, 0xC90FDAA22168C234);
            apbar_set_midpt_exp(apbar_test[1], -126);
            apbar_set_rad(apbar_test[1], 536870912, -156);

            apbar_mul_no_rad_exp(apbar_test[2], apbar_test[1], apbar_test[0], 128);

            // apbar_print_msg("pi is:     ", apbar_test[0]);
            // apbar_print_msg("pi * pi is:", apbar_test[2]);

            // From arblib mid point needs to be: 9.86960440109
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 1), 0x9de9e64df22ef2d2llu);
            ASSERT_EQUAL_UL(apbar_get_midpt_mant(apbar_test[2], 0), 0x56e26cd9808c1ac6llu);
            ASSERT_EQUAL_L(apbar_get_midpt_exp(apbar_test[2]), -124l);

            // From arblib radius is: 958528343 * 2^-153
            printf("Please verify rad is within limit: %llu * 2^%ld\n", apbar_test[2]->rad->mant, apbar_test[2]->rad->exp);
            // ASSERT_EQUAL_UL(apbar_test[2]->rad->mant, 958528343llu);
            // ASSERT_EQUAL_L(apbar_test[2]->rad->exp, -153l);
    });
});

apbar2_t apbar2_test[3];

static void apbar2_test_setup() {
    apbar2_init(apbar2_test[0], 256);
    apbar2_init(apbar2_test[1], 256);
    apbar2_init(apbar2_test[2], 256);
}

static void apbar2_test_teardown() {
    apbar2_free(apbar2_test[0]);
    apbar2_free(apbar2_test[1]);
    apbar2_free(apbar2_test[2]);
}

TEST_GROUP(apbar2, {
    WITH_SETUP(apbar2_test_setup);
    WITH_TEARDOWN(apbar2_test_teardown);

    TEST_CASE(apbar2: set to double, {
        apbar2_set_d(apbar2_test[0], 2.71828);

        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[0]), 1l);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[0], 3) >> 11, 0x15BF0995AAF790llu);
    });

    TEST_CASE(apbar2: add two numbers, {
        // Need to re-precision
        apbar2_free(apbar2_test[0]);
        apbar2_free(apbar2_test[1]);
        apbar2_free(apbar2_test[2]);

        apbar2_init(apbar2_test[0], 128);
        apbar2_init(apbar2_test[1], 128);
        apbar2_init(apbar2_test[2], 128);

        // 1 * 2^-127
        apbar2_set_midpt_limb(apbar2_test[0], 1, APBAR2_LIMB_MSBMASK);
        apbar2_set_midpt_exp(apbar2_test[0], -127);

        // 16703571626015105435307505830654230989 * 2^-249
        apbar2_set_midpt_limb(apbar2_test[1], 1, 0xc90fdaa22168c234);
        apbar2_set_midpt_limb(apbar2_test[1], 0, 0xc4c6628b80dc1cd0);
        apbar2_set_midpt_exp(apbar2_test[1], -126);

        apbar2_add(apbar2_test[2], apbar2_test[1], apbar2_test[0], 128);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 1), 0x8487ed5110b4611allu);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 0), 0x62633145c06e0e68llu);

        //Check value of exponent and sign
        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[2]), -125l);
    });

    TEST_CASE(apbar2: apbar2_add addition with positive numbers 1.000...00 X 2^10 + 1.000...00 X 2^10, {
        apbar2_set_midpt_limb(apbar2_test[0], 3, 0x8000000000000000LLU);
        apbar2_set_midpt_limb(apbar2_test[0], 2, 0);
        apbar2_set_midpt_limb(apbar2_test[0], 1, 0);
        apbar2_set_midpt_limb(apbar2_test[0], 0, 0);
        apbar2_set_midpt_exp(apbar2_test[0], 10);
        apbar2_set_sign(apbar2_test[0], 0);

        apbar2_set_midpt_limb(apbar2_test[1], 3, 0x8000000000000000LLU);
        apbar2_set_midpt_limb(apbar2_test[1], 2, 0);
        apbar2_set_midpt_limb(apbar2_test[1], 1, 0);
        apbar2_set_midpt_limb(apbar2_test[1], 0, 0);
        apbar2_set_midpt_exp(apbar2_test[1], 10);
        apbar2_set_sign(apbar2_test[1], 0);

        apbar2_add(apbar2_test[2], apbar2_test[0], apbar2_test[1], 256);

        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 3), 0x8000000000000000LLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 2), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 1), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 0), 0);

        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[2]), 11);
        ASSERT_EQUAL_I(apbar2_get_sign(apbar2_test[2]), 0);
    });

    TEST_CASE(apbar2: apbar2_add addition with positive numbers 1.111...11 X 2^10 + 1.111...11 X 2^10, {
        apbar2_set_midpt_limb(apbar2_test[0], 3, 0xffffffffffffffffLLU);
        apbar2_set_midpt_limb(apbar2_test[0], 2, 0xffffffffffffffffLLU);
        apbar2_set_midpt_limb(apbar2_test[0], 1, 0xffffffffffffffffLLU);
        apbar2_set_midpt_limb(apbar2_test[0], 0, 0xffffffffffffffffLLU);
        apbar2_set_midpt_exp(apbar2_test[0], 10);
        apbar2_set_sign(apbar2_test[0], 0);

        apbar2_set_midpt_limb(apbar2_test[1], 3, 0xffffffffffffffffLLU);
        apbar2_set_midpt_limb(apbar2_test[1], 2, 0xffffffffffffffffLLU);
        apbar2_set_midpt_limb(apbar2_test[1], 1, 0xffffffffffffffffLLU);
        apbar2_set_midpt_limb(apbar2_test[1], 0, 0xffffffffffffffffLLU);
        apbar2_set_midpt_exp(apbar2_test[1], 10);
        apbar2_set_sign(apbar2_test[1], 0);

        apbar2_add(apbar2_test[2], apbar2_test[0], apbar2_test[1], 256);

        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 3), 0xffffffffffffffffLLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 2), 0xffffffffffffffffLLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 1), 0xffffffffffffffffLLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 0), 0xffffffffffffffffLLU);

        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[2]), 11);
        ASSERT_EQUAL_I(apbar2_get_sign(apbar2_test[2]), 0);
    });

    TEST_CASE(apbar2: apbar2_add addition with positive numbers 1.000...01 X 2^256 + 1.000...00 X 2^1, {
        apbar2_set_midpt_limb(apbar2_test[0], 3, 0x8000000000000000LLU);
        apbar2_set_midpt_limb(apbar2_test[0], 2, 0);
        apbar2_set_midpt_limb(apbar2_test[0], 1, 0);
        apbar2_set_midpt_limb(apbar2_test[0], 0, 1);
        apbar2_set_midpt_exp(apbar2_test[0], 256);
        apbar2_set_sign(apbar2_test[0], 0);

        apbar2_set_midpt_limb(apbar2_test[1], 3, 0x8000000000000000LLU);
        apbar2_set_midpt_limb(apbar2_test[1], 2, 0);
        apbar2_set_midpt_limb(apbar2_test[1], 1, 0);
        apbar2_set_midpt_limb(apbar2_test[1], 0, 1);
        apbar2_set_midpt_exp(apbar2_test[1], 1);
        apbar2_set_sign(apbar2_test[1], 0);

        apbar2_add(apbar2_test[2], apbar2_test[0], apbar2_test[1], 256);

        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 3), 0x8000000000000000LLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 2), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 1), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 0), 2);

        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[2]), 256);
        ASSERT_EQUAL_I(apbar2_get_sign(apbar2_test[2]), 0);
    });

    TEST_CASE(apbar2: apbar2_add with positive and negative number 1.000...00 X 2^256 + -1.000...00 X 2^192, {
        apbar2_set_d(apbar2_test[0], 1.0);
        apbar2_set_midpt_exp(apbar2_test[0], 256);
        apbar2_set_sign(apbar2_test[0], 0);

        apbar2_set_d(apbar2_test[1], 1.0);
        apbar2_set_midpt_exp(apbar2_test[1], 192);
        apbar2_set_sign(apbar2_test[1], 1);

        apbar2_add(apbar2_test[2], apbar2_test[0], apbar2_test[1], 256);

        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 3), 0xffffffffffffffffLLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 2), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 1), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 0), 0);

        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[2]), 255);
        ASSERT_EQUAL_I(apbar2_get_sign(apbar2_test[2]), 0);
    });

    TEST_CASE(apbar2: subtraction with positive numbers a-b a>b, {
        apbar2_set_d(apbar2_test[0], 1.0);
        apbar2_set_midpt_exp(apbar2_test[0], 256);
        apbar2_set_sign(apbar2_test[0], 0);

        apbar2_set_d(apbar2_test[1], 1.0);
        apbar2_set_midpt_exp(apbar2_test[1], 192);
        apbar2_set_sign(apbar2_test[1], 0);

        apbar2_sub(apbar2_test[2], apbar2_test[0], apbar2_test[1], 256);

        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 3), 0xffffffffffffffffLLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 2), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 1), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 0), 0);

        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[2]), 255);
        ASSERT_EQUAL_I(apbar2_get_sign(apbar2_test[2]), 0);
    });

    TEST_CASE(apbar2: subtraction with positive numbers a-b a<b, {
        apbar2_set_d(apbar2_test[0], 1.0);
        apbar2_set_midpt_exp(apbar2_test[0], 192);
        apbar2_set_sign(apbar2_test[0], 0);

        apbar2_set_d(apbar2_test[1], 1.0);
        apbar2_set_midpt_exp(apbar2_test[1], 256);
        apbar2_set_sign(apbar2_test[1], 0);

        apbar2_sub(apbar2_test[2], apbar2_test[0], apbar2_test[1], 256);

        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 3), 0xffffffffffffffffLLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 2), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 1), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 0), 0);

        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[2]), 255);
        ASSERT_EQUAL_I(apbar2_get_sign(apbar2_test[2]), 1);
    });

    TEST_CASE(apbar2: subtraction with positive numbers a-b a>b 2pi - pi, {
        apbar2_set_midpt_limb(apbar2_test[0], 3, 0xC90FDAA22168C234LLU);
        apbar2_set_midpt_limb(apbar2_test[0], 2, 0xC4C6628B80DC1CD1LLU);
        apbar2_set_midpt_limb(apbar2_test[0], 1, 0);
        apbar2_set_midpt_limb(apbar2_test[0], 0, 0);
        apbar2_set_midpt_exp(apbar2_test[0], -125);
        apbar2_set_sign(apbar2_test[0], 0);

        apbar2_set_midpt_limb(apbar2_test[1], 3, 0xC90FDAA22168C234LLU);
        apbar2_set_midpt_limb(apbar2_test[1], 2, 0xC4C6628B80DC1CD1LLU);
        apbar2_set_midpt_limb(apbar2_test[1], 1, 0);
        apbar2_set_midpt_limb(apbar2_test[1], 0, 0);
        apbar2_set_midpt_exp(apbar2_test[1], -126);
        apbar2_set_sign(apbar2_test[1], 0);

        apbar2_sub(apbar2_test[2], apbar2_test[0], apbar2_test[1], 256);

        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 3), 0xC90FDAA22168C234LLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 2), 0xC4C6628B80DC1CD1LLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 1), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 0), 0);

        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[2]), -126);
        ASSERT_EQUAL_I(apbar2_get_sign(apbar2_test[2]), 0);
    });

    TEST_CASE(apbar2: subtraction with positive - negative number a-b a>b, {
        apbar2_set_midpt_limb(apbar2_test[0], 3, 0x8000000000000000LLU);
        apbar2_set_midpt_limb(apbar2_test[0], 2, 0);
        apbar2_set_midpt_limb(apbar2_test[0], 1, 0);
        apbar2_set_midpt_limb(apbar2_test[0], 0, 1);
        apbar2_set_midpt_exp(apbar2_test[0], 256);
        apbar2_set_sign(apbar2_test[0], 0);

        apbar2_set_midpt_limb(apbar2_test[1], 3, 0x8000000000000000LLU);
        apbar2_set_midpt_limb(apbar2_test[1], 2, 0);
        apbar2_set_midpt_limb(apbar2_test[1], 1, 0);
        apbar2_set_midpt_limb(apbar2_test[1], 0, 0);
        apbar2_set_midpt_exp(apbar2_test[1], 1);
        apbar2_set_sign(apbar2_test[1], 1);

        apbar2_sub(apbar2_test[2], apbar2_test[0], apbar2_test[1], 256);

        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 3), 0x8000000000000000LLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 2), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 1), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 0), 2);

        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[2]), 256);
        ASSERT_EQUAL_I(apbar2_get_sign(apbar2_test[2]), 0);
    });

    TEST_CASE(apbar2: subtraction with negative - positive number a-b a>b, {
        apbar2_set_midpt_limb(apbar2_test[0], 3, 0x8000000000000000LLU);
        apbar2_set_midpt_limb(apbar2_test[0], 2, 0);
        apbar2_set_midpt_limb(apbar2_test[0], 1, 0);
        apbar2_set_midpt_limb(apbar2_test[0], 0, 1);
        apbar2_set_midpt_exp(apbar2_test[0], 256);
        apbar2_set_sign(apbar2_test[0], 1);

        apbar2_set_midpt_limb(apbar2_test[1], 3, 0x8000000000000000LLU);
        apbar2_set_midpt_limb(apbar2_test[1], 2, 0);
        apbar2_set_midpt_limb(apbar2_test[1], 1, 0);
        apbar2_set_midpt_limb(apbar2_test[1], 0, 0);
        apbar2_set_midpt_exp(apbar2_test[1], 1);
        apbar2_set_sign(apbar2_test[1], 0);

        apbar2_sub(apbar2_test[2], apbar2_test[0], apbar2_test[1], 256);

        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 3), 0x8000000000000000LLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 2), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 1), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 0), 2);

        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[2]), 256);
        ASSERT_EQUAL_I(apbar2_get_sign(apbar2_test[2]), 1);
    });

    TEST_CASE(apbar2: subtract with differing signs and overflow, {
        apbar2_set_midpt_limb(apbar2_test[0], 3, 0xffffffffffffffffLLU);
        apbar2_set_midpt_exp(apbar2_test[0], 255);
        apbar2_set_sign(apbar2_test[0], 0);

        apbar2_set_midpt_limb(apbar2_test[1], 3, 0x8000000000000000LLU);
        apbar2_set_midpt_exp(apbar2_test[1], 192);
        apbar2_set_sign(apbar2_test[1], 1);

        apbar2_sub(apbar2_test[2], apbar2_test[0], apbar2_test[1], 256);

        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 3), 0x8000000000000000LLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 2), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 1), 0);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 0), 0);

        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[2]), 256);
        ASSERT_EQUAL_I(apbar2_get_sign(apbar2_test[2]), 0);
    });

    TEST_CASE(apbar2: pi * pi, {
        apbar2_set_midpt_limb(apbar2_test[0], 3, 0xc90fdaa22168c234LLU);
        apbar2_set_midpt_limb(apbar2_test[0], 2, 0xc4c6628b80dc1cd1LLU);
        apbar2_set_midpt_limb(apbar2_test[0], 1, 0x29024e088a67cc74LLU);
        apbar2_set_midpt_limb(apbar2_test[0], 0, 0x020bbea63b139b22LLU);
        apbar2_set_midpt_exp(apbar2_test[0], 1);
        apbar2_set_sign(apbar2_test[0], 0);

        apbar2_copy(apbar2_test[1], apbar2_test[0]);

        apbar2_mul(apbar2_test[2], apbar2_test[1], apbar2_test[0], 256);

        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 3), 0x9de9e64df22ef2d2LLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 2), 0x56e26cd9808c1ac7LLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 1), 0x08566a3fe0d0a228LLU);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[2], 0), 0x6682210253c7368eLLU);
        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[2]), 3);
    });
});


apbar2_t apbar2_add4_a[4];
apbar2_t apbar2_add4_b[4];
apbar2_t apbar2_add4_x[4];

static void apbar2_add4_test_setup() {
    for (size_t i = 0; i < 4; i++)
    {
        apbar2_init(apbar2_add4_a[i], 256);
        apbar2_init(apbar2_add4_b[i], 256);
        apbar2_init(apbar2_add4_x[i], 256);
    }
}

static void apbar2_add4_test_teardown() {
    for (size_t i = 0; i < 4; i++)
    {
        apbar2_free(apbar2_add4_a[i]);
        apbar2_free(apbar2_add4_b[i]);
        apbar2_free(apbar2_add4_x[i]);
    }
}

TEST_GROUP(apbar2_add4_test, {
    WITH_SETUP(apbar2_add4_test_setup);
    WITH_TEARDOWN(apbar2_add4_test_teardown);

    TEST_CASE(apbar2_add4: add two numbers, {
        // Need to re-precision
        for (size_t i = 0; i < 4; i++)
        {
            apbar2_free(apbar2_add4_a[i]);
            apbar2_free(apbar2_add4_b[i]);
            apbar2_free(apbar2_add4_x[i]);
        }

        for (size_t i = 0; i < 4; i++)
        {
            apbar2_init(apbar2_add4_a[i], 128);
            apbar2_init(apbar2_add4_b[i], 128);
            apbar2_init(apbar2_add4_x[i], 128);
        }

        for (size_t i = 0; i < 4; i++)
        {
            // 1 * 2^-127
            apbar2_set_midpt_limb(apbar2_add4_a[i], 1, APBAR2_LIMB_MSBMASK);
            apbar2_set_midpt_exp(apbar2_add4_a[i], -127);
        }

        for (size_t i = 0; i < 4; i++)
        {
            // 16703571626015105435307505830654230989 * 2^-249
            apbar2_set_midpt_limb(apbar2_add4_b[i], 1, 0xc90fdaa22168c234);
            apbar2_set_midpt_limb(apbar2_add4_b[i], 0, 0xc4c6628b80dc1cd0);
            apbar2_set_midpt_exp(apbar2_add4_b[i], -126);
        }

        apbar2_add4(apbar2_add4_x[0], apbar2_add4_a[0], apbar2_add4_b[0],
                    apbar2_add4_x[1], apbar2_add4_a[1], apbar2_add4_b[1],
                    apbar2_add4_x[2], apbar2_add4_a[2], apbar2_add4_b[2],
                    apbar2_add4_x[3], apbar2_add4_a[3], apbar2_add4_b[3],
                    128);

        for (size_t i = 0; i < 4; i++)
        {
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 1), 0x8487ed5110b4611allu);
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 0), 0x62633145c06e0e68llu);

            // Check value of exponent and sign
            ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_add4_x[i]), -125l);
        }
    });

    TEST_CASE(apbar2_add4: addition with positive numbers 1.000...00 X 2^10 + 1.000...00 X 2^10, {
        for (size_t i = 0; i < 4; i++)
        {
            apbar2_set_midpt_limb(apbar2_add4_a[i], 3, 0x8000000000000000LLU);
            apbar2_set_midpt_limb(apbar2_add4_a[i], 2, 0);
            apbar2_set_midpt_limb(apbar2_add4_a[i], 1, 0);
            apbar2_set_midpt_limb(apbar2_add4_a[i], 0, 0);
            apbar2_set_midpt_exp(apbar2_add4_a[i], 10);
            apbar2_set_sign(apbar2_add4_a[i], 0);
        }

        for (size_t i = 0; i < 4; i++)
        {
            apbar2_set_midpt_limb(apbar2_add4_b[i], 3, 0x8000000000000000LLU);
            apbar2_set_midpt_limb(apbar2_add4_b[i], 2, 0);
            apbar2_set_midpt_limb(apbar2_add4_b[i], 1, 0);
            apbar2_set_midpt_limb(apbar2_add4_b[i], 0, 0);
            apbar2_set_midpt_exp(apbar2_add4_b[i], 10);
            apbar2_set_sign(apbar2_add4_b[i], 0);
        }

        apbar2_add4(apbar2_add4_x[0], apbar2_add4_a[0], apbar2_add4_b[0],
                    apbar2_add4_x[1], apbar2_add4_a[1], apbar2_add4_b[1],
                    apbar2_add4_x[2], apbar2_add4_a[2], apbar2_add4_b[2],
                    apbar2_add4_x[3], apbar2_add4_a[3], apbar2_add4_b[3],
                    256);

        for (size_t i = 0; i < 4; i++)
        {
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 3), 0x8000000000000000LLU);
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 2), 0);
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 1), 0);
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 0), 0);

            ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_add4_x[i]), 11);
            ASSERT_EQUAL_I(apbar2_get_sign(apbar2_add4_x[i]), 0);
        }
    });

    TEST_CASE(apbar2_add4: addition with positive numbers 1.111...11 X 2^10 + 1.111...11 X 2^10, {
        for (size_t i = 0; i < 4; i++)
        {
            apbar2_set_midpt_limb(apbar2_add4_a[i], 3, 0xffffffffffffffffLLU);
            apbar2_set_midpt_limb(apbar2_add4_a[i], 2, 0xffffffffffffffffLLU);
            apbar2_set_midpt_limb(apbar2_add4_a[i], 1, 0xffffffffffffffffLLU);
            apbar2_set_midpt_limb(apbar2_add4_a[i], 0, 0xffffffffffffffffLLU);
            apbar2_set_midpt_exp(apbar2_add4_a[i], 10);
            apbar2_set_sign(apbar2_add4_a[i], 0);
        }

        for (size_t i = 0; i < 4; i++)
        {
            apbar2_set_midpt_limb(apbar2_add4_b[i], 3, 0xffffffffffffffffLLU);
            apbar2_set_midpt_limb(apbar2_add4_b[i], 2, 0xffffffffffffffffLLU);
            apbar2_set_midpt_limb(apbar2_add4_b[i], 1, 0xffffffffffffffffLLU);
            apbar2_set_midpt_limb(apbar2_add4_b[i], 0, 0xffffffffffffffffLLU);
            apbar2_set_midpt_exp(apbar2_add4_b[i], 10);
            apbar2_set_sign(apbar2_add4_b[i], 0);
        }

        apbar2_add4(apbar2_add4_x[0], apbar2_add4_a[0], apbar2_add4_b[0],
                    apbar2_add4_x[1], apbar2_add4_a[1], apbar2_add4_b[1],
                    apbar2_add4_x[2], apbar2_add4_a[2], apbar2_add4_b[2],
                    apbar2_add4_x[3], apbar2_add4_a[3], apbar2_add4_b[3],
                    256);

        for (size_t i = 0; i < 4; i++)
        {
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 3), 0xffffffffffffffffLLU);
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 2), 0xffffffffffffffffLLU);
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 1), 0xffffffffffffffffLLU);
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 0), 0xffffffffffffffffLLU);

            ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_add4_x[i]), 11);
            ASSERT_EQUAL_I(apbar2_get_sign(apbar2_add4_x[i]), 0);
        }
    });

    TEST_CASE(apbar2_add4: addition with positive numbers 1.000...01 X 2^256 + 1.000...00 X 2^1, {
        for (size_t i = 0; i < 4; i++)
        {
            apbar2_set_midpt_limb(apbar2_add4_a[i], 3, 0x8000000000000000LLU);
            apbar2_set_midpt_limb(apbar2_add4_a[i], 2, 0);
            apbar2_set_midpt_limb(apbar2_add4_a[i], 1, 0);
            apbar2_set_midpt_limb(apbar2_add4_a[i], 0, 1);
            apbar2_set_midpt_exp(apbar2_add4_a[i], 256);
            apbar2_set_sign(apbar2_add4_a[i], 0);
        }

        for (size_t i = 0; i < 4; i++)
        {
            apbar2_set_midpt_limb(apbar2_add4_b[i], 3, 0x8000000000000000LLU);
            apbar2_set_midpt_limb(apbar2_add4_b[i], 2, 0);
            apbar2_set_midpt_limb(apbar2_add4_b[i], 1, 0);
            apbar2_set_midpt_limb(apbar2_add4_b[i], 0, 1);
            apbar2_set_midpt_exp(apbar2_add4_b[i], 1);
            apbar2_set_sign(apbar2_add4_b[i], 0);
        }

        apbar2_add4(apbar2_add4_x[0], apbar2_add4_a[0], apbar2_add4_b[0],
                    apbar2_add4_x[1], apbar2_add4_a[1], apbar2_add4_b[1],
                    apbar2_add4_x[2], apbar2_add4_a[2], apbar2_add4_b[2],
                    apbar2_add4_x[3], apbar2_add4_a[3], apbar2_add4_b[3],
                    256);

        for (size_t i = 0; i < 4; i++)
        {
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 3), 0x8000000000000000LLU);
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 2), 0);
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 1), 0);
            ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_add4_x[i], 0), 2);

            ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_add4_x[i]), 256);
            ASSERT_EQUAL_I(apbar2_get_sign(apbar2_add4_x[i]), 0);
        }
    });
});

void run_test_suite() {
    RUN_TEST_GROUP(apint);
    RUN_TEST_GROUP(apfp);
    RUN_TEST_GROUP(ball_arithmetic);
    RUN_TEST_GROUP(apbar2);
    RUN_TEST_GROUP(apbar2_add4_test);
}

int main(int argc, char *argv[]) {
    run_test_suite();
}
