#include "directory/directory.h"

Directory::Directory(iid_t id, iid_t parent): entry_m() {
    this->insert_entry(".",id);
    this->insert_entry("..",parent);
}
int Directory::insert_entry(const std::string& s,iid_t id) {
    // TODO(lonhh) sanity check
    this->entry_m[s] = id;
}
int Directory::remove_entry(const std::string& s) {
    // TODO(lonhh) sanity check
    this->entry_m.erase(s);

}
int Directory::contains_entry(const std::string& s) const {
    return this->entry_m.find(s) != this->entry_m.end();
}

int serialize(uint8_t* byte_stream) {

}

int deserialize(const uint8_t* byte_stream) {

}