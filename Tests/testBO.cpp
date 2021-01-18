#include "catch.hpp"
#include "Common/SharedArray.h"
#include "Target/BufferObject.h"


TEST_CASE("Test Buffer Objects", "[bo]") {
  using SharedArray  = V3DLib::SharedArray<uint32_t>;
  using SharedArrays = std::vector<std::unique_ptr<SharedArray>>;

  V3DLib::emu::BufferObject heap(1024*1024);  // Using in-memory version to avoid having to use devices

  auto init_arrays = [&heap] (SharedArrays &arrays, int size) {
    for (int i = 0; i < size; ++i) {
      arrays[i].reset(new SharedArray(1024, heap));
    }
  };
    

  SECTION("Freeing up a BO in a controlled manner should work") {
    const int NUM_ARRAYS = 7;

    REQUIRE(heap.empty());

    SharedArrays arrays(NUM_ARRAYS);
    init_arrays(arrays, NUM_ARRAYS);

    REQUIRE(!heap.empty());

    // The goal is to touch all possible paths within BufferObject::dealloc_array()
    arrays[0]->dealloc();
    arrays[6]->dealloc();
    arrays[1]->dealloc();

    arrays[0]->alloc(1024); // Trigger reclaim of freed memory
    arrays[6]->alloc(1024); // idem

    arrays[5]->dealloc();
    arrays[3]->dealloc();
    arrays[2]->dealloc();
    arrays[4]->dealloc();
    arrays[0]->dealloc();
    arrays[6]->dealloc();

    REQUIRE(heap.empty());
  }


  SECTION("BO should be empty when SharedArray instances go out of scope") {
    const int NUM_ARRAYS = 5;

    {
      REQUIRE(heap.empty());

      SharedArrays arrays(NUM_ARRAYS);
      init_arrays(arrays, NUM_ARRAYS);

      REQUIRE(!heap.empty());

      // Dealloc some but not all
      arrays[0]->dealloc();
      arrays[4]->dealloc();
      arrays[2]->dealloc();

      REQUIRE(!heap.empty());
    }

    REQUIRE(heap.empty());  // Whatever happens, after SharedArray instances are gone, heap should be empty
  }


  SECTION("BO should survive chaotic assignment of SharedArray instances") {
    const int NUM_PASSES = 200;  // Not too big, to prevent heap overflow
    const int NUM_ARRAYS = 10;

    REQUIRE(heap.empty());

    {
      SharedArrays arrays(NUM_ARRAYS);
      init_arrays(arrays, NUM_ARRAYS);
      REQUIRE(!heap.empty());

      // Hoping for no assertions while running this
      srand((unsigned) time(0));
      for (int n = 0; n < NUM_PASSES; ++n) {
        uint32_t index = rand() % NUM_ARRAYS;
        uint32_t size_alloc = 1 +  rand() % 2048;

        auto &arr = *arrays[index];

        if (arr.size() == 0) {
          arr.alloc(size_alloc);
        } else {
          arr.dealloc();
          REQUIRE(arr.size() == 0);
        }
      }
    }

    REQUIRE(heap.empty());  // Should be all cleaned up
  }


  SECTION("Heap view should not be marked as freed space") {
    {
      SharedArray view;
      view.heap_view(heap);

      REQUIRE(heap.empty());
      view.dealloc();
      REQUIRE(heap.empty());
      REQUIRE(heap.num_free_ranges() == 0);
    }

    {
      SharedArray view;
      view.heap_view(heap);
      REQUIRE(heap.empty());

      SharedArray arr1(64, heap);
      SharedArray arr2(64, heap);
      REQUIRE(!heap.empty());

      arr1.dealloc();
      REQUIRE(!heap.empty());
      REQUIRE(heap.num_free_ranges() == 1);

      view.dealloc();
      REQUIRE(!heap.empty());
      REQUIRE(heap.num_free_ranges() == 1);

      arr2.dealloc();
      REQUIRE(heap.empty());
      REQUIRE(heap.num_free_ranges() == 0);
    }
  }
}
