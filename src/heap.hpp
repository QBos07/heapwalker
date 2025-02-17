#include <cstddef>
#include <cstdint>

#pragma once

typedef struct HeapBlock {
  std::size_t size;
  struct HeapBlock *next; // only valid if free block
} HeapBlock;

#ifndef __clang__ // clangd uses host type sizes
static_assert(sizeof(HeapBlock) == 8, "block size does not match");
#endif

const auto free_list_head = reinterpret_cast<HeapBlock **>(0x8C1A35F0);
const auto heap_ptr = reinterpret_cast<void **>(0x8C2233A0);
const auto heap_max_barrier = reinterpret_cast<void *>(0x8CE80001);
const auto heap_start = reinterpret_cast<void *>(0x8C280000);

constexpr std::size_t PAGE_SIZE = 1024;
constexpr std::size_t MALLOC_ALIGN = 4;
constexpr std::size_t MAX_BLOCK_SIZE = 0x7FFFFFFF;
constexpr std::size_t HEADER_SIZE = sizeof(HeapBlock::size);
const std::size_t HEAP_SIZE =
    (reinterpret_cast<std::uintptr_t>(heap_max_barrier) - 1) -
    reinterpret_cast<std::uintptr_t>(heap_start);

namespace os {
// includes error handling
const auto malloc = reinterpret_cast<void *(*)(size_t)>(0x800CFB00);

// internal function called by malloc
const auto allocate_memory = reinterpret_cast<void *(*)(size_t)>(0x800A7794);

// gets you 1 or more pages of fresh heap memory
const auto request_more_memory = reinterpret_cast<void *(*)()>(0x800A7824);

// gets you the requested amount of fresh heap bytes
const auto system_allocate = reinterpret_cast<void *(*)(size_t)>(0x800CDEC6);

// adds the memory back for malloc the be used
const auto free = reinterpret_cast<void (*)(void *)>(0x800A76FC);

// only here for completeness, DO NOT USE
const auto init_heap = reinterpret_cast<void (*)()>(0x800CDEEC);

inline bool donate_memory(size_t size, void *ptr) {
  if (size < 8)
    return false;

  auto mem = reinterpret_cast<HeapBlock *>(ptr);
  mem->size = size - sizeof(mem->size);
  os::free(&mem->next);

  return true;
};
} // namespace os