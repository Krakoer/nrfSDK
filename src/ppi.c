#include "ppi.h"
#include <stdint.h>

static uint32_t taken_channels = 0;

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

int ppi_alloc_channel(){
    // Allocate the first free channel
    int i = 0;
    while(i < 19 && ((taken_channels & (1<<i)) != 0)){
        i++;
    }
    // If all channels are taken, return -1
    if(i == 19){
        return -1;
    }
    // Alloc and return
    taken_channels |= (1<<i);
    return i;
}

void ppi_free_channel(int channel){
    if(channel >= 0 && channel < 19){
        taken_channels &= ~(1 << channel);
    }
}