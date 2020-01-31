#pragma once

#include "common.h"
#include <unordered_map>
#include <string>
#include <map>

class Directory {
    // unordered_map is more efficient for pure entity look up
    //std::unordered_map<std::string, int> entry_m;
    std::map<std::string, iid_t> entry_m;

    public:
    Directory(iid_t id, iid_t parent);
    int insert_entry(const std::string& s,iid_t id);
    int remove_entry(const std::string& s);
    int contains_entry(const std::string& s) const;
};