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
  TypedParameter::List const &parameters() const { return m_all_params.parameters(); }

private:
  CmdParameters * const m_derived_params;
  CmdParameters m_all_params;
  bool const m_use_num_qpus;
  int output_count = 0;

  void check_params(CmdParameters &params, int argc, char const *argv[]);
  bool process();
  void startPerfCounters();
  void stopPerfCounters();
  void show_help();
};

}  // namespace

#endif  // _EXAMPLE_SUPPORT_SETTINGS_H
