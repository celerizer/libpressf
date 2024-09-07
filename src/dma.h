#ifndef PRESS_F_DMA_H
#define PRESS_F_DMA_H

#include "config.h"

void *pf_dma_alloc(unsigned size, unsigned zero);

void pf_dma_free(void *value);

void pf_dma_set_oom_cb(void *cb);

#endif
