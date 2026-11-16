#define NVIC_ISER0    (*(volatile unsigned long *)0xE000E100UL)
#define NVIC_ISER1    (*(volatile unsigned long *)0xE000E104UL)
#define NVIC_ISPR1    (*(volatile unsigned long *)0xE000E204UL)
#define NVIC_IPR_N(n) (*(volatile char *)(0xE000E400UL+(n)))