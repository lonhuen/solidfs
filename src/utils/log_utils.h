#pragma once

#include <glog/logging.h>
#include <cstring>
#include <map>

class LogUtils {
public:
  static bool initialized;
  static std::string log_buf_level;
  static std::string log_dir;
  static std::string log_level;
  static bool log_to_stderr;

  static const std::map<std::string, int> log_level_mapping;

  static void init(const char* program_name) {
    if (initialized) {
      return;
    }
    FLAGS_logtostderr = log_to_stderr;
    if (!log_dir.empty()) {
      FLAGS_log_dir = log_dir;
    }
    FLAGS_minloglevel = log_level_mapping.at(log_level);
    FLAGS_logbuflevel = log_level_mapping.at(log_buf_level);
    google::InitGoogleLogging(program_name);
    initialized = true;
  }

};