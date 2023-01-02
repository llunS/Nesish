#pragma once

#include "console/ppu/pipeline/tickable.hpp"
#include "common/klass.hpp"
#include "console/ppu/pipeline/functor_tickable.hpp"
#include "console/types.hpp"

namespace ln {

struct PipelineAccessor;

struct SpEvalFetch : public Tickable {
  public:
    SpEvalFetch(PipelineAccessor *io_accessor, bool i_sp_fetch_only);
    LN_KLZ_DELETE_COPY_MOVE(SpEvalFetch);

    void
    reset() override;
    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    PipelineAccessor *m_accessor;
    const bool m_sp_fetch_only;

    FunctorTickable m_sec_oam_clear;
    FunctorTickable m_sp_eval;
    FunctorTickable m_sp_fetch_reload;

    struct Context {
        /* eval */
        Byte sp_eval_bus;
        Byte sec_oam_write_idx;
        int cp_counter;
        Byte init_oam_addr;
        int n, m;
        bool n_overflow;
        Byte sp_got;
        bool sp_overflow; // in one scanline

        Byte sec_oam_read_idx;
        Byte sp_nt_byte;
        Byte sp_attr_byte;
        Byte sp_pos_y;
        Byte sp_idx_reload;
    } m_ctx;

  private:
    static Cycle
    pvt_sec_oam_clear(Cycle i_curr, Cycle i_total,
                      PipelineAccessor *io_accessor);
    static Cycle
    pvt_sp_eval(Cycle i_curr, Cycle i_total, PipelineAccessor *io_accessor,
                Context *io_ctx);
    static Cycle
    pvt_sp_fetch_reload(Cycle i_curr, Cycle i_total,
                        PipelineAccessor *io_accessor, Context *io_ctx);
};

} // namespace ln
