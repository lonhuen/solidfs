#pragma once

#include <sstream>
#include <string>

// mainly taken from a private repo called "nova"

namespace solid {
  class String {
  public:
    template<typename A, typename ... ArgT>
    static std::string of(const A &first, const ArgT &... args) {
      std::ostringstream oss;
      of(oss, first, args ...);
      return std::string(oss.str());
    }

    template<typename A>
    static std::string of(const A &first) {
      std::ostringstream oss;
      of(oss, first);
      return std::string(oss.str());
    }

  protected:
    template<typename A, typename ... ArgT>
    static void of(std::ostringstream &oss, const A &first, const ArgT &... args) {
      oss << first;
      of(oss, args ...);
    }

    template<typename A>
    static void of(std::ostringstream &oss, const A &first) {
      oss << first;
    }
  };
}
