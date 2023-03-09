#pragma once

#include "common/klass.hpp"

#include "console/types.hpp"

namespace nh {

struct Divider {
  public:
    Divider();
    Divider(Byte2 i_reload);
    ~Divider() = default;
    LN_KLZ_DEFAULT_COPY(Divider);

  public:
    Byte2
    get_reload() const;
    void
    set_reload(Byte2 i_reload);
    void
    reload();

    Byte2
    value() const;

    bool
    tick();

  private:
    Byte2 m_reload;
    Byte2 m_ctr;
};

} // namespace nh
