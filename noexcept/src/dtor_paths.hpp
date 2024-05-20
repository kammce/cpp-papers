#pragma once

namespace dtor {
struct action_exception_t
{};

struct non_trivial_dtor
{
  non_trivial_dtor() = default;
  [[gnu::noinline]] void action();
  [[gnu::noinline]] void noexcept_action() noexcept;
  [[gnu::noinline]] ~non_trivial_dtor();
};

// Exhibit 9
[[gnu::noinline]] void
noexcept_calls_all_noexcept() noexcept;
[[gnu::noinline]] void
except_calls_all_noexcept();

// Exhibit 10
[[gnu::noinline]] void
noexcept_calls_all_except() noexcept;
[[gnu::noinline]] void
except_calls_all_except();

// Exhibit 11
[[gnu::noinline]] void
noexcept_calls_experiment1() noexcept;
[[gnu::noinline]] void
noexcept_calls_experiment2() noexcept;
[[gnu::noinline]] void
noexcept_calls_experiment3() noexcept;
[[gnu::noinline]] void
noexcept_calls_experiment4() noexcept;
[[gnu::noinline]] void
noexcept_calls_experiment5() noexcept;
[[gnu::noinline]] void
noexcept_calls_experiment6() noexcept;
[[gnu::noinline]] void
noexcept_calls_experiment7() noexcept;

[[gnu::noinline]] void
except_calls_experiment1();
[[gnu::noinline]] void
except_calls_experiment2();
[[gnu::noinline]] void
except_calls_experiment3();
[[gnu::noinline]] void
except_calls_experiment4();
[[gnu::noinline]] void
except_calls_experiment5();
[[gnu::noinline]] void
except_calls_experiment6();
[[gnu::noinline]] void
except_calls_experiment7();

[[gnu::noinline]] void
link_in_dtor_paths();
}