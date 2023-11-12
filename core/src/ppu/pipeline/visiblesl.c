#include "visiblesl.h"

void
visiblesl_Init(visiblesl_s *self, placcessor_s *accessor)
{
    rendersl_Init(&self->render_, accessor);
    bgfetchsl_Init(&self->bg_, accessor);
    spevalfetchsl_Init(&self->sp_, accessor);
}

void
visiblesl_Tick(visiblesl_s *self, cycle_t col)
{
    // Rendering happens before other data priming workload
    if (2 <= col && col <= 257) {
        rendersl_Tick(&self->render_, col);
    }
    if ((1 <= col && col <= 257) || (321 <= col && col <= 340)) {
        bgfetchsl_Tick(&self->bg_, col);
    }
    if (1 <= col && col <= 320) {
        spevalfetchsl_Tick(&self->sp_, col);
    }
}
