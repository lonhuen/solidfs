#pragma once

#include "common.h"
#include <unordered_map>
#include <string>
#include <map>

//TODO(lonhh): do we need move constructor here?
class Directory {
    // unordered_map is more efficient for pure entity look up
    //std::unordered_map<std::string, int> entry_m;
    iid_t id;

    public:
    //TODO(lonhh): this should not be public
    std::unordered_map<std::string, iid_t> entry_m;
    Directory();
    Directory(iid_t id, iid_t parent);
    Directory(iid_t id,const uint8_t* byte_stream, uint32_t size);
    int insert_entry(const std::string& s,iid_t id);
    int remove_entry(const std::string& s);
    int contain_entry(const std::string& s) const;
    int get_entry(const std::string& s, iid_t* ret) const;

    int serialize(uint8_t* byte_stream, uint32_t size);
    int deserialize(const uint8_t* byte_stream, uint32_t size);
};