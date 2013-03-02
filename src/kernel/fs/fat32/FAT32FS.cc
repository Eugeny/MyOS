#include <fs/fat32/FAT32FS.h>
#include <fcntl.h>
#include <kutil.h>
#include <string.h>


FAT32FS::FAT32FS() {
    fs = new FATFS();
    f_mount(0, fs);
}


File* FAT32FS::open(char* path, int flags) {
    FIL* fil = new FIL();

    int mode = FA_READ;
    if (flags & O_RDONLY)   mode |= FA_READ;
    if (flags & O_WRONLY)   mode |= FA_WRITE;
    if (flags & O_RDWR)     mode |= FA_READ | FA_WRITE;
    if (flags & O_CREAT)    mode |= FA_OPEN_ALWAYS;
    if (flags & O_TRUNC)    mode |= FA_CREATE_ALWAYS;

    f_open(fil, path, mode);
    return new FAT32File(this, fil);
}

Directory* FAT32FS::opendir(char* path) {
    FDIR* dir = new FDIR();
    f_opendir(dir, path);
    return new FAT32Directory(this, dir);
}


FAT32File::FAT32File(FAT32FS* fs, FIL* f) {
    filesystem = fs;
    fil = f;
}



void FAT32File::write(const void* buffer, uint64_t count) {

}

uint64_t FAT32File::read(void* buffer, uint64_t count) {
    uint32_t num;
    f_read(fil, buffer, count, &num);
    return num;
}

void FAT32File::close() {
    f_close(fil);
    delete fil;
}

bool FAT32File::canRead() {
    return true;
}


FAT32Directory::FAT32Directory(FAT32FS* f, FDIR* d) {
    filesystem = f;
    dir = d;
}

struct dirent* FAT32Directory::read() {
    FILINFO fi;
    char lfnBuffer[255];
    fi.lfname = lfnBuffer;
    fi.lfsize = 255;
    f_readdir(dir, &fi);
    memcpy(currentEntry.d_name, lfnBuffer, 255);
    currentEntry.d_type = (fi.fattrib & AM_FDIR) ? DT_DIR : DT_REG;

    if (currentEntry.d_name[0])
        return &currentEntry;
    else
        return NULL;
}

void FAT32Directory::close() {
    delete dir;
}