/*
 * Copyright 2016, Victor van der Veen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <map>
#include <string>
#include <numeric>
#include <sstream>
#include <vector>

#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/ion.h>

#include "helper.h"

#define HAMMER_READCOUNT 1000000
#define MAX_NOP 1
#define REPEAT 1
#define AREA 4096 // kB
#define ROW_NUMBERS 2 * 1024 / 64
#define ROW_SIZE 64 * 1024
#define BANK_SIZE 8 * 1024

#define TESTAREA 128

void rowsize(char* mapping) {
    char* h1, * h2;
    volatile char* th1, *th2;
    unsigned long t0 = 0;
    unsigned long t1 = 0;
    double t_all[TESTAREA*TESTAREA];
    for (int i = 0; i < TESTAREA; i++) {
        h1 = mapping + PAGESIZE * i / 4;
        th1 = h1;
        for (int j = 0; j < TESTAREA; j++) {
            h2 = mapping + PAGESIZE * j / 4;
            th2 = h2;
            t0 = get_ns();
            for (int k = 0; k < 100000; k++) {
                *th1;
                *th2;
            }
            t1 = get_ns();
            t_all[i*TESTAREA+j] = (t1-t0) / 100000.0 / 2;
        }
    }
    for (int i = 0; i < TESTAREA * TESTAREA; i++) {
            // printf("@@@ time-at %d: %lu\n", i, t_all[i]);
        printf("%lf ", t_all[i]);
    }
    printf("\n");
}

void doublesided_nexus5(char* mapping, int direction) {
    char pa;
    char pv;

    char* h1, * h2, * v;
    volatile char* th1, *th2;
    std::vector<int> flipped;
    flipped.reserve(65536);

    if (direction == 1) {
        pv = 0xFF;
        pa = 0x00;
    } else {
        pv = 0x00;
        pa = 0xFF;
    }
    

    for (int nops = 0; nops < MAX_NOP; nops += 2) {
        int sum = 0;
        for (int k = 0; k < REPEAT; k++) {
            for (int offset = 0; offset < ROW_NUMBERS - 2; offset++) {
                h1 = mapping + ROW_SIZE * offset;
                v = h1 + ROW_SIZE;
                h2 = v + ROW_SIZE;
                memset(mapping, pa, ROW_NUMBERS * ROW_SIZE);
                memset( v, pv, ROW_SIZE );
                for (int bank_offset = 0; bank_offset < ROW_SIZE; bank_offset += BANK_SIZE) {
                    th1 = h1 + bank_offset;
                    th2 = h2 + bank_offset;
                    for (int i = 0; i < HAMMER_READCOUNT; i++) {
                        *th1;
                        *th2;
                    }
                }
                for (int i = 0; i < ROW_SIZE; i++) {
                    if (v[i] != pv) {
                        flipped.push_back(i);
                        sum += 1;
                    }
                }
                memset(mapping, pa, ROW_NUMBERS * ROW_SIZE);
                memset( v, pv, ROW_SIZE );
                for (int bank_offset = 0; bank_offset < ROW_SIZE; bank_offset += BANK_SIZE) {
                    th1 = h1 + bank_offset + 1024;
                    th2 = h2 + bank_offset + 1024;
                    for (int i = 0; i < HAMMER_READCOUNT; i++) {
                        *th1;
                        *th2;
                    }
                }
                for (int i = 0; i < ROW_SIZE; i++) {
                    if (v[i] != pv) {
                        flipped.push_back(i);
                        sum += 1;
                    }
                }
            }
        }
        printf("\n");
        for (auto id: flipped) {
            printf("%d ", id);
        }
        printf("\n");
        printf("@@@ Flipped: %d\n", sum / REPEAT);
    }
}

void doublesided(char* mapping, int direction) {
    char pa;
    char pv;

    char* h1, * h2, * v;
    volatile char* th1, *th2;
    unsigned long t0 = 0;
    unsigned long t1 = 0;
    unsigned long t_all[ROW_NUMBERS];
    std::vector<int> flipped;
    flipped.reserve(65536);

    if (direction == 1) {
        pv = 0xFF;
        pa = 0x00;
    } else {
        pv = 0x00;
        pa = 0xFF;
    }
    

    for (int nops = 0; nops < MAX_NOP; nops += 2) {
        int sum = 0;
        for (int k = 0; k < REPEAT; k++) {
            for (int offset = 0; offset < ROW_NUMBERS - 2; offset++) {
                h1 = mapping + ROW_SIZE * offset;
                v = h1 + ROW_SIZE;
                h2 = v + ROW_SIZE;
                memset(mapping, pa, ROW_NUMBERS * ROW_SIZE);
                memset( v, pv, ROW_SIZE );
                for (int bank_offset = 0; bank_offset < ROW_SIZE; bank_offset += BANK_SIZE) {
                    th1 = h1 + bank_offset;
                    th2 = h2 + bank_offset;
                    t0 = get_ns();
                    for (int i = 0; i < HAMMER_READCOUNT; i++) {
                        *th1;
                        *th2;
                    }
                    t1 = get_ns();
                    t_all[offset] = (t1-t0) / HAMMER_READCOUNT / 2;
                }
                for (int i = 0; i < ROW_SIZE; i++) {
                    if (v[i] != pv) {
                        flipped.push_back(i);
                        sum += 1;
                    }
                }
            }
        }
        for (int i = 0; i < 16; i++) {
            printf("%lu ", t_all[i]);
        }
        printf("\n");
        for (auto id: flipped) {
            printf("%d ", id);
        }
        printf("\n");
        printf("@@@ Flipped: %d\n", sum / REPEAT);
    }
}

void singlesided(char* mapping, int direction) {
    char pa;
    char pv;

    char* h1, * h2, * v1, * v2;
    volatile char* th1, *th2;
    std::vector<int> flipped;
    flipped.reserve(65536);

    if (direction == 1) {
        pv = 0xFF;
        pa = 0x00;
    } else {
        pv = 0x00;
        pa = 0xFF;
    }
    

    for (int nops = 0; nops < MAX_NOP; nops += 2) {
        int sum = 0;
        for (int k = 0; k < REPEAT; k++) {
            for (int offset = 1; offset < ROW_NUMBERS - 6; offset++) {
                h1 = mapping + ROW_SIZE * offset;
                v1 = h1 - ROW_SIZE;
                v2 = h1 + ROW_SIZE;
                h2 = h1 + ROW_SIZE * 5;
                memset(mapping, pa, ROW_NUMBERS * ROW_SIZE);
                memset( v1, pv, ROW_SIZE );
                memset( v2, pv, ROW_SIZE );
                for (int bank_offset = 0; bank_offset < ROW_SIZE; bank_offset += BANK_SIZE) {
                    th1 = h1 + bank_offset;
                    th2 = h2 + bank_offset;
                    for (int i = 0; i < HAMMER_READCOUNT; i++) {
                        *th1;
                        *th2;
                    }
                }
                for (int i = 0; i < ROW_SIZE; i++) {
                    if (v1[i] != pv) {
                        flipped.push_back(i);
                        sum += 1;
                    }
                    if (v2[i] != pv) {
                        flipped.push_back(i);
                        sum += 1;
                    }
                }
                memset(mapping, pa, ROW_NUMBERS * ROW_SIZE);
                memset( v1, pv, ROW_SIZE );
                memset( v2, pv, ROW_SIZE );
                for (int bank_offset = 0; bank_offset < ROW_SIZE; bank_offset += BANK_SIZE) {
                    th1 = h1 + bank_offset + 1024;
                    th2 = h2 + bank_offset + 1024;
                    for (int i = 0; i < HAMMER_READCOUNT; i++) {
                        *th1;
                        *th2;
                    }
                }
                for (int i = 0; i < ROW_SIZE; i++) {
                    if (v1[i] != pv) {
                        flipped.push_back(i);
                        sum += 1;
                    }
                    if (v2[i] != pv) {
                        flipped.push_back(i);
                        sum += 1;
                    }
                }
            }
        }
        printf("\n");
        for (auto id: flipped) {
            printf("%d ", id);
        }
        printf("\n");
        printf("@@@ Flipped: %d\n", sum / REPEAT);
    }
}

void manysided(char* mapping, int direction) {
    char pa;
    char pv;

    char* h1, * h2, * h3, * h4, *h5, *h6, *h7;
    char* v1, * v2, * v3, * v4;
    volatile char* th1, *th2, * th3, *th4, *th5, *th6, *th7;
    unsigned long t0 = 0;
    unsigned long t1 = 0;
    unsigned long t_all[ROW_NUMBERS];
    std::vector<int> flipped;
    flipped.reserve(65536);

    if (direction == 1) {
        pv = 0xFF;
        pa = 0x00;
    } else {
        pv = 0x00;
        pa = 0xFF;
    }

    for (int nops = 0; nops < MAX_NOP; nops += 2) {
        int sum = 0;
        for (int k = 0; k < REPEAT; k++) {
            for (int offset = 0; offset < ROW_NUMBERS - 12; offset++) {
                h1 = mapping + ROW_SIZE * offset;
                v1 = h1 + ROW_SIZE;
                h2 = v1 + ROW_SIZE;
                v2 = h2 + ROW_SIZE;
                h3 = h2 + ROW_SIZE * 2;
                v3 = h3 + ROW_SIZE;
                h4 = h3 + ROW_SIZE * 2;
                v4 = h4 + ROW_SIZE;
                h5 = h4 + ROW_SIZE * 2;
                h6 = h5 + ROW_SIZE * 2;
                h7 = h6 + ROW_SIZE * 2;
                memset(mapping, pa, ROW_NUMBERS * ROW_SIZE);
                // memset( h1, pa, ROW_SIZE );
                memset( v1, pv, ROW_SIZE );
                memset( v2, pv, ROW_SIZE );
                memset( v3, pv, ROW_SIZE );
                memset( v4, pv, ROW_SIZE );
                // memset( h2, pa, ROW_SIZE );
                for (int bank_offset = 0; bank_offset < ROW_SIZE; bank_offset += BANK_SIZE) {
                    th1 = h1 + bank_offset;
                    th2 = h2 + bank_offset+1024;
                    th3 = h3 + bank_offset+2048;;
                    th4 = h4 + bank_offset;
                    th5 = h5 + bank_offset;
                    th6 = h6 + bank_offset;
                    th7 = h7 + bank_offset;
                    // printk(KERN_DEBUG "%p %p\n", h1, h2);
                    t0 = get_ns();
                    for (int i = 0; i < HAMMER_READCOUNT; i++) {
                        *th1;
                        *th2;
                        *th3;
                        *th4;
                        *th5;
                        *th6;
                        *th7;
                    }
                    t1 = get_ns();
                    t_all[offset] = (t1-t0) / HAMMER_READCOUNT / 7;
                }
                for (int i = 0; i < ROW_SIZE; i++) {
                    if (v1[i] != pv) {
                        flipped.push_back(i);
                        sum += 1;
                    }
                    if (v2[i] != pv) {
                        flipped.push_back(i);
                        sum += 1;
                    }
                    if (v3[i] != pv) {
                        flipped.push_back(i);
                        sum += 1;
                    }
                    if (v4[i] != pv) {
                        flipped.push_back(i);
                        sum += 1;
                    }
                }
            }
        }
        for (int i = 0; i < 16; i++) {
            printf("%lu ", t_all[i]);
        }
        printf("\n");
        for (auto id: flipped) {
            printf("%d ", id);
        }
        printf("\n");
        printf("@@@ Flipped: %d\n", sum / REPEAT);
    }
}

int main(int argc, char *argv[]) {
    int err;
    // init
    int chipset = 30; // for Nexus 5
    chipset = 25; // 25 For Nexus 5x & Pixel XL, 21 contig
    int ion_fd = open("/dev/ion", O_RDONLY);
    if (!ion_fd) {
        perror("Could not open ion");
        exit(EXIT_FAILURE);
    }

    // alloc
    int len = ROW_NUMBERS * ROW_SIZE;
    struct ion_allocation_data allocation_data;
    allocation_data.heap_id_mask = (0x1 << chipset);
    allocation_data.flags = 0;
    allocation_data.align = 0;
    allocation_data.len = len;
    err = ioctl(ion_fd, ION_IOC_ALLOC, &allocation_data);
    if (err || allocation_data.handle == 0) {
        perror("Could not alloc");
        return -1;
    }

    // share
    struct ion_fd_data fd_data;
    fd_data.handle = allocation_data.handle;
    err = ioctl(ion_fd, ION_IOC_SHARE, &fd_data);
    if (err || fd_data.fd < 0) {
        perror("Could not share");
        return -1;
    }

    // mmap
    char *mapping;
    mapping = (char*)mmap(nullptr, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd_data.fd, 0);
    if (mapping == MAP_FAILED) {
        perror("Could not mmap");
        return -1;
    }

    printf("Allocation success.\n");

    auto pfn = get_phys_addr((uintptr_t)mapping);

    printf("phys %p %lu\n", (void*)pfn, (unsigned long)pfn%ROW_SIZE);
    printf("@@@ Discontinuity check (%d)\n", PAGESIZE);
    for (int i=0; i < len / 4096 - 1; i++) {
        if (get_phys_addr((uintptr_t) (mapping+i*4096+4096)) / PAGESIZE != get_phys_addr((uintptr_t)(mapping+i*4096)) / PAGESIZE+1)
            printf("@@@ Discontinuous %d/%d\n", i, len/4096);
        }

    rowsize(mapping);
    // doublesided_nexus5(mapping, 1);
    // doublesided_second(mapping, 1);
    // doublesided(mapping, 0);
    // singlesided(mapping, 1);
    // manysided(mapping, 1);

    std::vector<uintptr_t> hammerable_rows;

    close(ion_fd);
}
