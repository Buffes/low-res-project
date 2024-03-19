#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../el_random.h"

// prints an array of ints as a histogram, for showing e.g.
// distributions of random generated numbers.
void print_histogram(int *buckets, int num_buckets) {
    int bars[num_buckets];
    int max = 0;
    for (int bucket = 0; bucket < num_buckets; bucket++) {
       if (buckets[bucket] > max) max = buckets[bucket]; 
    }

    for (int bucket = 0; bucket < num_buckets; bucket++) {
        bars[bucket] = 30 * buckets[bucket] / max;
    }
    
    for (int bar = 0; bar < num_buckets; bar++) {
        printf("|");
        for (int i = 0; i < bars[bar]; i++) 
            printf("#");
        printf("\n");
    }
}

void test_random_int32(uint32_t state) { 
    int out_of_bounds = 0;
    int num_buckets = 20;
    int buckets[num_buckets];
    int tests = 10000;
    int counter = 0;
    printf("random s32, seed %u\n", state);
    for (uint64_t i = 0; i < tests; i++) {
        int32_t min = -50;
        int32_t max = 500;
        int32_t random_val = random_s32(&state, min, max);
        if (random_val < min || random_val > max) {
            counter++;
        }
        // find the bucket
        float span = max - min;
        float spot = (float)(random_val - min) / span; // 0 to 1
        int bucket = (int)(spot * num_buckets); // 0 to num_buckets
        buckets[bucket]++;
        printf("  %i\n", random_val);  
    }
    printf("out of bounds %i\n", counter);
    print_histogram(buckets, num_buckets);
}

void test_random_float32(uint32_t state) { 
    int out_of_bounds = 0;
    int num_buckets = 20;
    int buckets[num_buckets];
    int tests = 1000;
    printf("random f32, seed %u\n", state);
    for (int i = 0; i < tests; i++) {
        float min = -7839.3f;
        float max = -3004.4f;
        float random_val = random_f32(&state, min, max);
        if (random_val < min || random_val > max) {
            out_of_bounds++;
        }
        // find the bucket
        float span = max - min;
        float spot = (random_val - min) / span; // 0 to 1
        int bucket = (int)(spot * num_buckets); // 0 to num_buckets
        buckets[bucket]++;
        printf("  %f\n", random_val);
    }
    printf("out of bounds %i\n", out_of_bounds);
    print_histogram(buckets, num_buckets);
}

int main(int argc, char* argv[]) {
    uint32_t state = 1;
    if (argc > 1) {
        state = (uint32_t)strtoul(argv[1], NULL, 10); 
    } 

    test_random_int32(state);
    
}

