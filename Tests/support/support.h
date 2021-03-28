//
// Support code for tests
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _TEST_SUPPORT_SUPPORT_H
#define _TEST_SUPPORT_SUPPORT_H
#include <cstdio>
#include <vector>
#include <stdint.h>
#include "../catch.hpp"
#include "Support/basics.h"
#include "Source/Int.h"
#include "Source/Float.h"
#include "v3d/Driver.h"
#include "v3d/instr/Instr.h"
#include "v3d/BufferObject.h"

double get_time();
bool running_on_v3d();

void match_kernel_outputs(
  std::vector<uint64_t> const &expected,
  std::vector<uint64_t> const &received,
  bool skip_nops = false);


template<typename T>
void dump_data(T const &arr, bool do_all = false, bool as_float = false) {
  int const DISP_LENGTH = 4;

  char const *format = nullptr;
  if (as_float) {
    // floats get promoted to doubles in printf, hence both '%f'
    if (sizeof(arr[0]) == 8) {
      format = "%8d: 0x%016llx - %f\n";
    } else {
      format = "%8d: 0x%08x - %f\n";
    }
  } else {
    // assume type is int
    if (sizeof(arr[0]) == 8) {
      format = "%8d: 0x%016llx - %lld\n";
    } else {
      format = "%8d: 0x%08x - %d\n";
    }
  }
  assert(format != nullptr);

  auto print = [&arr, format] (int offset) {
    auto val = arr[offset];

    if (sizeof(arr[0]) == 8) {
      int64_t *bits = (int64_t*) &val;
      printf(format, offset, *bits, arr[offset]);
    } else {
      // assume 4
      int32_t *bits = (int32_t*) &val;
      printf(format, offset, *bits, arr[offset]);
    }
  };

  int first_size = (int) arr.size();

  if (do_all) {
    for (int offset = 0; offset < first_size; ++offset) {
      print(offset);
    }
    return;
  }


  if (first_size > DISP_LENGTH) {
    first_size = DISP_LENGTH;
  }
    
  for (int offset = 0; offset < first_size; ++offset) {
    print(offset);
  }

  if (first_size == (int) arr.size()) {
    return;
  }

  printf("      ...\n");

  for (int offset = arr.size() - DISP_LENGTH; offset < (int) arr.size(); ++offset) {
    print(offset);
  }

  printf("\n");
}


void dump_array(float *a, int size, int linesize = -1);
std::string dump_array2(float *a, int size, int linesize = -1);
void dump_array(V3DLib::Float::Array const &a, int linesize = -1);
std::string dump_array2(V3DLib::Float::Array const &a, int linesize = -1);
void dump_array(V3DLib::Int::Array const &a, int linesize = -1);


extern const char *SUDO;

void make_test_dir();

#endif  //  _TEST_SUPPORT_SUPPORT_H
