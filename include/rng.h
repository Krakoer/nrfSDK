#define RNG_BASE 0x4000D000UL

#define RNG_TASKS_START (*(volatile unsigned long *)(RNG_BASE + 0x000))
#define RNG_TASKS_STOP (*(volatile unsigned long *)(RNG_BASE + 0x004))
#define RNG_EVENTS_VALRDY (*(volatile unsigned long *)(RNG_BASE + 0x100))
#define RNG_SHORTS (*(volatile unsigned long *)(RNG_BASE + 0x200))
#define RNG_INTENSET (*(volatile unsigned long *)(RNG_BASE + 0x304))
#define RNG_INTENCLR (*(volatile unsigned long *)(RNG_BASE + 0x308))
#define RNG_CONFIG (*(volatile unsigned long *)(RNG_BASE + 0x504))
#define RNG_VALUE (*(volatile unsigned long *)(RNG_BASE + 0x508))

void rng_get(unsigned *out);
