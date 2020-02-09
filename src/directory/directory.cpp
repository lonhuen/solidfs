#include <algorithm>
#include "directory/directory.h"
#include "utils/log_utils.h"
#include "utils/fs_exception.h"

namespace solid {
    Directory::Directory() {}
    Directory::Directory(INodeID id, INodeID parent): entry_m({{".",id},{"..",parent}}),id(id) {
    }
    Directory::Directory(INodeID id,const uint8_t* byte_stream, uint32_t size): id(id) {
        deserialize(byte_stream,size);
    }
    void Directory::insert_entry(const std::string& s,INodeID id) {
        // TODO(lonhh) sanity check
        if(contain_entry(s)) {
            throw fs_exception(std::errc::file_exists,
                "@insert_entry Already exists ", s, " in directory ", id);
        }
        this->entry_m[s] = id;
    }
    void Directory::remove_entry(const std::string& s) {
        if(!contain_entry(s)) {
            throw fs_exception(std::errc::no_such_file_or_directory,
                "@remove_entry No such file/directory ", s, " in directory ", id);
        }
        this->entry_m.erase(s);

    }
    bool Directory::contain_entry(const std::string& s) const {
        if(this->entry_m.find(s) != this->entry_m.end()) {
            return true;
        }
        return false;
    }

    INodeID Directory::get_entry(const std::string& s) const {
        auto p = this->entry_m.find(s);
        if(p == this->entry_m.end()){
            throw fs_exception(std::errc::no_such_file_or_directory,
                "@get_entry No such file/directory ", s, " in directory ", id);
        }
        return p->second;
    }

    /**
     * @brief serialize the directory to bytes
     * @return 0 if not enough space, size of bytes returned otherwise
    */
    int Directory::serialize(uint8_t* byte_stream, uint32_t size) {
        uint32_t s = 0;
        for(auto p=entry_m.begin();p!=entry_m.end();p++) {
            if( s + p->first.length() + sizeof(INodeID) + 1 <= size) {
                memcpy(byte_stream+s,p->first.c_str(),p->first.length() + 1);
                s += p->first.length() + 1;
                memcpy(byte_stream+s,&(p->second),sizeof(INodeID));
                s += sizeof(INodeID);
            } else {
                LOG(WARNING) << "Not enough space for serializing Directory";
                return 0;
            }
        }
        //memset(byte_stream+s,0,size - s);
        return s;
    }

    int Directory::deserialize(const uint8_t* byte_stream, uint32_t size) {
        // TODO(lonhh): whether we need to clear the entries?
        // entry_m.clear();
        uint32_t i = 0;
        while(i < size) {
            if(byte_stream[i] == '\0')
                return i;
            std::string s((const char*)byte_stream+i);
            INodeID id = *((INodeID*)(byte_stream+i+s.length()+1));
            //insert_entry(s,i);
            entry_m[s] = id;
            i += s.length() + 1 + sizeof(INodeID);
        }
        return i;
    }
};