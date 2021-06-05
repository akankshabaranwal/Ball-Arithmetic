#include <test.h>
#include <apbar2.h>

apbar2_t apbar2_test[3];

static void fp_test_setup() {
    apbar2_init(apbar2_test[0], 256);
    apbar2_init(apbar2_test[1], 256);
    apbar2_init(apbar2_test[2], 256);
}

static void fp_test_teardown() {
    apbar2_free(apbar2_test[0]);
    apbar2_free(apbar2_test[1]);
    apbar2_free(apbar2_test[2]);
}

TEST_GROUP(floating_point, {
    WITH_SETUP(fp_test_setup);
    WITH_TEARDOWN(fp_test_teardown);

    TEST_CASE(set to double, {
        apbar2_set_d(apbar2_test[0], 2.71828);

        ASSERT_EQUAL_L(apbar2_get_midpt_exp(apbar2_test[0]), 1l);
        ASSERT_EQUAL_UL(apbar2_get_midpt_limb(apbar2_test[0], 3) >> 11, 0x15BF0995AAF790llu);
    });

    TEST_CASE(add two numbers, {
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

    // To-do: Import and convert unit tests to new "collapsed" API.
});

void run_test_suite() {
    RUN_TEST_GROUP(floating_point);
}

int main(int argc, char *argv[]) {
    run_test_suite();
}
