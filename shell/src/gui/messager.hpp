#pragma once

#include "input/controller.hpp"

namespace sh {

struct Application;
struct Logger;

struct Messager {
  public:
    Messager(Application *i_app)
        : m_app(i_app)
    {
    }

  public:
    bool
    running_game() const;

    Logger *
    get_logger() const;

    const KeyMapping &
    get_key_mapping(NHCtrlPort i_port) const;

    void
    reset_key_mapping(const KeyMapping &i_p1, const KeyMapping &i_p2) const;

  private:
    Application *m_app;
};

} // namespace sh
