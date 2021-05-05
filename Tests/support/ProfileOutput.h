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

  void use_max_qpus(bool val) { m_use_max_qpus = val; }
  void show_compile(bool val) { ShowCompile = val; }
  void add_compile(std::string const &label, std::string const &timer_val, int Dim);
  void add_compile(std::string const &label, Timer &timer, int Dim);
  std::string dump();

  static std::string header();

  template<typename KernelType>
  void run(KernelType &k, int Dim, std::string const &label, std::function<void(int numQPUs)> f) {
    for (auto num : num_qpus()) {
      Timer timer;

      for (int i = 0; i < num_iterations; i++) {
        f(num);
      }
      add_call(label, timer, Dim, num);
    }
  };


  template<typename KernelType>
  void run(KernelType &k, int Dim, std::string const &label) {
    for (auto num : num_qpus()) {
      k.setNumQPUs(num);
      Timer timer;

      for (int i = 0; i < num_iterations; i++) {
        k.call();
      }
      add_call(label, timer, Dim, num);
    }
  }

private:
  std::vector<int> num_qpus() const;
  bool ShowCompile = false;
  bool m_use_max_qpus = false;
  std::vector<out_data> output;

  void add_call(std::string const &label, Timer &timer, int Dim, int num_qpus);
};

#endif  // _TEST_SUPPORT_PROFILEOUTPUT_H
