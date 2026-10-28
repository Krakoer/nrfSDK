extern unsigned long _sidata, _sdata, _edata, _sbss, _ebss, _estack;

#define SCB_VTOR (*(volatile unsigned long *)0xE000ED08)
#define VECTOR_TABLE_ADDR 0x00001000UL

void Reset_Handler(void);
void Default_Handler(void);
void RADIO_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
int main(void);

__attribute__((section(".isr_vector")))
void (* const vector_table[])(void) = {
    (void (*)(void))&_estack,
    Reset_Handler,
    Default_Handler, Default_Handler, Default_Handler, Default_Handler,
    Default_Handler, 0, 0, 0, 0,
    Default_Handler, Default_Handler, 0,
    Default_Handler, Default_Handler, 
    // Start of IRQs
    Default_Handler, // Timer
    RADIO_IRQHandler,
};

void Reset_Handler(void)
{
    SCB_VTOR = VECTOR_TABLE_ADDR;   /* repoint NVIC at our table, not the MBR's */

    unsigned long *src = &_sidata;
    unsigned long *dst = &_sdata;
    while (dst < &_edata) *dst++ = *src++;

    dst = &_sbss;
    while (dst < &_ebss) *dst++ = 0;

    main();
    while (1) {}
}

void Default_Handler(void)
{
    while (1) {}
}