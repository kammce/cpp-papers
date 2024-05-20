/**
 * @file main.cpp
 * @author Khalil Estell
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
#include <exception>
#include <span>

#include "dtor_paths.hpp"
#include "except_vs_noexcept.hpp"
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
namespace __cxxabiv1 {                               // NOLINT
std::terminate_handler __terminate_handler = +[]() { // NOLINT
  while (true) {
    continue;
  }
};
}

extern "C"
{
  void _exit([[maybe_unused]] int rc) // NOLINT
  {
    std::terminate();
  }
  void* __wrap___cxa_allocate_exception(unsigned int p_size) noexcept // NOLINT
  {
    // Size of the GCC exception object header is 128 bytes. Will have to update
    // this if the size of the EO increases. 😅
    // Might need to add some GCC macro flags here to keep track of all of the
    // EO sizes over the versions.
    constexpr size_t header_size = 200;
    static std::array<std::uint8_t, 256> exception_memory{};
    return exception_memory.data() + header_size;
  }

  void __wrap___cxa_free_exception(void*) noexcept // NOLINT
  {
  }
  extern const arm_index_entry __exidx_start;
  extern const arm_index_entry __exidx_end;
  extern void __gxx_personality_v0(...);
}

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
  unknown = 0,
  no_entry,
  inlined_noexcept,
  inlined_personality,
  table_personality,
  table_gcc_lsda,
};

struct lsda_section_size
{
  size_t count = 0;
  size_t size = 0;
};

struct exception_info
{
  exception_info() = default;

  exception_info(const arm_index_entry& p_entry)
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

      if (data_handler == reinterpret_cast<void*>(__gxx_personality_v0)) {
        // This is GCC LSDA data
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

std::uint32_t
read_uleb128(const std::uint8_t** p_ptr)
{
  std::uint32_t result = 0;
  std::uint8_t shift_amount = 0;

  while (true) {
    const std::uint8_t uleb128 = **p_ptr;

    result |= (uleb128 & 0b0111'1111) << shift_amount;
    shift_amount += 7;
    (*p_ptr)++;

    if (not(uleb128 & 0b1000'0000)) {
      break;
    }
  }

  return result;
}

std::int32_t
read_leb128(const std::uint8_t** p_ptr)
{
  std::int32_t result = 0;
  std::uint8_t shift_amount = 0;

  while (true) {
    const std::uint8_t leb128 = **p_ptr;

    result |= (leb128 & 0b0111'1111) << shift_amount;
    shift_amount += 7;
    (*p_ptr)++;

    if (not(leb128 & 0b1000'0000)) {
      if (leb128 & 0b0100'000) {
        result |= (~0 << shift_amount);
      }
      break;
    }
  }

  return result;
}

enum class personality_encoding : std::uint8_t
{
  absptr = 0x00,
  uleb128 = 0x01,
  udata2 = 0x02,
  udata4 = 0x03,
  udata8 = 0x04,
  sleb128 = 0x09,
  sdata2 = 0x0A,
  sdata4 = 0x0B,
  sdata8 = 0x0C,

  pcrel = 0x10,
  textrel = 0x20,
  datarel = 0x30,
  funcrel = 0x40,
  aligned = 0x50,

  // no data follows
  omit = 0xff,
};

struct lsda_info
{
  void* function = nullptr;
  bool valid = false;
  std::uint32_t total_size = 0;
  std::uint32_t max_action = 0;
  std::uint32_t type_offset = 0;
  personality_encoding type_encoding = personality_encoding::omit;
  personality_encoding call_site_encoding = personality_encoding::omit;
  lsda_section_size call_site{};
  lsda_section_size action_table{};
  lsda_section_size type_table{};
};

template<typename T>
volatile const T*
as(volatile const void* p_ptr)
{
  return reinterpret_cast<volatile const T*>(p_ptr);
}

personality_encoding
operator&(const personality_encoding& p_encoding, const std::uint8_t& p_byte)
{
  return static_cast<personality_encoding>(
    static_cast<std::uint8_t>(p_encoding) & p_byte);
}
personality_encoding
operator&(const personality_encoding& p_encoding,
          const personality_encoding& p_byte)
{
  return static_cast<personality_encoding>(
    static_cast<std::uint8_t>(p_encoding) & static_cast<std::uint8_t>(p_byte));
}

std::uintptr_t
read_encoded_data(const std::uint8_t** p_data, personality_encoding p_encoding)
{
  const std::uint8_t* ptr = *p_data;
  std::uintptr_t result = 0;
  const auto encoding = static_cast<personality_encoding>(p_encoding);

  if (encoding == personality_encoding::omit) {
    return 0;
  }

  // TODO: convert to hal::bit_extract w/ bit mask
  const auto encoding_type = p_encoding & 0x0F;

  switch (encoding_type) {
    case personality_encoding::absptr:
      result = *as<std::uintptr_t>(ptr);
      ptr += sizeof(std::uintptr_t);
      break;
    case personality_encoding::uleb128:
      result = read_uleb128(&ptr);
      break;
    case personality_encoding::udata2:
      result = *as<std::uint16_t>(ptr);
      ptr += sizeof(std::uint16_t);
      break;
    case personality_encoding::udata4:
      result = *as<std::uint32_t>(ptr);
      ptr += sizeof(std::uint32_t);
      break;
    case personality_encoding::sdata2:
      result = *as<std::int16_t>(ptr);
      ptr += sizeof(std::int16_t);
      break;
    case personality_encoding::sdata4:
      result = *as<std::int32_t>(ptr);
      ptr += sizeof(std::int32_t);
      break;
    case personality_encoding::sleb128:
      result = read_leb128(&ptr);
      break;
    case personality_encoding::sdata8:
      result = *as<std::int64_t>(ptr);
      break;
    case personality_encoding::udata8:
      result = *as<std::uint64_t>(ptr);
    default:
      std::terminate();
      break;
  }

  const auto encoding_offset = p_encoding & 0x70;

  switch (encoding_offset) {
    case personality_encoding::absptr:
      // do nothing
      break;
    case personality_encoding::pcrel:
      result += static_cast<std::uintptr_t>(*ptr);
      break;
    case personality_encoding::textrel:
    case personality_encoding::datarel:
    case personality_encoding::funcrel:
    case personality_encoding::aligned:
    default:
      break;
  }

  // Handle indirection GCC extension
  if (static_cast<bool>(p_encoding & 0x80)) {
    result = *reinterpret_cast<const std::uintptr_t*>(result);
  }

  *p_data = ptr;
  return result;
}

[[gnu::noinline]] lsda_info
generate_lsda_info(const exception_info& p_info)
{
  lsda_info info{};
  info.function = p_info.function_address;
  if (p_info.rank != metadata_rank::table_gcc_lsda) {
    return info;
  }

  const auto* const top_of_lsda_data = reinterpret_cast<const std::uint32_t*>(
    to_absolute_address(&p_info.index_entry->content));
  auto* lsda_data_word = top_of_lsda_data;

  lsda_data_word++; // skip personality function offset
  auto personality_type = (*lsda_data_word >> 24 & 0x7F);
  if (personality_type == 0x0) { // SU16
    lsda_data_word++;            // skip SU16 unwind instructions
  } else {
    auto words_after_first = (*lsda_data_word >> 16) & 0xFF;
    if (words_after_first > 2) {
      // This is weird case where the unwind instructions take the place of the
      // length field. This seems to be an optimization on LU16/LU32 needing an
      // additional word to support the maximum possible of 7, but with this ABI
      // break, the max number of 4 byte words is 2 in this case. This is an
      // assumption of what is happening. This could also be a bug.
      lsda_data_word += 2;
    } else {
      // skip the words_after_first of the unwind instructions
      lsda_data_word += words_after_first + 1; // +1 to skip the initial word
    }
  }

  const std::uint8_t* lsda_data =
    reinterpret_cast<const std::uint8_t*>(lsda_data_word);

  // Check if DWARF info is include (return early if so, not supported
  // currently).
  if (personality_encoding{ *lsda_data } == personality_encoding::omit) {
    lsda_data++; // skip omit flag
  } else {
    return info;
  }

  // type table encoding of 0x00 means absolute address
  info.type_encoding = personality_encoding{ *lsda_data++ };
  if (info.type_encoding ==
      personality_encoding::omit) { // omit code: type table
    info.type_table.count = 0;
    info.type_table.size = 0;
  } else {
    info.type_offset = read_uleb128(&lsda_data);
  }

  info.call_site_encoding = personality_encoding{ *lsda_data++ };
  info.call_site.size = read_uleb128(&lsda_data);

  if (info.call_site.size == 0) {
    info.valid = true;
    return info;
  }

  const auto* call_site_end = lsda_data + info.call_site.size;
  const auto* end_of_lsda = lsda_data + info.type_offset;
  info.total_size =
    end_of_lsda - reinterpret_cast<const std::uint8_t*>(top_of_lsda_data);

  // Scan call site
  while (lsda_data < call_site_end) {
    read_encoded_data(&lsda_data, info.call_site_encoding); // start
    read_encoded_data(&lsda_data, info.call_site_encoding); // length
    read_encoded_data(&lsda_data, info.call_site_encoding); // landing pad
    auto action_record = read_uleb128(&lsda_data);          // action offset
    info.max_action = std::max(info.max_action, action_record);
    info.call_site.count++;
  }

  // If the max action record is zero, then there were no entries in the action
  // table, and thus nothing for us to do.
  // If the type table doesn't exist, then the action table shouldn't either.
  //
  // NOTE: This may not be true, but it seems logical that if the type table
  // doesn't exist then the action record should be empty as well, because the
  // call site table should have enough information to jump to the necessary
  // landing pad location for cleanup.
  if (info.max_action == 0 || info.type_offset == 0) {
    info.valid = true;
    return info;
  }

#if 0
  // Subtract one because the action offset is offset by 1, where zero means
  // "install context".
  lsda_data += info.max_action - 1;
#endif

  // Scan action table in reverse from the furthest offset.
  // The action table is
  //
  //       [leb128:filter_number][leb128:offset_to_next]
  //
  // From the end of the call site, scan through the action table and collect
  // the maximum action_value. Use the maximum found action value to determine
  // where the true end of the action table is. The length of the type table is
  // based on the maximum action record.
  while (lsda_data < (end_of_lsda - info.type_table.size)) {
    const std::uint8_t* previous_location = lsda_data;
    auto action_value = read_leb128(&lsda_data); // filter number
    read_leb128(&lsda_data);                     // read offset

    info.action_table.count++;
    info.action_table.size += (lsda_data - previous_location);
    info.type_table.count =
      std::max(static_cast<std::size_t>(action_value), info.type_table.count);

    // Infer the size of the type table entries based on the count and the size
    // of a pointer.
    info.type_table.size = sizeof(std::uintptr_t) * info.type_table.count;
  }

  info.valid = true;

  return info;
}

template<size_t N>
[[gnu::noinline]] std::array<exception_info, N>
generate_meta_info(std::array<void*, N>& p_functions)
{
  std::array<exception_info, N> meta_info{};

  // functions are sorted relative to their position in code just like the
  // exception index.
  std::ranges::sort(p_functions);
  std::span<const arm_index_entry> exception_index(&__exidx_start,
                                                   &__exidx_end);

  auto f_cursor = p_functions.begin();
  auto m_cursor = meta_info.begin();
  auto e_cursor = exception_index.begin();
  while (f_cursor != p_functions.end() && e_cursor != exception_index.end()) {
    void* exception_function = get_function(*e_cursor);

    if (*f_cursor == exception_function) {
      *m_cursor = exception_info(*e_cursor);
      f_cursor++;
      e_cursor++;
      m_cursor++;
    } else if (*f_cursor < exception_function) {
      m_cursor->function_address = *f_cursor;
      m_cursor->rank = metadata_rank::no_entry;
      f_cursor++;
      m_cursor++;
    } else if (*f_cursor > exception_function) {
      e_cursor++;
    }
  }

  return meta_info;
}

template<size_t N>
[[gnu::noinline]] std::array<lsda_info, N>
generate_lsda_info(std::array<exception_info, N>& p_exception_info)
{
  std::array<lsda_info, N> lsda_info_list{};
  auto iter = lsda_info_list.begin();
  for (const auto& element : p_exception_info) {
    *(iter++) = generate_lsda_info(element);
  }
  return lsda_info_list;
}

[[gnu::noinline]] void
throw_something()
{
  throw 5;
}

volatile exception_info* meta_ptr1 = nullptr;
volatile exception_info* meta_ptr2 = nullptr;
volatile lsda_info* lsda_ptr1 = nullptr;
volatile lsda_info* lsda_ptr2 = nullptr;

int
main()
{
  link_in_except_vs_noexcept();

  // Ensure and test that exceptions work in this code.
  try {
    dtor::link_in_dtor_paths();
  } catch (const dtor::action_exception_t&) {
    side_effect[17] = side_effect[17] + 1;
  }

  // Scan exception table and determine which functions have
  std::array noexcept_vs_except{
    // except vs noexcept
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
    to_void(&initialize_noexcept),
    to_void(&my_class::state),
    to_void(&my_class::state_noexcept),
    // external functions
    to_void(&bar),
    to_void(&bar_noexcept),
    to_void(&baz),
    to_void(&baz_noexcept),
    to_void(&qaz),
    to_void(&qaz_noexcept),
    to_void(&main), // NOLINT
  };
  std::array dtor{
    // dtor_paths
    to_void(&dtor::non_trivial_dtor::action),
    to_void(&dtor::non_trivial_dtor::noexcept_action),
    to_void(&dtor::noexcept_calls_all_except),
    to_void(&dtor::noexcept_calls_all_noexcept),
    to_void(&dtor::except_calls_all_except),
    to_void(&dtor::except_calls_all_noexcept),
    to_void(&dtor::noexcept_calls_experiment1),
    to_void(&dtor::noexcept_calls_experiment2),
    to_void(&dtor::noexcept_calls_experiment3),
    to_void(&dtor::noexcept_calls_experiment4),
    to_void(&dtor::noexcept_calls_experiment5),
    to_void(&dtor::noexcept_calls_experiment6),
    to_void(&dtor::noexcept_calls_experiment7),
    to_void(&dtor::except_calls_experiment1),
    to_void(&dtor::except_calls_experiment2),
    to_void(&dtor::except_calls_experiment3),
    to_void(&dtor::except_calls_experiment4),
    to_void(&dtor::except_calls_experiment5),
    to_void(&dtor::except_calls_experiment6),
    to_void(&dtor::except_calls_experiment7),
  };

  try {
    throw_something();
  } catch (...) {
    side_effect[0] = side_effect[0] + 1;
  }

  auto meta_noexcept = generate_meta_info(noexcept_vs_except);
  auto meta_dtor = generate_meta_info(dtor);
  auto noexcept_lsda = generate_lsda_info(meta_noexcept);
  auto dtor_lsda = generate_lsda_info(meta_dtor);

  meta_ptr1 = &meta_noexcept[3];
  meta_ptr2 = &meta_dtor[4];
  lsda_ptr1 = &noexcept_lsda[2];
  lsda_ptr2 = &dtor_lsda[3];

  while (true) {
    continue;
  }

  return 0;
}