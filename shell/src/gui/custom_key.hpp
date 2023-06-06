#pragma once

#include "gui/window.hpp"
#include "input/controller.hpp"

namespace sh {

struct CustomKey : public Window {
  public:
    CustomKey(const std::string &i_name, NHConsole io_emu,
              Messager *i_messager);
    ~CustomKey();

  public:
    void
    render() override;

  public:
    bool
    on_key_released(VirtualKey i_vkey, NHCtrlPort &o_port, NHKey &o_key);

  private:
    void
    draw_key_config(NHCtrlPort i_port, const char *i_port_name);

    void
    on_key_clicked(NHCtrlPort i_port, NHKey i_key);

    bool m_recording;
    NHCtrlPort m_recording_port;
    NHKey m_recording_key;
};

} // namespace sh
