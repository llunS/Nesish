#pragma once

#include "nesish/nesish.h"

#include <string>

#include "nhbase/klass.hpp"

namespace sh {

struct Messager;

struct Window {
  public:
    Window(const std::string &i_name, NHConsole io_emu, Messager *i_messager)
        : m_name(i_name)
        , m_emu(io_emu)
        , m_messager(i_messager)
        , m_open(false)
    {
    }
    virtual ~Window() {}

    NB_KLZ_DELETE_COPY_MOVE(Window);

  public:
    virtual void
    render() = 0;

    void
    show()
    {
        m_open = true;
    }

  protected:
    std::string m_name;
    NHConsole m_emu;
    Messager *m_messager;

    bool m_open;
};

} // namespace sh
