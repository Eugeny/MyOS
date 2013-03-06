#include <fs/File.h>
#include <string.h>


File::File(const char* path, FS* fs) {
    strcpy(this->path, path ? path : "");
    filesystem = fs;
}

void File::close() {
}


int File::stat(struct stat* stat) {
    stat->st_dev = 0;
    stat->st_ino = 1;
    stat->st_mode = S_IRWXU | S_IRWXG | S_IRWXO;
    stat->st_uid = 0;
    stat->st_gid = 0;
    stat->st_size = 0;
    stat->st_nlink = 1;
    return -1;
}




StreamFile::StreamFile(const char* path, FS* fs) : File(path, fs) {
    type = FILE_STREAM;
}

void StreamFile::write(const void* buffer, uint64_t count) {
}

uint64_t StreamFile::read(void* buffer, uint64_t count) {
    return 0;
}

bool StreamFile::canRead() {
    return false;
}



StaticFile::StaticFile(void* c, uint64_t s) : StreamFile(NULL, NULL) {
    content = c;
    size = s;
    offset = 0;
}

void StaticFile::write(const void* buffer, uint64_t count) {
}

uint64_t StaticFile::read(void* buffer, uint64_t count) {
    int c = 0;
    if (size - offset < count)
        c = size - offset;
    memcpy(buffer, (void*)((uint64_t)content + offset), c);
    offset += c;
    return c;
}

void StaticFile::close() {
}

bool StaticFile::canRead() {
    return true;
}

int StaticFile::stat(struct stat* stat) {
    File::stat(stat);
    stat->st_size = size;
    stat->st_mode |= S_IFREG;
    return 0;
}