#pragma once

#include <cstdint>

[[gnu::noinline]] void
noexcept_calls_all_noexcept() noexcept;
[[gnu::noinline]] void
noexcept_calls_mixed_functions() noexcept;
[[gnu::noinline]] void
except_calls_all_noexcept();
[[gnu::noinline]] void
except_calls_mixed() noexcept;
[[gnu::noinline]] void
except_calls_except();
[[gnu::noinline]] void
noexcept_calls_except() noexcept;
[[gnu::noinline]] void
noexcept_calls_all_noexcept_in_try_catch() noexcept;
[[gnu::noinline]] void
noexcept_calls_mixed_in_try_catch() noexcept;
[[gnu::noinline]] void
except_calls_except_in_try_catch();
[[gnu::noinline]] void
noexcept_calls_except_in_try_catch() noexcept;
[[gnu::noinline]] void
except_calling_mixed_in_try_catch();

struct my_struct_t
{
  int a;
  int b;
  int c;
};

[[gnu::noinline]] void
initialize(my_struct_t& my_struct);

[[gnu::noinline]] void
initialize_noexcept(my_struct_t&) noexcept;

class my_class
{
public:
  enum class state_t : std::uint8_t
  {
    initialize,
    busy,
    ready,
  };

  my_class(state_t p_state) noexcept;

  state_t state_noexcept() noexcept;
  state_t state();

private:
  state_t m_state = state_t::initialize;
};

void
link_in_except_vs_noexcept();