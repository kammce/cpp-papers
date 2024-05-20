
#include "external.hpp"

struct forbidden
{};

void
inner_side_effect()
{
  side_effect[22] = side_effect[22] + 1;

  if (side_effect[6] == 0xFFFF) {
    throw forbidden{};
  }
}

void
noexcept_bar() noexcept
{
  inner_side_effect();
  side_effect[0] = side_effect[0] + 1;
}

void
noexcept_baz() noexcept
{
  inner_side_effect();
  side_effect[1] = side_effect[1] + 1;
}

void
noexcept_qaz() noexcept
{
  inner_side_effect();
  side_effect[2] = side_effect[2] + 1;
}

void
bar()
{
  inner_side_effect();
  side_effect[3] = side_effect[3] + 1;
  if (side_effect[3] == 0xFFFF) {
    throw 5;
  }
}

void
baz()
{
  inner_side_effect();
  side_effect[4] = side_effect[4] + 1;
  if (side_effect[4] == 0xFFFF) {
    throw 'C';
  }
}

struct qaz_t
{};

void
qaz()
{
  inner_side_effect();
  side_effect[5] = side_effect[5] + 1;
  if (side_effect[5] == 0xFFFF) {
    throw qaz_t{};
  }
}
