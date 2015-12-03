#ifndef ARGS_H
#define ARGS_H
//Structure for saving arguments passed to the program
struct args_st {
    /** File descriptor  */
    int bin_fd;

    /** FILE stream of output file containing time slot data */
    FILE *out_file;

    //map size used while mmaping
    size_t map_size;

    /** Number of cycles for the busy wait */
    int busy_cycles;

    //size of the target shared library to attack
    size_t target_size;

    //base address returned by mmap
    char *base_address;

    char *end_address;

    //The test to run
    int test;

    //number of stride while probing from base_address to end_address 
    int stride;
};
#endif
