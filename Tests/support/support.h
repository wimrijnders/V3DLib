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
#include "Common/SharedArray.h"
#include "v3d/Driver.h"
#include "v3d/instr/Instr.h"
#include "v3d/BufferObject.h"

using ByteCode = std::vector<uint64_t>; 

template<typename T>
using SharedArray = QPULib::SharedArray<T>;

double get_time();
bool running_on_v3d();

void match_kernel_outputs(
	std::vector<uint64_t> const &expected,
	std::vector<uint64_t> const &received,
	bool skip_nops = false);


template<typename T>
void dump_data(T const &arr, bool do_all = false) {
	int const DISP_LENGTH = 4;
	char const *format = "%8d: 0x%x - %d\n";

	if (sizeof(arr[0]) == 8) {
		format = "%8d: 0x%llx - %lld\n";
	}

	int first_size = (int) arr.size();

	if (do_all) {
		for (int offset = 0; offset < first_size; ++offset) {
			printf(format, offset, arr[offset], arr[offset]);
		}
		return;
	}


	if (first_size > DISP_LENGTH) {
		first_size = DISP_LENGTH;
	}
		
	for (int offset = 0; offset < first_size; ++offset) {
		printf(format, offset, arr[offset], arr[offset]);
	}

	if (first_size == arr.size()) {
		return;
	}

	printf("      ...\n");

	for (int offset = arr.size() - DISP_LENGTH; offset < (int) arr.size(); ++offset) {
		printf(format, offset, arr[offset], arr[offset]);
	}

	printf("\n");
}

#endif  //  _TEST_SUPPORT_SUPPORT_H
