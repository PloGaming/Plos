#include <interrupts/interrupt.h>
#include <terminal/terminal.h>
#include <memory/memory.h>
#include <config.h>
#include <strings/string.h>

__attribute__((aligned(0x10))) static struct idt_descriptor_entry idt[MAX_NUM_INTERRUPTS];

static struct idtr_register idtr = {sizeof(idt) - 1, idt};

extern void idt_table_load(struct idtr_register *idtr);

void idt_table_set_gate(int interrupt, void *address, uint16_t segment_descriptor, uint8_t flags)
{
    idt[interrupt].base1 = (uint32_t)address & 0x0000ffff;
    idt[interrupt].segment_selector = segment_descriptor;
    idt[interrupt].reserved = 0;
    idt[interrupt].type_attributes = flags;
    idt[interrupt].base2 = ((uint32_t)address >> 16) & 0x0000ffff;
}

void idt_enable_gate(int interrupt)
{
    idt[interrupt].type_attributes |= IDT_FLAG_PRESENT;
}

void idt_disable_gate(int interrupt)
{
    idt[interrupt].type_attributes &= ~IDT_FLAG_PRESENT;
}

void idt_table_init()
{
    memset(idt, 0, sizeof(idt));
    idt_table_load(&idtr);
}