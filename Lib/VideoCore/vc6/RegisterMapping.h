#ifndef _VC6_REGISTERMAPPING_H
#define _VC6_REGISTERMAPPING_H
#include <stdint.h>


namespace QPULib {
namespace vc6 {

class RegisterMapping {
public:
	~RegisterMapping();

	void init();

	unsigned num_cores(); 

	struct CoreInfo {
		unsigned index      = 0;
		unsigned vpm_size   = 0;
		unsigned num_slice  = 0;
		unsigned num_tmu    = 0;  // all slices
		unsigned num_qpu    = 0;  // all slices
	};

	CoreInfo core_info(unsigned core_index);

private:
  uint32_t *m_addr = nullptr;
	unsigned  m_size = 0x4000;

	unsigned  m_ncores     = 1;
  uint32_t *map_cores[1] = { nullptr }; 
};


}  // vc6
}  // QPULib

#endif  // _VC6_REGISTERMAPPING_H
