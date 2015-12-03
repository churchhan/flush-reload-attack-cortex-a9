
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include "Args.h"
#include "Test1.h"

#define MAX_NUM_OF_ADDRS 10
#define TIME_SLOTS 50000


void memread(char *address);
size_t *timings;



typedef struct {
    unsigned long result[MAX_NUM_OF_ADDRS];
} time_slot;

//Use perf_event_open syscall for getting a timing from the userspace
//http://linux.die.net/man/2/perf_event_open
long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                  group_fd, flags);
    return ret;
}


//Method used for cleaning up stuff while exiting the program
void cleanup_args(struct args_st *arguments) {
    close(arguments->bin_fd);
    fclose(arguments->out_file);
}


//Method for parsing arguments that are given to the program
bool read_args(struct args_st *arguments, int argc, char *argv[]) {


    if (argc != 2) {
        fprintf(stderr, "Usage: %s <binary> <out>\n", argv[0]);
        goto fail;
    }

    char *binaryPath = argv[1];
    if ((arguments->bin_fd = open(binaryPath, O_RDONLY)) == -1) {
        perror("error opening binary path");
        goto fail;
    }

    size_t size = lseek(arguments->bin_fd, 0, SEEK_END);
    if(size == 0)
    {
       perror("bin empty");
       goto fail;
    }

    arguments->target_size = size;

    printf("%d %d\n", size, arguments->target_size);

    if(arguments->target_size == 0)
    {
        perror("file empty");
        goto fail;
    }

    arguments->map_size = arguments->target_size;

    if(arguments->map_size & 0xFFF != 0)
    {
        arguments->map_size |= 0xFFF;
        arguments->map_size += 1;
    }

    /*//char *out_path = argv[3];*/
    /*char *out_path = argv[2];*/
    /*if ((arguments->out_file = fopen(out_path, "w+")) == NULL) {*/
        /*perror("Error fopen out_path");*/
        /*goto out_fail;*/
    /*}*/

    return true;

    cycles_fail:
        fclose(arguments->out_file);
    out_fail:
        fclose(arguments->bin_fd);
    fail:
        return false;
}

size_t read_addrs(FILE *addr_file, char **addrs, size_t maxlen) {
    size_t linecap = 24;
    ssize_t linelen = 0;
    size_t i = -1;
    size_t num_addrs = 0;

    char *line = malloc(256 * sizeof(char));
    char *endptr;

    while(line = fgetln(addr_file, &linelen))
    {
        line[linelen - 1] = '\0';

        char *addr = (char *)strtol(&(line[2]), &endptr, 16);
        if(*endptr != '\0')
        {
            fprintf(stderr, "[+] Error parsing address %s\n", line);
            return 0;
        }

        addrs[++i] = addr;
        ++num_addrs;
    }

    if(!feof(addr_file))
    {
        fprintf(stderr, "[+] Error reading addr_file: %p\n", addr_file);
        free(line);
        return 0;
    }

    free(line);
    return num_addrs;
}


void adjust_addresses_offset(void *target_base_address, char **addrs, size_t num_addrs)
{
    size_t i;
    for(i = 0; i < num_addrs; i++)
    {
        unsigned long ptr_offset = (unsigned long)target_base_address;
        char *adjusted_ptr = addrs[i] + ptr_offset;

        addrs[i] = adjusted_ptr;
    }
}


void spy(struct args_st *arguments, size_t *timings)
{
    Test1(arguments, timings);
}



int main(int argc, char *argv[])
{
    void *target_base_addr;
    char *addresses[MAX_NUM_OF_ADDRS];
    struct args_st arguments;
    size_t i = 0;

    //parse args
    if(!read_args(&arguments, argc, argv))
    {
        return 1;
    }


    //mmap so we can force the OS to share this memory page with the targeted/attacked shared
    //library
    arguments.base_address = mmap(NULL, arguments.target_size, PROT_READ, MAP_FILE | MAP_SHARED, arguments.bin_fd, 0);
    arguments.end_address = arguments.base_address + arguments.target_size;

    if(target_base_addr == MAP_FAILED)
    {
        perror("Error while mmaping");
        return 1;
    }

    printf("[+] Target binary mmpaped to %p\n", arguments.base_address);

    //timings is a data structure in which the timings will be stored before to be displayed out at
    //the end of the spying step
    timings = malloc(((arguments.end_address - arguments.base_address)/64) * sizeof(size_t*));


    //run the spying step
    spy(&arguments, timings);

    printf("[+] Finished spying...\n");

    //Display timings/results
    char *probe;
    i = 0;
    for(probe = arguments.base_address; probe < arguments.end_address; probe += 64) {
        printf("%lld\n", (long long)timings[i++]);
    }

    munmap(target_base_addr, arguments.target_size);
    cleanup_args(&arguments);
    return 0;

}
