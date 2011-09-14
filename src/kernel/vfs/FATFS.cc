#include <vfs/FATFS.h>
#include <hardware/Disk.h>
#include <memory/Heap.h>
#include <kutils.h>



void dumpsect(u8int* s){
char ss[2];
ss[1] = 0;
for (int i =0;i<32;i++) {
    ss[0]=(char)s[i];
    klogn(ss);
//    klogn(to_hex(s[i]));
//    klogn(" ");
}

for (int i =0;i<32;i++) {
    klogn(to_hex(s[i]));
    klogn(" ");
}

klog("");
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

    return fat_table[ent_offset] & 0x0FFFFFF;
}

void FATFS::readFile(u32int cluster, void* buffer) {
    u32int fatval = getFATValue(cluster);
    DEBUG(to_hex(first_data_sector+cluster*fat_boot->sectors_per_cluster));
    while (fatval < 0x0FFFFFF7 && fatval > 1) {
        for (int i = 0; i < fat_boot->sectors_per_cluster; i++) {
            Disk::get()->read(first_data_sector+cluster*fat_boot->sectors_per_cluster + i, buffer);
            buffer += 512;
        }
        cluster = fatval;
        fatval = getFATValue(cluster);
    }
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
    node->nameptr = &(node->name[254]);

    dumpsect((u8int*)file);
    while (file->attr == 0xF) {
        fat_lname_t* name = (fat_lname_t*)data;
        DEBUG(node->nameptr);
        u8int* cptr;

        cptr = name->text3 + 5;
        for (int i = 0; i < 2; i++) {
            cptr -= 2;
            if (*cptr == 0 || *cptr == 0xFF) continue;
            DEBUG(to_hex(*cptr))
            *(u8int*)node->nameptr = *cptr;
            node->nameptr--;
        }

        cptr = name->text2 + 13;
        for (int i = 0; i < 6; i++) {
            cptr -= 2;
            if (*cptr == 0 || *cptr == 0xFF) continue;
            DEBUG(to_hex(*cptr))
            *(u8int*)node->nameptr = *cptr;
            node->nameptr--;
        }

        cptr = name->text1 + 11;
        for (int i = 0; i < 5; i++) {
            cptr -= 2;
            DEBUG(to_hex(*cptr))
            if (*cptr == 0 || *cptr == 0xFF) continue;
            *(u8int*)node->nameptr = *cptr;
            node->nameptr--;
        }

        offset += 0x20;
        data += 0x20;
        if (offset == size) return NULL;
        file = (fat_file_t*)data;
    }

    *len = offset;
    return node;
}


FATFS::FATFS() {
    fat_page = -1;
    fat_boot = (fat_bs_t*)kmalloc(512);
    fat_table = (u32int*)kmalloc(512);

TRACE
    Disk::get()->read(0, fat_boot);
TRACE
    first_data_sector = fat_boot->reserved_sector_count + (fat_boot->table_count * fat_boot->table_size_32);
TRACE
    first_fat_sector = fat_boot->reserved_sector_count;
TRACE
    root_cluster = fat_boot->root_cluster;

TRACE
    void* buf = kmalloc(512*100);
TRACE
    readFile(0, buf);
TRACE

    u32int offset = 0;
TRACE
    fat_node *n = parseDir(buf+32, 512*100, &offset);
    dumpsect((u8int*)buf+32);
    DEBUG(n->nameptr);
}
