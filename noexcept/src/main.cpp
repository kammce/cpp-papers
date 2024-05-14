/**
 * @file main.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-05-14
 *
 * Compilation Command:
 *
 *     arm-none-eabi-g++ main.cpp external.cpp -g -std=c++23 -Os -fexceptions
 *     -fno-rtti -mcpu=cortex-m3 -mfloat-abi=soft -mthumb --specs=nosys.specs
 *     --specs=nano.specs -Wl,-T linker.ld -o app.elf
 *
 * @copyright Copyright Khalil Estell(c) 2024
 *
 */
#include <cstdint>

#include <algorithm>
#include <span>
#include <system_error>

#include "external.hpp"

void*
to_absolute_address(volatile const void* p_address)
{
  auto address = reinterpret_cast<std::intptr_t>(p_address);
  auto offset = *reinterpret_cast<volatile const std::int32_t*>(p_address);

  // Sign extend the 31st bit
  if (offset & (1 << 30)) {
    offset |= 1 << 31;
  }

  std::intptr_t final_address = address + offset;
  return reinterpret_cast<void*>(final_address);
}

struct arm_index_entry
{
  std::uint32_t function;
  std::uint32_t content;
};

[[gnu::noinline]] void*
get_function(volatile const arm_index_entry& p_entry)
{
  return to_absolute_address(&p_entry.function);
}

extern "C"
{
  extern const arm_index_entry __exidx_start;
  extern const arm_index_entry __exidx_end;
  void __wrap___gxx_personality_v0() {}
}

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
except_calling_mixed_in_try_catch()
{
  try {
    bar();
    baz_noexcept();
  } catch (...) {
    side_effect[22] = side_effect[22] + 1;
  }
}

struct my_struct_t
{
  int a;
  int b;
  int c;
};

[[gnu::noinline]] void
initialize(my_struct_t& my_struct)
{
  my_struct.a = 5;
  my_struct.b = 15;
  my_struct.c = 15;
}

class my_class
{
public:
  enum class state_t : std::uint8_t
  {
    initialize,
    busy,
    ready,
  };

  my_class(state_t p_state) noexcept
    : m_state(p_state)
  {
  }

  state_t state_noexcept() noexcept { return m_state; }
  state_t state() { return m_state; }

private:
  state_t m_state = state_t::initialize;
};

template<typename T>
auto*
to_void(T* p_thing)
{
  auto intptr = reinterpret_cast<std::intptr_t>(p_thing);
  intptr &= ~1; // clear least significant bit produced by ARM function call ABI
  return reinterpret_cast<void*>(intptr);
}

// Specialization for member function pointers
template<typename R, typename C, typename... Args>
void*
to_void(R (C::*p_member_function)(Args...))
{
  union
  {
    R (C::*member_function)(Args...);
    void* ptr;
  } u;
  u.member_function = p_member_function;
  return to_void(u.ptr);
}

enum class metadata_rank : std::uint8_t
{
  unknown,
  no_entry,
  inlined_noexcept,
  inlined_personality,
  table_personality,
  table_gcc_lsda,
};

struct function_meta_info
{
  function_meta_info() = default;
  function_meta_info(const arm_index_entry& p_entry)
  {
    function_address = get_function(p_entry);
    index_entry = &p_entry;
    constexpr std::uint32_t is_personality_data = 1 << 31;
    constexpr std::uint32_t cannot_unwind_token = 0x1;
    if (p_entry.content == cannot_unwind_token) {
      rank = metadata_rank::inlined_noexcept;
    } else if (p_entry.content & is_personality_data) {
      rank = metadata_rank::inlined_personality;
    } else { // contains ptr to LSDA or ARM Personality
      auto* table_data_ptr = to_absolute_address(&p_entry.content);
      auto first_word = *reinterpret_cast<std::uint32_t*>(table_data_ptr);

      if (first_word & is_personality_data) {
        rank = metadata_rank::table_personality;
        return;
      }
      // If this isn't personality data, then it has to be a pointer to a
      // function that knowns how to understand the data in this section of the
      // table.
      void* data_handler = to_absolute_address(table_data_ptr);

      if (data_handler ==
          reinterpret_cast<void*>(&__wrap___gxx_personality_v0)) {
        rank = metadata_rank::table_gcc_lsda;
        return;
      }
      // Unecessary code, but make it clear that the exception rank of this
      // function is unknown at this point.
      rank = metadata_rank::unknown;
    }
  }
  void* function_address = nullptr;
  const arm_index_entry* index_entry = nullptr;
  metadata_rank rank = metadata_rank::unknown;
};

volatile my_class::state_t current_state;
volatile const arm_index_entry* start = nullptr;
volatile const arm_index_entry* end = nullptr;
volatile void* starting_function = 0;
volatile void* end_function = 0;

int
main()
{
  start = &__exidx_start;
  end = &__exidx_end;
  starting_function = get_function(*start);
  end_function = get_function(*end);

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

  my_class object1(my_class::state_t::busy);
  current_state = object1.state();
  current_state = object1.state_noexcept();

  // Scan exception table and determine which functions have
  std::array functions{
    to_void(&noexcept_calls_all_noexcept),
    to_void(&noexcept_calls_mixed_functions),
    to_void(&except_calls_all_noexcept),
    to_void(&except_calls_except),
    to_void(&noexcept_calls_except),
    to_void(&except_calls_mixed),
    to_void(&except_calls_except_in_try_catch),
    to_void(&noexcept_calls_except_in_try_catch),
    to_void(&noexcept_calls_mixed_in_try_catch),
    to_void(&except_calling_mixed_in_try_catch),
    to_void(&initialize),
    to_void(&my_class::state),
    to_void(&my_class::state_noexcept),
    to_void(&bar),
    to_void(&bar_noexcept),
    to_void(&baz),
    to_void(&baz_noexcept),
    to_void(&qaz),
    to_void(&qaz_noexcept),
  };

  std::array<function_meta_info, functions.size()> meta_info{};

  // functions are sorted relative to their position in code just like the
  // exception index.
  std::ranges::sort(functions);
  std::span<const arm_index_entry> exception_index(&__exidx_start,
                                                   &__exidx_end);

  auto f_cursor = functions.begin();
  auto m_cursor = meta_info.begin();
  auto e_cursor = exception_index.begin();
  while (f_cursor != functions.end() && e_cursor != exception_index.end()) {
    void* exception_function = get_function(*e_cursor);
    if (*f_cursor == exception_function) {
      *m_cursor = function_meta_info(*e_cursor);
      f_cursor++;
      e_cursor++;
      m_cursor++;
    } else if (*f_cursor < exception_function) {
      m_cursor->function_address = *f_cursor;
      m_cursor->rank = metadata_rank::no_entry;
      f_cursor++;
      m_cursor++;
    } else if (*f_cursor > exception_function) {
      // m_cursor->function_address = *f_cursor;
      e_cursor++;
    }
  }

  while (true) {
    continue;
  }

  return 0;
}