#include "heap.hpp"
#include "hhk_functions.h"
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>

uint32_t traverse_heap(HeapBlock *start) {
  auto ptr = start;

  uint32_t i = 0;
  while (true) {
    if (ptr < heap_start && ptr >= heap_max_barrier) {
      std::printf("Pointer %p out of heap! END\n",
                  reinterpret_cast<void *>(ptr));
      return i;
    }

    if (reinterpret_cast<intptr_t>(ptr) % alignof(decltype(ptr))) {
      std::printf("Pointer %p not aligned (%zu)! END\n",
                  reinterpret_cast<void *>(ptr), alignof(decltype(ptr)));
      return i;
    }

    /*if (i % 50 == 0 && i != 0) {
      std::fflush(stdout);
      Debug_WaitKey();
    }*/

    // std::printf("%p: %zu\n", reinterpret_cast<void *>(ptr), ptr->size);
    if (!ptr->size) {
      std::printf("Size 0! END\n");
      return i;
    }
    ptr = reinterpret_cast<HeapBlock *>(reinterpret_cast<std::intptr_t>(ptr) +
                                        ptr->size + sizeof(ptr->size));
    i++;
  }
}

uint32_t traverse_free_list(HeapBlock *start) {
  auto ptr = start;

  uint32_t i = 0;
  while (true) {
    if (!ptr) {
      std::printf("Reached the end!\n");
      return i;
    }

    if (reinterpret_cast<intptr_t>(ptr) % alignof(decltype(ptr))) {
      std::printf("Pointer %p not aligned (%zu)! END\n",
                  reinterpret_cast<void *>(ptr), alignof(decltype(ptr)));
      return i;
    }

    if (i % 50 == 0 && i != 0) {
      std::fflush(stdout);
      Debug_WaitKey();
    }

    std::printf("%p: %zu\n", reinterpret_cast<void *>(ptr), ptr->size);
    ptr = ptr->next;
    i++;
  }
}

int main() {
  std::printf("QBos07's HeapWalker!\n");
  std::printf("Heap end: %p\n", *heap_ptr);
  std::printf("Heap size: %zu (%zu free)\n",
              reinterpret_cast<std::intptr_t>(*heap_ptr) -
                  reinterpret_cast<std::intptr_t>(heap_start),
              (reinterpret_cast<std::intptr_t>(heap_max_barrier) - 1) -
                  reinterpret_cast<std::intptr_t>(*heap_ptr));
  {
    std::printf("Traversing Heap from start:\n");
    auto depth = traverse_heap(reinterpret_cast<HeapBlock *>(heap_start));
    std::printf("%" PRIu32 " blocks\n", depth);
  }
  {
    //auto free_list_head = reinterpret_cast<HeapBlock *>(reinterpret_cast<intptr_t>(os::allocate_memory(0)) + HEADER_SIZE);
    std::printf("Prepare Heap: \n");
    auto small = os::malloc(1024);
    asm volatile ("" : : "r" (small)); // block
    std::printf("malloc(1k) = %p\n", small);
    auto big = os::malloc(4096);
    asm volatile ("" : : "r" (small), "r" (big)); // block
    std::printf("malloc(4k) = %p\n", big);
    os::free(small);
    std::printf("free(small)\n");
    auto donate = reinterpret_cast<void *>(reinterpret_cast<intptr_t>(big) + 1024);
    os::donate_memory(2048, donate);
    std::printf("donate(2k, big+1k)\n");
    std::printf("Traversing free list:\n");
    auto depth =
        traverse_free_list(reinterpret_cast<HeapBlock *>(*free_list_head));
    std::printf("%" PRIu32 " blocks\n", depth);
    os::donate_memory(1024, big);
    std::printf("donate(1k, big)\n");
    os::donate_memory(2048, reinterpret_cast<void *>(reinterpret_cast<intptr_t>(donate) + 2048));
    std::printf("donate(1k, big+3k)\n");
  }

  std::fflush(stdout);
  Debug_WaitKey();
}