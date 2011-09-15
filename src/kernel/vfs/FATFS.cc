#include <vfs/FATFS.h>
#include <hardware/Disk.h>
#include <memory/Heap.h>
#include <kutils.h>
#include <vfs/VFS.h>


void dumpsect(u8int* s, int l){
char* ss = (char*)kmalloc(l+1);
memcpy(ss, s, l);
ss[l] = 0;
for (int i =0;i<l;i++)
    if (ss[i] == 0) ss[i] = 1;
klog(ss);

for (int i =0;i<l;i++) {
//    klogn(to_hex(s[i]));
//    klogn(" ");
}

klog("");
delete (ss);
}

u32int FATFS::getFATValue(u32int cluster) {
    u32int cls = fat_boot->bytes_per_sector;
    u32int fat_offset = cluster * 4;
    u32int fat_sector = first_fat_sector + (fat_offset / cls);
    u32int ent_offset = fat_offset % cls;
    if (fat_sector != fat_page) {
       Disk::get()->read(fat_sector, fat_table);
       fat_page = fat_sector;
    }
    return fat_table[ent_offset/4] & 0x0FFFFFFF;
}

void FATFS::readFile(u32int cluster, void* buffer) {
    u32int fatval = cluster;
    do {
        cluster = fatval;
        fatval = getFATValue(cluster);
        for (int i = 0; i < fat_boot->sectors_per_cluster; i++) {
            Disk::get()->read(first_data_sector+(cluster-2)*fat_boot->sectors_per_cluster + i, buffer);
            buffer += 512;
        }
    } while (fatval < 0x0FFFFFF7 && fatval > 0);
}


fat_node *FATFS::parseDir(void *data, u32int size, u32int *len) {
    u32int offset = 0;
    if (*(u8int*)data == 0) return NULL;

    while (offset < size && (*(u8int*)data == 0xe5)) {
        offset += 0x20;
        data += 0x20;
    }

    if (offset >= size) return NULL;

    fat_file_t* file = (fat_file_t*)data;
    fat_node* node = (fat_node*)kmalloc(sizeof(fat_node));
    node->name[255] = 0;
    node->nameptr = &(node->name[255]);

    while (file->attr == 0xF) {
        fat_lname_t* name = (fat_lname_t*)data;
        char* cptr;

        int offsets[] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };

        for (int i = 12; i >= 0; i--) {
            cptr = (char*)((u32int)file + offsets[i]);
            if (*cptr <= 0 || *cptr >= 0xFF) continue;
            node->nameptr--;
            *(node->nameptr) = *cptr;
        }

        offset += 0x20;
        data += 0x20;
        if (offset == size) return NULL;
        file = (fat_file_t*)data;
    }

    node->attr = file->attr;
    node->size = file->size;
    node->cluster = file->ch * 65536 + file->cl;
    offset += 0x20;

    *len = offset;
    return node;
}


static void* buf;

LinkedList<char*>* FATFS::listFiles(char* node) {
    fat_node* fn = findFile(node);
    if (!fn) return NULL;
    u32int cls = fn->cluster;
    delete fn;

    LinkedList<char*>* r = new LinkedList<char*>();
    readFile(cls, buf);
    void* ptr = buf;
    while (true) {
        u32int offset = 0;
        fat_node* node = parseDir(ptr, 512*100, &offset);
        if (!node) {
            return r;
        }
        if (strlen(node->nameptr) > 0) {
            r->insertLast(strclone(node->nameptr));
        }
        delete node;
        ptr += offset;
    }

    return r;
}


fat_node* FATFS::findFile(char* name) {
    LinkedList<char*>* path = VFS::splitPath(name);
    readFile(fat_boot->root_cluster, buf);
    for (int i = 0; i < path->length(); i++) {
//        DEBUG(path->get(i));
        void* ptr = buf;
        u32int offset = 0;

        while (true) {
            fat_node* node = parseDir(ptr, 512*100, &offset);
            if (!node) {
                path->purge();
                delete path;
                return NULL;
            }
            ptr += offset;
            if (strcmp(node->nameptr, path->get(i))) {
                if (i == path->length()-1) {
                    path->purge();
                    delete path;
                    return node;
                }
                readFile(node->cluster, buf);
                delete node;
                break;
            }
            delete node;
        }
    }
    path->purge();
    delete path;

    fat_node* r = (fat_node*)kmalloc(sizeof(fat_node));
    r->cluster = fat_boot->root_cluster;
    return r;
}


Stat* FATFS::stat(char* path) {
//    DEBUG(path);
    fat_node* fn = findFile(path);
//    DEBUG(fn->nameptr);
    Stat* s = new Stat();
    s->isDirectory = fn->attr & 0x10;
    delete fn;
    return s;
}

FATFS::FATFS() {
    fat_page = -1;
    fat_boot = (fat_bs_t*)kmalloc(512);

    Disk::get()->read(0, fat_boot);
    first_data_sector = fat_boot->reserved_sector_count + (fat_boot->table_count * fat_boot->table_size_32);
    first_fat_sector = fat_boot->reserved_sector_count;
    buf = kmalloc(512*100);
    fat_table = (u32int*)kmalloc(512);
}
