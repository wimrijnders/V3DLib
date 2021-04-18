#ifndef _EXAMPLE_SUPPORT_SETTINGS_H
#define _EXAMPLE_SUPPORT_SETTINGS_H
#include <string>
#include <CmdParameters.h>

namespace V3DLib {

class BaseKernel;

struct Settings {
  std::string name;

  bool output_code;
  bool compile_only;
  bool silent;
  int  run_type;
  int  num_qpus = 1;
#ifdef QPU_MODE
  bool   show_perf_counters;
#endif  // QPU_MODE

  Settings(CmdParameters *derived_params = nullptr, bool use_num_qpus = false);

  void init(int argc, const char *argv[]);
  void process(BaseKernel &k);
  virtual bool init_params() { return true; }

private:
  CmdParameters * const m_derived_params;
  bool const m_use_num_qpus;
  int output_count = 0;

  void set_name(const char *in_name);
  bool process(CmdParameters &in_params);
  CmdParameters &base_params();
  void startPerfCounters();
  void stopPerfCounters();
};


}  // namespace

#endif  // _EXAMPLE_SUPPORT_SETTINGS_H
