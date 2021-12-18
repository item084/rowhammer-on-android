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
#include <string>

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

#include "helper.h"
#include "ion.h"
#include "massage.h"
#include "rowsize.h"
#include "templating.h"

#define HAMMER_READCOUNT 10000000

FILE *global_of = NULL;
extern int rowsize;

void resetter(uint8_t *pattern) {
    for (int i = 0; i < MAX_ROWSIZE; i++) {
        pattern[i] = rand() % 255;
    }
}


int main(int argc, char *argv[]) {
    int c;
    int timer = 0;
    int alloc_timer = 0;
    char *outputfile = NULL;
    int hammer_readcount = HAMMER_READCOUNT;
    bool heap_type_detector = false;
    bool do_conservative = false;
    bool all_patterns = false;
    int cpu_pinning = -1;
    opterr = 0;

    ION_init();

        printf("[MAIN] Pinning to CPU...\n");
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(0, &cpuset);
        if (sched_setaffinity(0, sizeof(cpuset), &cpuset)) {
            perror("Could not pin CPU");
        }

    ION_detector();
    // rowsize = RS_autodetect();
    printf("Rowsize: %d", rowsize);

    // defrag(10);
    std::vector<struct ion_data *> ion_chunks;
    std::vector<struct template_t *> templates;
    rowsize = K(64);
    exhaust(ion_chunks, rowsize * 4);
    printf("[MAIN] Initializing patterns\n");printf("[MAIN] Initializing patterns\n");

    uint8_t  ones[MAX_ROWSIZE];
    uint8_t zeros[MAX_ROWSIZE];
    uint8_t rand1[MAX_ROWSIZE];
    uint8_t rand2[MAX_ROWSIZE];
    uint8_t rand3[MAX_ROWSIZE];
    memset( ones, 0xff, MAX_ROWSIZE);
    memset(zeros, 0x00, MAX_ROWSIZE);
    for (int i = 0; i < MAX_ROWSIZE; i++) {
        rand1[i] = rand() % 255;
        rand2[i] = rand() % 255;
        rand3[i] = rand() % 255;
    }

    pattern_t p101 = { .above =  ones, .victim = zeros, .below =  ones, .cur_use = 0, .max_use = 0, .reset_above = NULL, .reset_victim = NULL, .reset_below = NULL };
    pattern_t p010 = { .above = zeros, .victim =  ones, .below = zeros, .cur_use = 0, .max_use = 0, .reset_above = NULL, .reset_victim = NULL, .reset_below = NULL };
    std::vector<struct pattern_t *> patterns;
    patterns = {&p101, &p010};

    TMPL_run(ion_chunks, templates, patterns, timer, hammer_readcount, do_conservative);
    ION_clean_all(ion_chunks);
    ION_fini();



    return 0;
}