/* 64 bit version of the SpecOS kernel.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "limine.h"

#include "drivers/include/serial.h"
#include "drivers/include/vga.h"
#include "sys/include/gdt.h"
#include "sys/include/idt.h"
#include "drivers/include/keyboard.h"
#include "drivers/include/rtc.h"
#include "drivers/include/disk.h"
#include "utils/include/string.h"
#include "include/shell.h"
#include "utils/include/printf.h"
#include "mem/include/pmm.h"
#include "limine.h"
#include "mem/include/paging.h"
#include "sys/include/panic.h"

// get stuff from limine so that other kernel modules can use it
__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmapRequest = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdmRequest = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

void _start() {
    init_serial();
    initVGA(); 
    // Just send output to a serial port to test
    writestring("Trying to initialise GDT...\n");
    initGDT();
    writestring("\nGDT successfully initialised! (as far as can be told. All I know is that there isn't a gpf.)");
    writestring("\n\nTrying to initialise IDT & everything related...\n");
    initIDT();
    writestring("\nStarting physical memory manager...");
    initPMM(memmapRequest, hhdmRequest);
    // this is commented out cos paging doesn't work yet and it's still in progress.
    //writestring("\nInitiating paging...");
    //initPaging(hhdmRequest);
    test_userspace(memmapRequest);
    for (;;);
}
