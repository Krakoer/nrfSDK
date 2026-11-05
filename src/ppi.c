#include "ppi.h"

void ppi_set_channel_task_addr(int channel, void *task_addr){
    PPI_CH_N_TEP(channel) = (long unsigned int)task_addr;
}

void ppi_set_channel_event_addr(int channel, void *event_addr){
    PPI_CH_N_EEP(channel) = (long unsigned int)event_addr;
}

void ppi_enable_channel(int channel){
    PPI_CHENSET = 1UL << channel;
}

void ppi_clear_channel(int channel){
    PPI_CHENCLR = 1UL << channel;
}

void ppi_set_channel_fork_addr(int channel, void *task_addr){
    PPI_FORK_N_TEP(channel) = (long unsigned int)task_addr;
}