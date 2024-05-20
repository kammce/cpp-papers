#pragma once

#include <array>

inline std::array<volatile int, 25> side_effect{};

[[gnu::noinline]] void
noexcept_bar() noexcept;

[[gnu::noinline]] void
noexcept_baz() noexcept;

[[gnu::noinline]] void
noexcept_qaz() noexcept;

[[gnu::noinline]] void
bar();

[[gnu::noinline]] void
baz();

[[gnu::noinline]] void
qaz();
