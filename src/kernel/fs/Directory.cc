#include <fs/Directory.h>


Directory::Directory(const char* path, FS* fs) : File(path, fs) {
    type = FILE_DIRECTORY;
}

int Directory::stat(struct stat* stat) {
    File::stat(stat);
    stat->st_mode |= S_IFDIR;
    return -1;
}
