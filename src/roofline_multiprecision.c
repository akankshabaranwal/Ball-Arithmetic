#include <apbar2.h>

static const int NUM_RUNS = 1000000;
static const int PRECISION = 64;

void run_apbar2_add_p64(apbar2_t in1, apbar2_t in2, apbar2_t out)
{
    // Create separate functions for each precision so that they can be accordingly called.
    for(int i=0; i<NUM_RUNS; i++)
    {
        apbar2_add(out,in1,in2,64);
        apbar2_add(out,in1,in2,64);
        apbar2_add(out,in1,in2,64);
        apbar2_add(out,in1,in2,64);
    }
}

void run_apbar2_add_p512(apbar2_t in1, apbar2_t in2, apbar2_t out)
{
    // Create separate functions for each precision so that they can be accordingly called.
    for(int i=0; i<NUM_RUNS; i++)
    {
        apbar2_add(out,in1,in2,512);
        apbar2_add(out,in1,in2,512);
        apbar2_add(out,in1,in2,512);
        apbar2_add(out,in1,in2,512);
    }
}

void run_apbar2_add_p4096(apbar2_t in1, apbar2_t in2, apbar2_t out)
{
    // Create separate functions for each precision so that they can be accordingly called.
    for(int i=0; i<NUM_RUNS; i++)
    {
        apbar2_add(out,in1,in2,4096);
        apbar2_add(out,in1,in2,4096);
        apbar2_add(out,in1,in2,4096);
        apbar2_add(out,in1,in2,4096);
    }
}

void run_apbar2_add_p32768(apbar2_t in1, apbar2_t in2, apbar2_t out)
{
    // Create separate functions for each precision so that they can be accordingly called.
    for(int i=0; i<NUM_RUNS; i++)
    {
        apbar2_add(out,in1,in2,32768);
        apbar2_add(out,in1,in2,32768);
        apbar2_add(out,in1,in2,32768);
        apbar2_add(out,in1,in2,32768);
    }
}

int main()
{
    //Declare multiple precision apbar2 datatypes
    apbar2_t test_p64_in1, test_p64_in2, test_p64_out;
    apbar2_t test_p512_in1, test_p512_in2, test_p512_out;
    apbar2_t test_p4096_in1, test_p4096_in2, test_p4096_out;
    apbar2_t test_p32768_in1, test_p32768_in2, test_p32768_out;

    apbar2_init(test_p64_in1,64);
    apbar2_init(test_p64_in2,64);
    apbar2_init(test_p64_out,64);

    apbar2_init(test_p512_in1, 512);
    apbar2_init(test_p512_in2, 512);
    apbar2_init(test_p512_out, 512);

    apbar2_init(test_p4096_in1, 4096);
    apbar2_init(test_p4096_in2, 4096);
    apbar2_init(test_p4096_out, 4096);

    apbar2_init(test_p32768_in1, 32768);
    apbar2_init(test_p32768_in2, 32768);
    apbar2_init(test_p32768_out, 32768);

    // Initialize apbar2 datatypes
    apbar2_set_d(test_p64_in1, (double)rand() / RAND_MAX);
    apbar2_set_d(test_p64_in2, (double)rand() / RAND_MAX);
    apbar2_set_d(test_p512_in1, (double)rand() / RAND_MAX);
    apbar2_set_d(test_p512_in2, (double)rand() / RAND_MAX);
    apbar2_set_d(test_p4096_in1, (double)rand() / RAND_MAX);
    apbar2_set_d(test_p4096_in2, (double)rand() / RAND_MAX);
    apbar2_set_d(test_p32768_in1, (double)rand() / RAND_MAX);
    apbar2_set_d(test_p32768_in2, (double)rand() / RAND_MAX);

    // Call code for roofline plots
    run_apbar2_add_p64(test_p64_in1, test_p64_in2, test_p64_out);
    run_apbar2_add_p512(test_p512_in1, test_p512_in2, test_p512_out);
    run_apbar2_add_p4096(test_p4096_in1, test_p4096_in2, test_p4096_out);
    run_apbar2_add_p32768(test_p32768_in1, test_p32768_in2, test_p32768_out);


    // Clean up memory
    apbar2_free(test_p64_in1);
    apbar2_free(test_p64_in2);
    apbar2_free(test_p64_out);

    apbar2_free(test_p512_in1);
    apbar2_free(test_p512_in2);
    apbar2_free(test_p512_out);

    apbar2_free(test_p4096_in1);
    apbar2_free(test_p4096_in2);
    apbar2_free(test_p4096_out);

    apbar2_free(test_p32768_in1);
    apbar2_free(test_p32768_in2);
    apbar2_free(test_p32768_out);

    return 0;
}
