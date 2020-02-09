#pragma once

#include <sstream>
#include <string>
#include "inode/inode.h"

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
    
    static std::string of(const INode &first) {
      std::ostringstream oss;
      oss << "{\n";
      oss << "\tinode_number:\t" << first.inode_number << "\n";
      oss << "\tmode:\t" << first.mode << "\n";
      oss << "\tlinks:\t" << first.links<< "\n";
      oss << "\tuid:\t" << first.uid<< "\n";
      oss << "\tgid:\t" << first.gid<< "\n";
      oss << "\tsize:\t" << first.size<< "\n";
      oss << "\tblock:\t" << first.block<< "\n";
      oss << "\tatime:\t" << first.atime<< "\n";
      oss << "\tctime:\t" << first.ctime<< "\n";
      oss << "\tmtime:\t" << first.mtime<< "\n";
      oss << "\titype:\t" << first.itype<< "\n";
      oss << "}\n";
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
#include "utils/iostream_utils.h"