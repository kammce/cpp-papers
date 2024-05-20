#include "except_vs_noexcept.hpp"
#include "external.hpp"

[[gnu::noinline]] void
noexcept_calls_all_noexcept() noexcept
{
  bar_noexcept();
  baz_noexcept();
  qaz_noexcept();
}

[[gnu::noinline]] void
noexcept_calls_mixed_functions() noexcept
{
  bar_noexcept();
  baz();
  qaz_noexcept();
}

[[gnu::noinline]] void
except_calls_all_noexcept()
{
  bar_noexcept();
  baz_noexcept();
  qaz_noexcept();
}

[[gnu::noinline]] void
except_calls_mixed() noexcept
{
  bar_noexcept();
  baz();
  qaz_noexcept();
}

[[gnu::noinline]] void
except_calls_except()
{
  bar();
  baz();
  qaz();
}

[[gnu::noinline]] void
noexcept_calls_except() noexcept
{
  bar();
  baz();
  qaz();
}

[[gnu::noinline]] void
noexcept_calls_all_noexcept_in_try_catch() noexcept
{
  try {
    bar_noexcept();
    baz_noexcept();
  } catch (...) {
    side_effect[9] = side_effect[9] + 1;
  }
}

[[gnu::noinline]] void
noexcept_calls_mixed_in_try_catch() noexcept
{
  try {
    bar();
    baz_noexcept();
  } catch (...) {
    side_effect[15] = side_effect[15] + 1;
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
initialize(my_struct_t& my_struct)
{
  my_struct.a = 5;
  my_struct.b = 15;
  my_struct.c = 15;
}

[[gnu::noinline]] void
initialize_noexcept(my_struct_t& my_struct) noexcept
{
  my_struct.a = 17;
  my_struct.b = 22;
  my_struct.c = 33;
}

[[gnu::noinline]] void
except_calling_mixed_in_try_catch()
{
  try {
    bar();
    baz_noexcept();
  } catch (...) {
    side_effect[22] = side_effect[22] + 1;
  }
}

my_class::my_class(state_t p_state) noexcept
  : m_state(p_state)
{
}

my_class::state_t
my_class::state_noexcept() noexcept
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

  noexcept_calls_all_noexcept();
  noexcept_calls_mixed_functions();
  except_calls_all_noexcept();
  except_calls_mixed();
  except_calls_except();
  noexcept_calls_except();
  except_calls_except_in_try_catch();
  noexcept_calls_except_in_try_catch();
  noexcept_calls_mixed_in_try_catch();
  except_calling_mixed_in_try_catch();

  my_struct_t my_struct;
  initialize(my_struct);
  initialize_noexcept(my_struct);

  my_class object1(my_class::state_t::busy);
  current_state = object1.state();
  current_state = object1.state_noexcept();
}