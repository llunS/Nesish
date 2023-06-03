#pragma once

namespace sh {

struct Application;

struct Messager {
  public:
    Messager(Application *i_app)
        : m_app(i_app)
    {
    }

  public:
    bool
    running_game() const;

  private:
    Application *m_app;
};

} // namespace sh
