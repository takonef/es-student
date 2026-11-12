#include "memory.h"
#include "device.h"
#include "led.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "command.h"

#include "pico/stdlib.h"
#include "hardware/regs/addressmap.h"

uint32_t data_variable = 100;
uint32_t bss_variable;

int main(void);

#define VECTOR_TABLE 0x10000100
#define GPIO_IN      0xd0000004

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;

// memory.c

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}

void mem_info(void)
{
    const uintptr_t flash_st = XIP_BASE;
    const uintptr_t flash_en = XIP_BASE + PICO_FLASH_SIZE_BYTES;
    const uintptr_t sram_st = SRAM_BASE;
    const uintptr_t sram_en = SRAM_BASE + 264u * 1024u; // 264 КБ SRAM
    const uintptr_t rom_st  = ROM_BASE;
    const uintptr_t rom_en  = ROM_BASE + 16u * 1024u;   // 16 КБ Boot ROM

    const uintptr_t image_st = (uintptr_t)&__flash_binary_start;
    const uintptr_t image_en = (uintptr_t)&__flash_binary_end;
    const uintptr_t boot2_st = (uintptr_t)&__boot2_start__;
    const uintptr_t boot2_en = (uintptr_t)&__boot2_end__;
    const uintptr_t etext = (uintptr_t)&__etext;
    const uintptr_t data_ram_st = (uintptr_t)&__data_start__;
    const uintptr_t data_ram_en = (uintptr_t)&__data_end__;
    const uintptr_t bss_st = (uintptr_t)&__bss_start__;
    const uintptr_t bss_en = (uintptr_t)&__bss_end__;
    const uintptr_t heap_en = (uintptr_t)&__HeapLimit;
    const uintptr_t stack_btm = (uintptr_t)&__StackBottom;
    const uintptr_t stack_top = (uintptr_t)&__StackTop;

    const uintptr_t data_size = data_ram_en - data_ram_st;
    const uintptr_t boot2_size = boot2_en - boot2_st;
    const uintptr_t text_size = etext - boot2_en;
    const uintptr_t bss_size = bss_en - bss_st;
    const uintptr_t heap_size = heap_en - bss_en;
    const uintptr_t stack_size = stack_top - stack_btm;
    
    printf("area       start      end        size\n");

    row("flash", flash_st, flash_en);
    row("sram", sram_st, sram_en);
    row("rom", rom_st, rom_en);

    row("image", image_st, image_en);
    row("free", image_en, flash_en);
    row("boot2", boot2_st, boot2_en);
    row("text", boot2_en, etext);

    row("data flash", etext, etext + data_size); /* .data во флеш */
    row("data ram", data_ram_st, data_ram_en);      /* .data в ОЗУ   */
    row("bss", bss_st, bss_en);
    row("heap", bss_en, heap_en);
    row("stack", stack_btm, stack_top);

    printf("\n");
    printf("total\n");
    printf("  %-12s%8u = boot2 %u + text %u + data %u\n",
           "flash image",
           (unsigned)(image_en - image_st),
           (unsigned)boot2_size,
           (unsigned)text_size,
           (unsigned)data_size);
    printf("  %-12s%8u of %u\n",
           "flash free",
           (unsigned)(flash_en - image_en),
           (unsigned)PICO_FLASH_SIZE_BYTES);
    printf("  %-12s%8u = data %u + bss %u\n",
           "ram used",
           (unsigned)(data_size + bss_size),
           (unsigned)data_size,
           (unsigned)bss_size);
    printf("  %-12s%8u for heap and %u for stack\n",
           "ram free",
           (unsigned)heap_size,
           (unsigned)stack_size);
}

void fw_info(void)
{
    data_variable++;
    bss_variable++;

    uint32_t stack_variable = 1946;

    uint32_t *heap_variable = (uint32_t *)malloc(sizeof(uint32_t));
    if (heap_variable != NULL)
    {
        *heap_variable = 1951;
    }

    uint16_t *main_code    = (uint16_t *)((uintptr_t)main & ~1u);
    uint16_t *fw_info_code = (uint16_t *)((uintptr_t)fw_info & ~1u);

    printf("object          address     value\n");

    printf("main            0x%08x  0x%04x\n",
           (unsigned)(uintptr_t)main,
           (unsigned)*main_code);

    printf("fw_info         0x%08x  0x%04x\n",
           (unsigned)(uintptr_t)fw_info,
           (unsigned)*fw_info_code);

    printf("commands        0x%08x\n",
           (unsigned)(uintptr_t)commands);

    for (uint i = 0; i < command_count; i++)
    {
        printf("- %-12s 0x%08x\n",
               commands[i].name,
               (unsigned)(uintptr_t)commands[i].handler);
    }

    printf("DEVICE_PROJECT  0x%08x  %s\n",
           (unsigned)(uintptr_t)DEVICE_PROJECT,
           DEVICE_PROJECT);

    printf("DEVICE_BOARD    0x%08x  %s\n",
           (unsigned)(uintptr_t)DEVICE_BOARD,
           DEVICE_BOARD);

    printf("data_variable   0x%08x  %u\n",
           (unsigned)(uintptr_t)&data_variable,
           (unsigned)data_variable);

    printf("bss_variable    0x%08x  %u\n",
           (unsigned)(uintptr_t)&bss_variable,
           (unsigned)bss_variable);

    printf("stack_variable  0x%08x  %u\n",
           (unsigned)(uintptr_t)&stack_variable,
           (unsigned)stack_variable);

    printf("heap_variable   0x%08x  %u\n",
           (unsigned)(uintptr_t)heap_variable,
           heap_variable != NULL ? (unsigned)*heap_variable : 0u);

    free(heap_variable);
}

void boot_info(void)
{
    const uint32_t *vectors = (const uint32_t *)VECTOR_TABLE;

    uint32_t stack_top     = vectors[0];
    uint32_t reset_handler = vectors[1];

    volatile uint32_t *gpio_in = (volatile uint32_t *)GPIO_IN;
    uint32_t level = (*gpio_in >> led_pin()) & 1u;

    printf("vector table   0x%08x\n", (unsigned)VECTOR_TABLE);
    printf("  stack top    0x%08x\n", (unsigned)stack_top);
    printf("  reset        0x%08x\n", (unsigned)reset_handler);
    printf("  reset (even) 0x%08x\n", (unsigned)(reset_handler & ~1u));
    printf("gpio in        0x%08x\n", (unsigned)GPIO_IN);
    printf("  led bit      %u\n",   (unsigned)level);
    printf("  gpio_get     %u\n",   (unsigned)gpio_get(led_pin()));
}