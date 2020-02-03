#include <algorithm>
#include "directory/directory.h"
#include "utils/log_utils.h"

Directory::Directory(iid_t id, iid_t parent): entry_m({{".",id},{"..",parent}}),id(id) {
}
Directory::Directory(iid_t id,const uint8_t* byte_stream, uint32_t size): id(id) {
    deserialize(byte_stream,size);
}
int Directory::insert_entry(const std::string& s,iid_t id) {
    // TODO(lonhh) sanity check
    if(contain_entry(s)) {
        LOG(INFO) << "trying to insert the same entry " << s << " to directory";
        return 0;
    }
    this->entry_m[s] = id;
    return 1;
}
int Directory::remove_entry(const std::string& s) {
    if(!contain_entry(s)) {
        LOG(ERROR) << "trying to remove a non-exist entry " << s; 
        return 0;
    }
    // TODO(lonhh) sanity check
    this->entry_m.erase(s);
    return 1;

}
int Directory::contain_entry(const std::string& s) const {
    if(this->entry_m.find(s) != this->entry_m.end()) {
        return 1;
    }
    return 0;
}

int Directory::get_entry(const std::string& s,iid_t* ret) const {
    auto p = this->entry_m.find(s);
    if(p == this->entry_m.end()){
        LOG(WARNING) << "no such a entry for " << s;
        //TODO(lonhh): what to return if there is no such an entry?
        return 0;
    }
    *ret = p->second;
    return 1;
}

/**
 * @brief serialize the directory to bytes
 * @return 0 if not enough space, size of bytes returned otherwise
*/
int Directory::serialize(uint8_t* byte_stream, uint32_t size) {
    uint32_t s = 0;
    for(auto p=entry_m.begin();p!=entry_m.end();p++) {
        if( s + p->first.length() + sizeof(iid_t) + 1 < size) {
            memcpy(byte_stream+s,p->first.c_str(),p->first.length() + 1);
            s += p->first.length() + 1;
            memcpy(byte_stream+s,&(p->second),sizeof(iid_t));
            s += sizeof(iid_t);
        } else {
            LOG(WARNING) << "Not enough space for serializing Directory";
            return 0;
        }
    }
    memset(byte_stream+s,0,size - s);
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
        iid_t id = *((iid_t*)(byte_stream+i+s.length()+1));
        //insert_entry(s,i);
        entry_m[s] = id;
        i += s.length() + 1 + sizeof(iid_t);
    }
    return i;
}