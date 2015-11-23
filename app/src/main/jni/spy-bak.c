//
// Created by tiana on 11/18/15.
//

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

#define MAX_NUM_OF_ADDRS 10
#define TIME_SLOTS 50000


void memread(char *address);
size_t *timings;

struct args_st {
    /** File descriptor  */
    int bin_fd;

    /** FILE stream of file containing addresses to probe */
    //FILE *addr_file;

    /** FILE stream of output file containing time slot data */
    FILE *out_file;

    size_t map_size;

    /** Number of cycles for the busy wait */
    int busy_cycles;

    size_t target_size;

    char *base_address;

    char *end_address;
};



typedef struct {
    unsigned long result[MAX_NUM_OF_ADDRS];
} time_slot;


//http://linux.die.net/man/2/perf_event_open
long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                  group_fd, flags);
    return ret;
}


void cleanup_args(struct args_st *arguments) {
    close(arguments->bin_fd);
    //fclose(arguments->addr_file);
    fclose(arguments->out_file);
}


bool read_args(struct args_st *arguments, int argc, char *argv[]) {


    /*if (argc != 5) {
        fprintf(stderr, "Usage: %s <binary> <addr> <out> <cycles>\n", argv[0]);
        goto fail;
    }*/

    if (argc != 3) {
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


   /* char *addr_path = argv[2];
    if ((arguments->addr_file = fopen(addr_path, "r")) == NULL) {
        perror("error fopen addr_path");
        goto addr_fail;
    }*/

    //char *out_path = argv[3];
    char *out_path = argv[2];
    if ((arguments->out_file = fopen(out_path, "w+")) == NULL) {
        perror("Error fopen out_path");
        goto out_fail;
    }

    //char *cycles = argv[4];
    /*char *endptr = NULL;

    arguments->busy_cycles = strtol(cycles, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid cycles argument %s\n", cycles);
        goto cycles_fail;
    }*/

    return true;

    cycles_fail:
        fclose(arguments->out_file);
    out_fail:
        fclose(arguments->bin_fd);
        //fclose(arguments->addr_file);
    /*addr_fail:
        close(arguments->bin_fd);*/
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





/*void spy(char **addrs, size_t num_addrs, time_slot *slots, size_t num_slots, int busy_cycles, struct args_st *arguments)
{
    size_t slot = 0;

    struct perf_event_attr pe;

    int perf_event_fd, addr;

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;

    perf_event_fd = perf_event_open(&pe, 0, -1, -1, 0);
    if(perf_event_fd == -1)
    {
        fprintf(stderr, "Error opening leader %llx\n", pe.config);
        exit(EXIT_FAILURE);
    }


    for(slot = 0; slot < num_slots; slot++)
    {
        for(addr = 0; addr < (int) num_addrs; addr++)
        {
            char *ptr = addrs[addr];
        }
    }

    char *probe;

    for(probe = arguments->base_address; probe < arguments->end_address; probe += 64)
    {
        sched_yield();
        memread(probe);
        sched_yield();
    }
}*/

void spy(struct args_st *arguments, size_t *timings)
{
    size_t slot = 0;

    struct perf_event_attr pe;

    int perf_event_fd, addr;

    long long cpu_cycles;

    int i = 0;
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_CPU_CYCLES;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;

    perf_event_fd = perf_event_open(&pe, 0, -1, -1, 0);
    if(perf_event_fd == -1)
    {
        fprintf(stderr, "Error opening leader %llx\n", pe.config);
        exit(EXIT_FAILURE);
    }

    char *probe;

    sched_yield();
    for(probe = arguments->base_address; probe < arguments->end_address; probe += 64)
    {
        ioctl(perf_event_fd, PERF_EVENT_IOC_RESET, 0);
        ioctl(perf_event_fd, PERF_EVENT_IOC_ENABLE, 0);
        memread(probe);
        ioctl(perf_event_fd, PERF_EVENT_IOC_DISABLE, 0);
        read(perf_event_fd, &cpu_cycles, sizeof(long long));
        timings[i++] = cpu_cycles;
    }
    sched_yield();
}




int main(int argc, char *argv[])
{
    void *target_base_addr;
    char *addresses[MAX_NUM_OF_ADDRS];
    struct args_st arguments;
    size_t i = 0;

    if(!read_args(&arguments, argc, argv))
    {
        return 1;
    }


    //mmap so we can force the OS to share this memory page with the targeted process
    arguments.base_address = mmap(NULL, arguments.target_size, PROT_READ, MAP_FILE | MAP_SHARED, arguments.bin_fd, 0);
    arguments.end_address = arguments.base_address + arguments.target_size;

    if(target_base_addr == MAP_FAILED)
    {
        perror("Error while mmaping");
        return 1;
    }

    printf("[+] Target binary mmpaped to %p\n", arguments.base_address);

    /*size_t num_addresses = read_addrs(arguments.addr_file, addresses, MAX_NUM_OF_ADDRS);


    //List of addresses to test must not be empty
    assert(num_addresses != 0);

    printf("[+] Probing %lu addresses:\n", (long unsigned int)num_addresses);

    for(i = 0; i < num_addresses; i++)
    {
        printf("\t - %p\n", addresses[i]);
    }

    //Adjust addresses offset
    adjust_addresses_offset(target_base_addr, addresses, num_addresses);

    printf("[+] Adjusting addresses offset...\n");


    for(i = 0; i < num_addresses; i++)
    {
        printf("\t - %p\n", addresses[i]);
    }*/

    //Start spying

    //time_slot slots[TIME_SLOTS];

    //size_t pagesize = sysconf(_SC_PAGE_SIZE);

    //spy(addresses, num_addresses, slots, TIME_SLOTS, arguments.busy_cycles, &arguments);
    timings = malloc(((arguments.end_address - arguments.base_address)/64) * sizeof(size_t*));
    spy(&arguments, timings);

    printf("[+] Finished spying...\n");

    char *probe;
    i = 0;
    for(probe = arguments.base_address; probe < arguments.end_address; probe += 64) {
        printf("%lld\n", (long long)timings[i++]);
    }

    munmap(target_base_addr, arguments.target_size);
    cleanup_args(&arguments);
    return 0;

}