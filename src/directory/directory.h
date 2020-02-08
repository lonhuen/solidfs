#pragma once

#include "common.h"
#include <unordered_map>
#include <string>
#include <map>

namespace solid {
    //TODO(lonhh): do we need move constructor here?
    class Directory {
    public:
        INodeID id;
        std::unordered_map<std::string, INodeID> entry_m;

        Directory();
        Directory(INodeID id, INodeID parent);
        Directory(INodeID id,const uint8_t* byte_stream, uint32_t size);
        void insert_entry(const std::string& s,INodeID id);
        void remove_entry(const std::string& s);
        bool contain_entry(const std::string& s) const;
        INodeID get_entry(const std::string& s) const;

        int serialize(uint8_t* byte_stream, uint32_t size);
        int deserialize(const uint8_t* byte_stream, uint32_t size);
    };

};