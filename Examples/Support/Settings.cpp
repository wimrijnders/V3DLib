//
// Basic command line handling for the example programs.
//
// This contains common functionality for all examples.
//
///////////////////////////////////////////////////////////////////////////////
#include "Settings.h"
#include <memory>
#include <iostream>
#include "Kernel.h"

#ifdef QPU_MODE
#include "Support/Platform.h"
#include "vc4/PerformanceCounters.h"
#include "v3d/PerformanceCounters.h"

#endif  // QPU_MODE

namespace {

/**
 * Get the base filename, without extension from a path
 *
 * Source: https://stackoverflow.com/a/8520815
 *
 * In C++17, you can use `stem()`: https://en.cppreference.com/w/cpp/filesystem/path/stem
 */
std::string stem(const char *input) {
  std::string filename(input);

  // Remove directory if present.
  // Do this before extension removal incase directory has a period character.
  const size_t last_slash_idx = filename.find_last_of("\\/");
  if (std::string::npos != last_slash_idx)
  {
    filename.erase(0, last_slash_idx + 1);
  }

  // Remove extension if present.
  const size_t period_idx = filename.rfind('.');
  if (std::string::npos != period_idx)
  {
    filename.erase(period_idx);
  }

  return filename;
}


// ============================================================================
// Settings 
// ============================================================================

const char *blurb =
  "Example Program\n"      // TODO Perhaps fill this in dynamically
#ifdef EMULATION_MODE
  "\nRunning in emulation mode.\n"
#ifdef QPU_MODE
  "\nRunning in QPU mode AND  emulation mode! This will likely lead to segmentation faults.\n"
#endif
#endif
#ifdef QPU_MODE
  "\nRunning in QPU mode.\n"
#endif
;


CmdParameters base_params = {
  blurb,
  {{
    "Output Generated Code",
    "-f",
    ParamType::NONE,     // Prefix needed to dsambiguate
    "Write representations of the generated code to file"
  }, { 
    "Compile Only",
    "-c",
    ParamType::NONE,     // Prefix needed to dsambiguate
    "Compile the kernel but do not run it"
  }, {
    "Select run type",
    "-r=",
    {"default", "emulator", "interpreter"},
    "Run the kernel on the QPU, emulator or on the interpreter"
  }, {
    "Disable logging",
    { "-s", "-silent" },
    ParamType::NONE,     // Prefix needed to dsambiguate
    "Do not show the logging output on standard output"
#ifdef QPU_MODE
    }, {
    "Performance Counters",
    "-pc",
    ParamType::NONE,
    "Show the values of the performance counters (vc4 only)"
#endif  // QPU_MODE
  }}
};


CmdParameters numqpu_params = {
  "",
  {{
    "Num QPU's",
    "-n=",
    INTEGER,
    "Number of QPU's to use. The values depends on the platform being run on:\n"
    "  - vc4 (Pi3+ and earlier), emulator: an integer value from 1 to 12 (inclusive)\n"
    "  - v3d (Pi4)                       : 1 or 8\n",
    1
  }}
};


std::unique_ptr<CmdParameters> params;

CmdParameters &instance(bool use_numqpus = false) {
  if (!params) {
    CmdParameters *p = new CmdParameters(base_params);

    if (use_numqpus) {
      p->add(numqpu_params);
    }

    params.reset(p);
  }

  return *params;
}

}  // anon namespace


namespace V3DLib {

Settings::Settings(CmdParameters *derived_params, bool use_num_qpus) :
  m_derived_params(derived_params),
  m_use_num_qpus(use_num_qpus)
{}


CmdParameters &Settings::base_params() {
  return instance(m_use_num_qpus);
}


int Settings::init(int argc, const char *argv[]) {
  int ret = CmdParameters::ALL_IS_WELL;

  set_name(argv[0]);

  CmdParameters *params = nullptr;
  if (m_derived_params != nullptr) {
    m_derived_params->add(base_params());
    params = m_derived_params;
  } else {
    params = &instance();
  }

  if (params->has_errors()) {
    std::cout << params->get_errors();
    ret = CmdParameters::EXIT_ERROR;
  } else {
    ret = params->handle_commandline(argc, argv, false);

    if (ret == CmdParameters::ALL_IS_WELL) {
      if (process(*params)) {
        init_params();  // Set derived param's , if present
      } else {
        ret = CmdParameters::EXIT_ERROR;
      }
    }
  }

  return ret;
}


void Settings::set_name(const char *in_name) {
  assert(in_name != nullptr);
  name = stem(in_name);
}


bool Settings::process(CmdParameters &in_params) {
  output_code  = in_params.parameters()["Output Generated Code"]->get_bool_value();
  compile_only = in_params.parameters()["Compile Only"]->get_bool_value();
  silent       = in_params.parameters()["Disable logging"]->get_bool_value();
  run_type     = in_params.parameters()["Select run type"]->get_int_value();
#ifdef QPU_MODE
  show_perf_counters = in_params.parameters()["Performance Counters"]->get_bool_value();
#endif  // QPU_MODE

  if (m_use_num_qpus) {
    num_qpus    = in_params.parameters()["Num QPU's"]->get_int_value();

    if (run_type != 0 || Platform::instance().has_vc4) {  // vc4 only
      if (num_qpus < 0 || num_qpus > 12) {
        printf("ERROR: For vc4 and emulator, the number of QPU's selected must be between 1 and 12 inclusive.\n");
        return false;
      }
    } else {
      if (num_qpus != 1 && num_qpus != 8) {
        printf("ERROR: For v3d, the number of QPU's selected must be 1 or 8.\n");
        return false;
      }
    }
  }

  if (silent) {
    disable_logging();
  }

  if (compile_only || run_type != 0) {
    Platform::use_main_memory(true);
  }

  return true;
}


/**
 * @brief Performance Counters: Enable the counters we are interested in
 */
void Settings::startPerfCounters() {
  //printf("Entered Settings::startPerfCounters()\n");

#ifdef QPU_MODE
  if (!show_perf_counters) return;

  using PC = V3DLib::vc4::PerformanceCounters;
 
  if (Platform::instance().has_vc4) {
    PC::enable({
      PC::QPU_INSTRUCTIONS,
      PC::QPU_STALLED_TMU,
      PC::L2C_CACHE_HITS,
      PC::L2C_CACHE_MISSES,
      PC::QPU_INSTRUCTION_CACHE_HITS,
      PC::QPU_INSTRUCTION_CACHE_MISSES,
      PC::QPU_CACHE_HITS,
      PC::QPU_CACHE_MISSES,
      PC::QPU_IDLE,
    });
  } else {
    using PC3 = V3DLib::v3d::PerformanceCounters;

    PC3::enter({
      // vc4 counters, check if same and working.
      // They work, but overlap is hard to detect with vc4.
      PC::QPU_INSTRUCTIONS,
      PC::QPU_STALLED_TMU,
      PC::L2C_CACHE_HITS,
      PC::L2C_CACHE_MISSES,
      PC::QPU_INSTRUCTION_CACHE_HITS,
      PC::QPU_INSTRUCTION_CACHE_MISSES,
      PC::QPU_CACHE_HITS,
      PC::QPU_CACHE_MISSES,
      PC::QPU_IDLE,

      PC3::CORE_PCTR_CYCLE_COUNT,  // specific for v3d
      // CHECKED for <= 40
    });
  }
#endif
}


void Settings::stopPerfCounters() {
#ifdef QPU_MODE
  if (!show_perf_counters) return;
 
  std::string output;

  if (Platform::instance().has_vc4) {
    // Show values current counters
    using PC = V3DLib::vc4::PerformanceCounters;

    output = PC::showEnabled();
  } else {
    using PC = V3DLib::v3d::PerformanceCounters;
    output = PC::showEnabled();
  }

  printf("%s\n", output.c_str());
#endif
}


void Settings::process(KernelBase &k) {
  startPerfCounters();

  if (!compile_only) {
    switch (run_type) {
      case 0: k.call(); break;
      case 1: k.emu(); break;
      case 2: k.interpret(); break;
    }
  }

  stopPerfCounters();

  // NOTE: For multiple calls here (entirely possible, HeatMap does this),
  //       this will dump the v3d code (mnemonics, actually) on every call.
  if (output_code) {
    if (output_count == 0) {
      assert(!name.empty());
      std::string code_filename = name + "_code.txt";

      bool output_for_vc4 = Platform::instance().has_vc4 || (run_type != 0);
      k.pretty(output_for_vc4, code_filename.c_str());
    } else if (output_count == 1) {
      warning("Not outputting code more than once");
    }

    output_count++;
  }
}

}  // namespace V3DLib;
