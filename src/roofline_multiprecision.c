#include <apbar2.h>

static const int NUM_RUNS = 10000000;
static const int PRECISION = 64;

int main()
{

    //Declare multiple precision apbar2 datatypes
    apbar2_t test_p64, test_p512, test_p4096, test_p32768;

    apbar2_init(test_p64,64);
    apbar2_init(test_p512, 512);
    apbar2_init(test_p4096, 4096);
    apbar2_init(test_p32768, 32768);

    // Initialize apbar2 datatypes


    // Create separate functions for each precision so that they can be accordingly called.
    for(int i=0; i<NUM_RUNS; i++)
    {

    }

    apbar2_free(test_p64);
    apbar2_free(test_p512);
    apbar2_free(test_p4096);
    apbar2_free(test_p32768);

    return 0;
}
