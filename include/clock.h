#define CLOCK_BASE 0x40000000
#define CLOCK_TASKS_HFCLKSTART     (*(volatile unsigned long *)(CLOCK_BASE + 0x000))
#define CLOCK_EVENTS_HFCLKSTARTED  (*(volatile unsigned long *)(CLOCK_BASE + 0x100))
#define CLOCK_HFCLKSTAT            (*(volatile unsigned long *)(CLOCK_BASE + 0x40C))

void enable_hfxo();
int hfxo_enabled();