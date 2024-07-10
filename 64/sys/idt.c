/* 64 bit IDT implementation for the SpecOS kernel project.
 * Copyright (C) 2024 Jake Steinburger under the MIT license. See the GitHub repository for more information.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "include/idt.h"
#include "../drivers/include/vga.h"
#include "../utils/include/io.h"

// make it know what stuff is (today isn't a good day when it comes to wording for comments lol)
struct IDTEntry {
    uint16_t offset1;
    uint16_t segmentSelector;
    uint8_t ist : 3;
    uint8_t reserved0 : 5;
    uint8_t gateType : 4;
    uint8_t empty : 1;
    uint8_t dpl : 2;
    uint8_t present : 1;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t reserved;
} __attribute__((packed));

struct idtr {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

struct IDTEntry idt[256];

struct idtr IDTPtr;

// and the thingies to make it do stuff

void initIRQ();

// just a test exception handler to see it works fine
__attribute__((interrupt))
void handleKeyboard(void*) {
    writestring("\nKey pressed! :D");
    inb(0x60);
    outb(0x20, 0x20); // Acknowledge interrupt from slave PIC 
}


// takes: IDT vector number (eg. 0x01 for divide by 0 exception), a pointer to an ISR (aka the function it calls), & the flags
void idtSetDescriptor(uint8_t vect, void* isrThingy, uint8_t gateType, uint8_t dpl) {
    struct IDTEntry* thisEntry = &idt[vect];
    // set the thingies
    // isr offset
    thisEntry->offset1 = (uint64_t)isrThingy & 0xFFFF; // first 16 bits
    thisEntry->offset2 = ((uint64_t)isrThingy >> 16) & 0xFFFF; // next 16 bits (I think? not sure if this is wrong)
    thisEntry->offset3 = ((uint64_t)isrThingy >> 32) & 0xFFFFFFFF; // next 32 bits (again, not sure if i did that right.)
    // gdt segment
    thisEntry->segmentSelector = 0x08; // addresses kernel mode code segment in gdt
    // some "flags". idk why the wiki calls these flags tbh.
    thisEntry->ist = 0; // idk what this should be but apparently it can just be 0
    thisEntry->gateType = gateType; // Trap or interrupt gate?
    thisEntry->dpl = dpl;
    thisEntry->present = 1;
}

// define more stuff for PIC (hardware, yay!)

void remapPIC() {
    // ICW1: Start initialization of PIC
    outb(0x20, 0x11); // Master PIC
    outb(0xA0, 0x11); // Slave PIC

    // ICW2: Set interrupt vector offsets
    outb(0x21, 0x20); // Master PIC vector offset
    outb(0xA1, 0x28); // Slave PIC vector offset

    // ICW3: Tell Master PIC there is a slave PIC at IRQ2 (0000 0100)
    outb(0x21, 0x04);
    // Tell Slave PIC its cascade identity (0000 0010)
    outb(0xA1, 0x02);

    // ICW4: Set PIC to x86 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Mask interrupts on both PICs
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

void initIRQ() {
    remapPIC();
    idtSetDescriptor(33, &handleKeyboard, 14, 0); // map keyboard irq
    outb(0x21, ~(1 << 1)); // unmask keyboard IRQ
    asm("sti");
}

void initIDT() {
    writestring("\nSetting IDT descriptors..."); 
    writestring("\nCreating IDTR (that IDT pointer thingy)...");
    IDTPtr.offset = (uintptr_t)&idt[0];
    IDTPtr.size = ((uint16_t)sizeof(struct IDTEntry) *  256) - 1;
    writestring("\nLoading IDTR into the register thingy...");
    asm volatile("lidt %0" : : "m"(IDTPtr));
    writestring("Setting up IRQ hardware thingy...");
    initIRQ();
}






