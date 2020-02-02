#include "utils/log_utils.h"

std::string LogUtils::log_dir = "";
std::string LogUtils::log_level = "2";
std::string LogUtils::log_buf_level = "3";
bool LogUtils::log_to_stderr = true;

const std::map<std::string, int> LogUtils::log_level_mapping = {
  {"INFO",    0},
  {"WARNING", 1},
  {"ERROR",   2},
  {"FATAL",   3},
  {"0",       0},
  {"1",       1},
  {"2",       2},
  {"3",       3}
};

bool LogUtils::initialized = false;