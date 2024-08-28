#include "dma.h"

void pf_dma_oom_cb_default(void)
{
  exit(1);
}

#if PF_NO_DMA
static u8 pf_heap[PF_NO_DMA_SIZE];
static unsigned pf_heap_alloc = 0;
static void (*pf_dma_oom_cb)(void) = pf_dma_oom_cb_default;
#endif

void *pf_dma_alloc(unsigned size, unsigned zero)
{
/**
 * Implements very simple DMA for embedded systems that have no access to
 * malloc. Uses a static-sized heap, allocates purely linearly, and does not
 * actually free values. Use only if absolutely necessary.
 */
#if PF_NO_DMA
  if (size + pf_heap_alloc >= PF_NO_DMA_SIZE)
  {
    pf_dma_oom_cb();
    return NULL;
  }
  else
  {
    u8 *allocated_value = &pf_heap[pf_heap_alloc];

    if (zero)
    {
      unsigned offset;

      for (offset = pf_heap_alloc; offset < pf_heap_alloc + size; offset++)
        pf_heap[offset] = 0;
    }
    pf_heap_alloc += size;

    return allocated_value;
  }
#else
  return zero ? calloc(size, 1) : malloc(size);
#endif
}

void pf_dma_free(void *value)
{
#if PF_NO_DMA
  F8_UNUSED(value);
#else
  free(value);
#endif
}

void pf_dma_set_oom_cb(void *cb)
{
  pf_dma_oom_cb = cb;
}

#endif
