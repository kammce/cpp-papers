#pragma once

#include <array>

inline std::array<volatile int, 25> side_effect{};

[[gnu::noinline]] void
bar_noexcept() noexcept;

[[gnu::noinline]] void
baz_noexcept() noexcept;

[[gnu::noinline]] void
qaz_noexcept() noexcept;

[[gnu::noinline]] void
bar();

[[gnu::noinline]] void
baz();

[[gnu::noinline]] void
qaz();
