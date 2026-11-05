#pragma once

#define PPI_BASE 0x4001F000UL

#define PPI_TASKS_CHG_N_EN(n)  (*(volatile unsigned long *)(PPI_BASE + 0x000UL + 8UL*(n)))
#define PPI_TASKS_CHG_N_DIS(n)  (*(volatile unsigned long *)(PPI_BASE + 0x004UL + 8UL*(n)))
#define PPI_CHEN (*(volatile unsigned long *)(PPI_BASE + 0x500))
#define PPI_CHENSET (*(volatile unsigned long *)(PPI_BASE + 0x504))
#define PPI_CHENCLR (*(volatile unsigned long *)(PPI_BASE + 0x508))
#define PPI_CH_N_EEP(n)  (*(volatile unsigned long *)(PPI_BASE + 0x510UL + 8UL*(n)))
#define PPI_CH_N_TEP(n)  (*(volatile unsigned long *)(PPI_BASE + 0x514UL + 8UL*(n)))
#define PPI_CHG_N(n)  (*(volatile unsigned long *)(PPI_BASE + 0x800UL + 4UL*(n)))
#define PPI_FORK_N_TEP(n)  (*(volatile unsigned long *)(PPI_BASE + 0x910UL + 4UL*(n)))

void ppi_set_channel_task_addr(int channel, void *task_addr);
void ppi_set_channel_event_addr(int channel, void *event_addr);
void ppi_set_channel_fork_addr(int channel, void *task_addr);
void ppi_enable_channel(int channel);
void ppi_clear_channel(int channel);
int ppi_alloc_channel();
void ppi_free_channel(int channel);