#include <core/Process.h>
#include <memory/Heap.h>
#include <vfs/VFS.h>
#include <io/FileObject.h>
#include <kutils.h>
#include <vfs/VFS.h>


Process::Process() {
    static int id = 0;
    pid = id++;
    lastFD = 0;
    lastDFD = 0;
    dead = false;
    memset(files, 0, sizeof(files));
    threads = new LinkedList<Thread*>();
    children = new LinkedList<Process*>();
    allocatorTop = Heap::get()->getFreeSpaceBoundary();
}

void *Process::requestMemory(u32int sz) {
    void* res = (void*)allocatorTop;
    for (u32int i = allocatorTop; i < allocatorTop + sz+1; i += 0x1000)
        addrSpace->allocatePage(i, true, false, true);
    allocatorTop += sz;
    return res;
}

Process::~Process() {
    delete children;
    delete threads;
    delete name;
}


u32int Process::openFile(FileObject* f) {
    files[lastFD++] = f;
    return lastFD - 1;
}

void Process::reopenFile(int fd, FileObject* f) {
    files[fd] = f;
    if (lastFD <= fd) lastFD = fd + 1;
}

u32int Process::openDir(char* path) {
    ProcessDirFDHolder* fdh = new ProcessDirFDHolder();
    fdh->nodes = VFS::get()->listFiles(path);
    fdh->pos = 0;
    dirs[lastDFD++] = fdh;
    return lastDFD-1;
}

u32int Process::create(char* path, int argc, char** argv, FileObject* stdin, FileObject* stdout, FileObject* stderr) {
    Stat* stat = VFS::get()->stat(path);
    char* ss = (char*)kmalloc(stat->size);

    FileObject* f = VFS::get()->open(path, MODE_R);
    f->read(ss, 0, stat->size);
    f->close();
    delete f;
    delete stat;

    ELF_exec((u8int*)ss, path, argc, argv, stdin, stdout, stderr);
    delete ss;
}



char* ProcessDirFDHolder::read() {
    if (pos < nodes->length())
        return nodes->get(pos++);
    else
        return NULL;
}

void ProcessDirFDHolder::rewind() {
    pos = 0;
}