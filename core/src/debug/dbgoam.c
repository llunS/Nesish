#include "dbgoam.h"

void
dbgoam_Init(dbgoam_s *self)
{
    for (int i = 0; i < NHD_OAM_SPRITES; ++i)
    {
        dbgspr_Init(&self->sprs_[i]);
    }
}

const dbgspr_s *
dbgoam_GetSpr(const dbgoam_s *self, int idx)
{
    return &self->sprs_[idx];
}

dbgspr_s *
dbgoam_SprOf(dbgoam_s *self, int idx)
{
    return &self->sprs_[idx];
}
