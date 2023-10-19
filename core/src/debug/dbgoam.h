#pragma once

#include "nesish/nesish.h"

#include "debug/dbgspr.h"

typedef struct dbgoam_s {
    dbgspr_s sprs_[NHD_OAM_SPRITES];
} dbgoam_s;

void
dbgoam_Init(dbgoam_s *self);

const dbgspr_s *
dbgoam_GetSpr(const dbgoam_s *self, int idx);

dbgspr_s *
dbgoam_SprOf(dbgoam_s *self, int idx);
