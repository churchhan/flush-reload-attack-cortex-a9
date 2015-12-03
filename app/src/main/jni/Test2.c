#include <stdlib.h>
#include <linux/perf_event.h>
#include <stdio.h>
#include "Args.h"
#include "ClearCache.h"
//method for timing a memory access (memory read) of addresses
void Test2(struct args_st *arguments, size_t *timings)
{
    size_t slot = 0;

    struct perf_event_attr pe;

    int perf_event_fd, addr;

    long long cpu_cycles;

    int i = 0;

    //init the perf_event_attr before calling the syscall
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_CPU_CYCLES; //we are going to count the number of CPU cycles
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;

    perf_event_fd = perf_event_open(&pe, 0, -1, -1, 0);
    if(perf_event_fd == -1)
    {
        fprintf(stderr, "Error opening leader %llx\n", pe.config);
        exit(EXIT_FAILURE);
    }


    //Check overhead
    printf("[+] Computing overhead\n");
    long long overhead = 0, tmp = 0;
    int N = 10000;

    for(i = 0; i < N; i++)
    {
        ioctl(perf_event_fd, PERF_EVENT_IOC_RESET, 0);
        ioctl(perf_event_fd, PERF_EVENT_IOC_ENABLE, 0);
        ioctl(perf_event_fd, PERF_EVENT_IOC_DISABLE, 0);
        read(perf_event_fd, &cpu_cycles, sizeof(long long));
        overhead += cpu_cycles;
    }

    overhead /= N;

    //Start probing addresses of the shared library from base_address to end_address
    //in order to test the timing variations
    char *probe;
    i = 0;
    printf("[+] Probing memread\n");
    sched_yield();
    for(probe = arguments->base_address; probe < arguments->end_address; probe += arguments->stride)
    {
        clearcache(arguments->base_address, arguments->end_address);
        //start counting the cpu cycles
        ioctl(perf_event_fd, PERF_EVENT_IOC_RESET, 0);
        ioctl(perf_event_fd, PERF_EVENT_IOC_ENABLE, 0);

        //do a memory read
        memread(probe);

        //stop counting the cpu cycles
        ioctl(perf_event_fd, PERF_EVENT_IOC_DISABLE, 0);

        //read the cpu cycles counter
        read(perf_event_fd, &cpu_cycles, sizeof(long long));

        //store it
        timings[i++] = cpu_cycles - overhead;
    }
    sched_yield();
}
