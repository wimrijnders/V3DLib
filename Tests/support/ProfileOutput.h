#ifndef _TEST_SUPPORT_PROFILEOUTPUT_H
#define _TEST_SUPPORT_PROFILEOUTPUT_H
#include <cassert>
#include <vector>
#include <string>
#include <functional>
#include "Support/Timer.h"
#include "Support/Platform.h"

class ProfileOutput {
  using Timer = V3DLib::Timer;
  using Platform = V3DLib::Platform;

  struct out_data {
    out_data(
      std::string const &in_label,
      std::string const &in_timer,
      int in_Dim,
      int in_num_qpus
    ) : label(in_label), Dim(in_Dim), num_qpus(in_num_qpus), timer(in_timer) {}

    static std::string header();

    std::string str() const;


    std::string label;
    int Dim;
    int num_qpus;
    std::string timer;
  };

public:
  enum {
    num_iterations = 10
  };

  void use_single_qpu(bool val)  { m_use_single_qpu = val; }
  void use_max_qpus(bool val)    { m_use_max_qpus = val; }
  void show_compile(bool val)    { ShowCompile = val; }
  void add_compile(std::string const &label, std::string const &timer_val, int Dim);
  void add_compile(std::string const &label, Timer &timer, int Dim);
  void run(int Dim, std::string const &label, std::function<void(int numQPUs)> f);
  std::string dump();

  static std::string header();


private:
  std::vector<int> num_qpus() const;
  bool ShowCompile      = false;
  bool m_use_single_qpu = false;  // Takes precedence over m_use_max_qpus
  bool m_use_max_qpus   = false;
  std::vector<out_data> output;

  void add_call(std::string const &label, Timer &timer, int Dim, int num_qpus);
};

#endif  // _TEST_SUPPORT_PROFILEOUTPUT_H
