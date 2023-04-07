#pragma once

#include "nhbase/klass.hpp"
#include "types.hpp"

namespace nh {

struct PipelineAccessor;

struct SpEvalFetch {
  public:
    SpEvalFetch(PipelineAccessor *io_accessor);
    NB_KLZ_DELETE_COPY_MOVE(SpEvalFetch);

    void
    tick(Cycle i_col);

  private:
    PipelineAccessor *m_accessor;

    struct Context {
        /* eval */
        /* some states might be used afterwards at fetch stage */
        Byte sp_eval_bus; // temporary bus for eval stage
        Byte sec_oam_write_idx;
        int cp_counter;
        Byte init_oam_addr;
        int n, m;
        bool n_overflow;
        Byte sp_got;
        bool sp_overflow; // in one scanline
        bool sec_oam_written;
        bool sp0_in_range;

        /* fetch */
        Byte sec_oam_read_idx;
        Byte sp_tile_byte;
        Byte sp_attr_byte;
        Byte sp_pos_y;
        Byte sp_idx_reload;
    } m_ctx;

  private:
    static void
    pv_sec_oam_clear(Cycle i_step, PipelineAccessor *io_accessor);
    static void
    pv_sp_eval(Cycle i_step, PipelineAccessor *io_accessor, Context *io_ctx);
    static void
    pv_sp_fetch_reload(Cycle i_step, PipelineAccessor *io_accessor,
                       Context *io_ctx);
};

} // namespace nh
