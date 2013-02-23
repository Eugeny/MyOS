#include <io/FileObject.h>

void FileObject::writeString(char* s) {
    write(s, 0, strlen(s));
}

void FileObject::writeByte(char c) {
    write(&c, 0, 1);
}
