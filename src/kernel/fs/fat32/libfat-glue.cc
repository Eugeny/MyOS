#include <hardware/ata/ATA.h>
#include <libfat/diskio.h>

extern "C" {
    DSTATUS disk_initialize (BYTE pdrv) {
        return 0;
    }

    DSTATUS disk_status (BYTE pdrv) {
        return 0;

    }

    DRESULT disk_read (BYTE pdrv, BYTE* buff, DWORD sector, BYTE count) {
        for (int i = 0; i < count; i++)
            ata_read(sector + i, (uint8_t*)((uint64_t)buff + 512 * i));
        return (DRESULT)0;
    }

    DRESULT disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, BYTE count) {
        for (int i = 0; i < count; i++)
            ata_write(sector + i, (uint8_t*)((uint64_t)buff + 512 * i));
        return (DRESULT)0;
    }

    DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff) {
        return (DRESULT)0;
    }

    DWORD get_fattime (void) {
        return 0;
    }

    WCHAR ff_convert (WCHAR chr, UINT dir) {
        return chr;
    }

    WCHAR ff_wtoupper (WCHAR chr) {
        return chr;
    }
}