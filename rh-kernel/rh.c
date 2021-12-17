#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <asm/mman.h> 
#include <linux/sysinfo.h>
#include <asm/page.h>
#include <asm/io.h>
#include <linux/string.h>
#include <linux/ion.h>
#include <linux/msm_ion.h>
#include <linux/time.h>
// #include "/home/yuan/projects/drammer/exp/msm/drivers/gpu/ion/ion_priv.h"
// cd /data/local; rmmod rh; insmod rh.ko; dmesg -c

// dmesg --console-level debug

// #define EXPECTED_ROW_COUNT 16
#define ROW_NUMBERS 5 * 1024 / 64
#define ROW_SIZE 64 * 1024
#define MAX_NOP 2

struct ion_handle *data;
struct ion_client *client;
struct timespec ts;

int init_module(void) {
    uint mapping_size = ROW_NUMBERS * ROW_SIZE;
    uint offset = 0;
    uint i = 0;
    uint j = 0;
    uint nops = 0;
    char* mapping = NULL;
    char* h1, * h2, * v;
    volatile char* th1, *th2;
    int sum = 0;
    unsigned long t0 = 0;
    unsigned long t1 = 0;
    unsigned long t_all[ROW_NUMBERS];

    client = msm_ion_client_create(-1, "user");
    data = ion_alloc(client, mapping_size, 0x1000, (0x1 << 30 ), 0);
    mapping = ion_map_kernel(client, data);
    // printk(KERN_DEBUG "pfn: %ld, phys: %p", vmalloc_to_pfn(mapping), (void *)PFN_PHYS(vmalloc_to_pfn(mapping)));
    // printk(KERN_DEBUG "pfn: %ld, phys: %p", vmalloc_to_pfn(mapping+1), (void *)PFN_PHYS(vmalloc_to_pfn(mapping+1)));
    // printk(KERN_DEBUG "pfn: %ld, phys: %p", vmalloc_to_pfn(mapping+4096), (void *)PFN_PHYS(vmalloc_to_pfn(mapping+4096)));
    // physical page frame number
    printk(KERN_DEBUG "@@@ Discontinuity check");
    for (i=0; i<mapping_size/4096-1; i++) {
        if (vmalloc_to_pfn(mapping+i*4096+4096) != vmalloc_to_pfn(mapping+i*4096)+1)
            printk(KERN_DEBUG "@@@ Discontinuous %d/%d", i, mapping_size/4096);
        }

    printk(KERN_DEBUG "### Page size: %ld ...\n", PAGE_SIZE);

    for (nops = 0; nops < MAX_NOP; nops += 2) {
        printk(KERN_DEBUG "HAMMERING with %d nops\n", nops);

        sum = 0;
        for (offset = 0; offset < ROW_NUMBERS - 2; offset++) {

            h1 = mapping + ROW_SIZE * offset;
            v = h1 + ROW_SIZE;
            h2 = v + ROW_SIZE;
            memset( h1, 0x00, ROW_SIZE );
            memset( v, 0xFF, ROW_SIZE );
            memset( h2, 0x00, ROW_SIZE );
        
            th1 = h1;
            th2 = h2;
            // printk(KERN_DEBUG "%p %p\n", h1, h2);
            getnstimeofday(&ts);
            t0 = ts.tv_nsec;
            for (i = 0; i < 1000000; i++) {
                *th1;
                for (j = 0; j < nops; j ++) { asm ("nop"); }
                *th2;
                for (j = 0; j < nops; j ++) { asm ("nop"); }
            }
            getnstimeofday(&ts);

            t1 = ts.tv_nsec;
            t_all[offset] = (t1-t0) / 2000000;
            // printk(KERN_DEBUG "%d RowIdx: %lu Time: %lu %lu %lu [ns]\n", offset, row_index_start+offset+1, t0, t1, (t1-t0)/2000000);
            
            for (i = 0; i < ROW_SIZE; i++) {
                if (v[i] != 0xFF) {
                    // printk(KERN_DEBUG "[FLIP]");
                    // printk(KERN_DEBUG "[FLIP] @Phys %p ", (char*)phys_start + ROW_SIZE * (offset + 1) + i);
                    // printk(KERN_DEBUG "[FLIP] @ %d ", ROW_SIZE * (offset + 1) + i);
                    sum += 1;
                }
            }
            // printk(KERN_DEBUG "@@@ time: %lu", t_all[offset]);
            // printk(KERN_DEBUG "@@@ Flipped: %d\n", sum);
        
        }
        for (i = 0; i < 32; i++) {
             printk(KERN_DEBUG "@@@ time-at %d: %lu", i, t_all[i]);
        }
        printk(KERN_DEBUG "@@@ Flipped: %d\n", sum);
    }


    // adb push rh.ko /data/local
    // cd /data/local; rmmod rh; insmod rh.ko; dmesg -c

    //
    printk(KERN_DEBUG "Hello android kernel...\n");
    return 0;
}

void cleanup_module(void) {
    ion_unmap_kernel(client, data);
    ion_free(client, data);
    printk(KERN_DEBUG "Goodbye android kernel...\n");
}