#include <fs/fat32/FAT32FS.h>
#include <fcntl.h>
#include <kutil.h>
#include <string.h>
#include <errno.h>


FAT32FS::FAT32FS() {
    fs = new FATFS();
    f_mount(0, fs);
}


StreamFile* FAT32FS::open(char* path, int flags) {
    FIL* fil = new FIL();
    //klog('w', "fatopenfile %s", path);  

    int mode = FA_READ;
    if (flags & O_RDONLY)   mode |= FA_READ;
    if (flags & O_WRONLY)   mode |= FA_WRITE;
    if (flags & O_RDWR)     mode |= FA_READ | FA_WRITE;
    if (flags & O_CREAT)    mode |= FA_OPEN_ALWAYS;
    if (flags & O_TRUNC)    mode |= FA_CREATE_ALWAYS;

    int result = f_open(fil, path, mode);
//klog('i', "FRESULT = %i", result);
    if (result == FR_NO_FILE || result == FR_NO_PATH || result == FR_INVALID_NAME) {
        delete fil;
        seterr(ENOENT);
        return NULL;
    }

    return new FAT32File(path, this, fil);
}

Directory* FAT32FS::opendir(char* path) {
    FDIR* dir = new FDIR();
    
    int result = f_opendir(dir, path);
    if (result == FR_NO_FILE || result == FR_NO_PATH) {
        delete dir;
        seterr(ENOENT);
        return NULL;
    }

    return new FAT32Directory(path, this, dir);
}



FAT32File::FAT32File(const char* p, FAT32FS* fs, FIL* f) : StreamFile(p, fs) {
    fil = f;
    eof = false;
}

FAT32File::~FAT32File() {
    delete path;
}


void FAT32File::write(const void* buffer, uint64_t count) {
}

uint64_t FAT32File::read(void* buffer, uint64_t count) {
    uint32_t num;
    f_read(fil, buffer, count, &num);
    if (num == 0)
        eof = true;
    return num;
}

bool FAT32File::isEOF() {
    return eof;
}

void FAT32File::close() {
    f_close(fil);
    delete fil;
}

bool FAT32File::canRead() {
    return true;
}

int FAT32File::stat(struct stat* stat) {
    File::stat(stat);
    FILINFO fi;
    fi.lfsize = 0;
    f_stat(path, &fi);
    stat->st_size = fi.fsize;
    stat->st_mode |= S_IFREG;
    return 0; 
}


FAT32Directory::FAT32Directory(const char* path, FAT32FS* f, FDIR* d) : Directory(path, f) {
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

int FAT32Directory::stat(struct stat* stat) {
    Directory::stat(stat);
    //FILINFO fi;
    //f_stat(path, &fi);
    stat->st_nlink = 5;
    return 0; 
}