#include "V3DLib.h"
#include "vc4/DMA/Operations.h"
#include "Support/Settings.h"

using namespace V3DLib;

V3DLib::Settings settings;

void dma(Int::Ptr p) {
  // Setup load of 16 vectors into VPM, starting at word address 0
  dmaSetReadPitch(64);
  dmaSetupRead(HORIZ, 16, 0);
  // Start loading from memory at address 'p'
  dmaStartRead(p);
  // Wait until load complete
  dmaWaitRead();

  // Setup load of 16 vectors from VPM, starting at vector address 0
  vpmSetupRead(HORIZ, 16, 0);
  // Setup store to VPM, starting at vector address 16
  vpmSetupWrite(HORIZ, 16);

  // Read each vector, increment it, and write it back
  for (int i = 0; i < 16; i++)
    vpmPut(vpmGetInt() + 1);

  // Setup store of 16 vectors into VPM, starting at word address 256
  dmaSetupWrite(HORIZ, 16, 256);
  // Start writing to memory at address 'p'
  dmaStartWrite(p);
  // Wait until store complete
  dmaWaitWrite();
}


int main(int argc, const char *argv[]) {
  auto ret = settings.init(argc, argv);
  if (ret != CmdParameters::ALL_IS_WELL) return ret;

  if (!Platform::has_vc4() && settings.run_type == 0) {
    printf("\nThe DMA example does not work on v3d, it is only meant for vc4.\n"
           "It will only work for the emulator on v3d.\n\n");
    return 1;
  }

  // Construct kernel
  auto k = compile(dma, true);  // true: only compile for vc4

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(256);
  for (int i = 0; i < 256; i++)
    array[i] = i;

  // Invoke the kernel
  k.load(&array);  
  settings.process(k);  

  // Display the result
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      printf("%i ", array[16*i + j]);
    }

    printf("\n");
  }
  
  return 0;
}
