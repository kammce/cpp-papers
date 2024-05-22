#include "dtor_paths.hpp"
#include "external.hpp"

namespace dtor {
[[gnu::noinline]] void
non_trivial_dtor::action()
{
  side_effect[1] = side_effect[1] + 1;
  if (side_effect[1] >= 15) {
    throw action_exception_t{};
  }
}

[[gnu::noinline]] void
non_trivial_dtor::noexcept_action() noexcept
{
  side_effect[1] = side_effect[1] + 1;
}

non_trivial_dtor::~non_trivial_dtor()
{
  side_effect[0] = side_effect[0] + 1;
}

[[gnu::noinline]] void
noexcept_calls_all_noexcept() noexcept
{
  non_trivial_dtor obj1;
  obj1.noexcept_action();
  non_trivial_dtor obj2;
  obj1.noexcept_action();
  obj2.noexcept_action();
  non_trivial_dtor obj3;
  obj1.noexcept_action();
  obj2.noexcept_action();
  obj3.noexcept_action();
}

[[gnu::noinline]] void
except_calls_all_noexcept()
{
  non_trivial_dtor obj1;
  obj1.noexcept_action();
  non_trivial_dtor obj2;
  obj1.noexcept_action();
  obj2.noexcept_action();
  non_trivial_dtor obj3;
  obj1.noexcept_action();
  obj2.noexcept_action();
  obj3.noexcept_action();
}

[[gnu::noinline]] void
noexcept_calls_all_except() noexcept
{
  non_trivial_dtor obj1;
  obj1.action();
  non_trivial_dtor obj2;
  obj1.action();
  obj2.action();
  non_trivial_dtor obj3;
  obj1.action();
  obj2.action();
  obj3.action();
}

[[gnu::noinline]] void
except_calls_all_except()
{
  non_trivial_dtor obj1;
  obj1.action();
  non_trivial_dtor obj2;
  obj1.action();
  obj2.action();
  non_trivial_dtor obj3;
  obj1.action();
  obj2.action();
  obj3.action();
}

[[gnu::noinline]] void
noexcept_calls_experiment1() noexcept
{
  non_trivial_dtor obj1;
  obj1.action(); // experiment 1
  non_trivial_dtor obj2;
  obj1.noexcept_action(); // experiment 2
  obj2.noexcept_action(); // experiment 3
  non_trivial_dtor obj3;
  obj1.noexcept_action(); // experiment 4
  obj2.noexcept_action(); // experiment 5
  obj3.noexcept_action(); // experiment 6
}

[[gnu::noinline]] void
noexcept_calls_experiment2() noexcept
{
  non_trivial_dtor obj1;
  obj1.noexcept_action();
  non_trivial_dtor obj2;
  obj1.action();
  obj2.noexcept_action();
  non_trivial_dtor obj3;
  obj1.noexcept_action();
  obj2.noexcept_action();
  obj3.noexcept_action();
}

[[gnu::noinline]] void
noexcept_calls_experiment3() noexcept
{
  non_trivial_dtor obj1;
  obj1.noexcept_action();
  non_trivial_dtor obj2;
  obj1.noexcept_action();
  obj2.action();
  non_trivial_dtor obj3;
  obj1.noexcept_action();
  obj2.noexcept_action();
  obj3.noexcept_action();
}

[[gnu::noinline]] void
noexcept_calls_experiment4() noexcept
{
  non_trivial_dtor obj1;
  obj1.noexcept_action();
  non_trivial_dtor obj2;
  obj1.noexcept_action();
  obj2.noexcept_action();
  non_trivial_dtor obj3;
  obj1.action();
  obj2.noexcept_action();
  obj3.noexcept_action();
}

[[gnu::noinline]] void
noexcept_calls_experiment5() noexcept
{
  non_trivial_dtor obj1;
  obj1.noexcept_action();
  non_trivial_dtor obj2;
  obj1.noexcept_action();
  obj2.noexcept_action();
  non_trivial_dtor obj3;
  obj1.noexcept_action();
  obj2.action();
  obj3.noexcept_action();
}

[[gnu::noinline]] void
noexcept_calls_experiment6() noexcept
{
  non_trivial_dtor obj1;
  obj1.noexcept_action();
  non_trivial_dtor obj2;
  obj1.noexcept_action();
  obj2.noexcept_action();
  non_trivial_dtor obj3;
  obj1.noexcept_action();
  obj2.noexcept_action();
  obj3.action();
}

[[gnu::noinline]] void
noexcept_calls_experiment7() noexcept
{
  non_trivial_dtor obj1;
  obj1.action();
  non_trivial_dtor obj2;
  obj1.action();
  obj2.noexcept_action();
  non_trivial_dtor obj3;
  obj1.noexcept_action();
  obj2.noexcept_action();
  obj3.action();
}

[[gnu::noinline]] void
except_calls_experiment1()
{
  non_trivial_dtor obj1;
  obj1.action();
  non_trivial_dtor obj2;
  obj1.noexcept_action();
  obj2.noexcept_action();
  non_trivial_dtor obj3;
  obj1.noexcept_action();
  obj2.noexcept_action();
  obj3.noexcept_action();
}

[[gnu::noinline]] void
except_calls_experiment2()
{
  non_trivial_dtor obj1;
  obj1.noexcept_action();
  non_trivial_dtor obj2;
  obj1.action();
  obj2.noexcept_action();
  non_trivial_dtor obj3;
  obj1.noexcept_action();
  obj2.noexcept_action();
  obj3.noexcept_action();
}

[[gnu::noinline]] void
except_calls_experiment3()
{
  non_trivial_dtor obj1;
  obj1.noexcept_action();
  non_trivial_dtor obj2;
  obj1.noexcept_action();
  obj2.action();
  non_trivial_dtor obj3;
  obj1.noexcept_action();
  obj2.noexcept_action();
  obj3.noexcept_action();
}

[[gnu::noinline]] void
except_calls_experiment4()
{
  non_trivial_dtor obj1;
  obj1.noexcept_action();
  non_trivial_dtor obj2;
  obj1.noexcept_action();
  obj2.noexcept_action();
  non_trivial_dtor obj3;
  obj1.action();
  obj2.noexcept_action();
  obj3.noexcept_action();
}

[[gnu::noinline]] void
except_calls_experiment5()
{
  non_trivial_dtor obj1;
  obj1.noexcept_action();
  non_trivial_dtor obj2;
  obj1.noexcept_action();
  obj2.noexcept_action();
  non_trivial_dtor obj3;
  obj1.noexcept_action();
  obj2.action();
  obj3.noexcept_action();
}

[[gnu::noinline]] void
except_calls_experiment6()
{
  non_trivial_dtor obj1;
  obj1.noexcept_action();
  non_trivial_dtor obj2;
  obj1.noexcept_action();
  obj2.noexcept_action();
  non_trivial_dtor obj3;
  obj1.noexcept_action();
  obj2.noexcept_action();
  obj3.action();
}

[[gnu::noinline]] void
except_calls_experiment7()
{
  non_trivial_dtor obj1;
  obj1.action();
  non_trivial_dtor obj2;
  obj1.action();
  obj2.noexcept_action();
  non_trivial_dtor obj3;
  obj1.noexcept_action();
  obj2.noexcept_action();
  obj3.action();
}

[[gnu::noinline]] void
link_in_dtor_paths()
{
  noexcept_calls_all_except();
  noexcept_calls_all_noexcept();
  except_calls_all_except();
  except_calls_all_noexcept();
  noexcept_calls_experiment1();
  noexcept_calls_experiment2();
  noexcept_calls_experiment3();
  noexcept_calls_experiment4();
  noexcept_calls_experiment5();
  noexcept_calls_experiment6();
  noexcept_calls_experiment7();
  except_calls_experiment1();
  except_calls_experiment2();
  except_calls_experiment3();
  except_calls_experiment4();
  except_calls_experiment5();
  except_calls_experiment6();
  except_calls_experiment7();
}
}