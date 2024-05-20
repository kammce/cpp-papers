#include "except_vs_noexcept.hpp"
#include "external.hpp"

// Exhibit 2
[[gnu::noinline]] void
noexcept_calls_all_noexcept() noexcept
{
  noexcept_bar();
  noexcept_baz();
  noexcept_qaz();
}

[[gnu::noinline]] void
except_calls_all_noexcept()
{
  noexcept_bar();
  noexcept_baz();
  noexcept_qaz();
}

[[gnu::noinline]] void
except_calls_mixed()
{
  noexcept_bar();
  baz();
  noexcept_qaz();
}

[[gnu::noinline]] void
noexcept_calls_mixed() noexcept
{
  noexcept_bar();
  baz();
  noexcept_qaz();
}

[[gnu::noinline]] void
noexcept_calls_all_except() noexcept
{
  bar();
  baz();
  qaz();
}

[[gnu::noinline]] void
except_calls_all_except()
{
  bar();
  baz();
  qaz();
}

[[gnu::noinline]] void
noexcept_calls_all_noexcept_in_try_catch() noexcept
{
  try {
    noexcept_bar();
    noexcept_baz();
  } catch (...) {
    side_effect[9] = side_effect[9] + 1;
  }
}

[[gnu::noinline]] void
except_calls_all_noexcept_in_try_catch()
{
  try {
    noexcept_bar();
    noexcept_baz();
  } catch (...) {
    side_effect[9] = side_effect[9] + 1;
  }
}

[[gnu::noinline]] void
noexcept_calls_mixed_in_try_catch() noexcept
{
  try {
    bar();
    noexcept_baz();
  } catch (...) {
    side_effect[15] = side_effect[15] + 1;
  }
}

[[gnu::noinline]] void
except_calling_mixed_in_try_catch()
{
  try {
    bar();
    noexcept_baz();
  } catch (...) {
    side_effect[22] = side_effect[22] + 1;
  }
}

[[gnu::noinline]] void
noexcept_calls_except_in_try_catch() noexcept
{
  try {
    bar();
    baz();
  } catch (...) {
    side_effect[17] = side_effect[17] + 1;
  }
}

[[gnu::noinline]] void
except_calls_except_in_try_catch()
{
  try {
    bar();
    baz();
  } catch (...) {
    side_effect[8] = side_effect[8] + 1;
  }
}

[[gnu::noinline]] void
initialize(my_struct_t& my_struct)
{
  my_struct.a = 5;
  my_struct.b = 15;
  my_struct.c = 15;
}

[[gnu::noinline]] void
noexcept_initialize(my_struct_t& my_struct) noexcept
{
  my_struct.a = 17;
  my_struct.b = 22;
  my_struct.c = 33;
}

my_class::my_class(state_t p_state) noexcept
  : m_state(p_state)
{
}

my_class::state_t
my_class::noexcept_state() noexcept
{
  return m_state;
}
my_class::state_t
my_class::state()
{
  return m_state;
}

volatile my_class::state_t current_state;

void
link_in_except_vs_noexcept()
{
  // Exhibit 1
  my_struct_t my_struct;
  initialize(my_struct);
  noexcept_initialize(my_struct);

  // Exhibit 2
  noexcept_calls_all_noexcept();
  except_calls_all_noexcept();

  // Exhibit 3
  noexcept_calls_mixed();
  except_calls_mixed();

  // Exhibit 4
  noexcept_calls_all_except();
  except_calls_all_except();

  // Exhibit 5
  noexcept_calls_all_noexcept_in_try_catch();
  except_calls_all_noexcept_in_try_catch();

  // Exhibit 6
  noexcept_calls_mixed_in_try_catch();
  except_calling_mixed_in_try_catch();

  // Exhibit 7
  noexcept_calls_except_in_try_catch();
  except_calls_except_in_try_catch();

  // Exhibit 8
  my_class object1(my_class::state_t::busy);
  current_state = object1.state();
  current_state = object1.noexcept_state();
}