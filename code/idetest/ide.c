#include "ide.h"
#include <stdio.h>

#define ATA_DEBUG

static void read_sector(uint16_t *buf)
{
    for (uint16_t i = 0; i < 256; i++)
    {
        // IDE_wait_for_data_ready();   // needed at higher speeds?
        buf[i] = MEM16(IDE_DATA);
    }
}

static void IDE_print_status_byte(uint8_t status)
{
    if (status & IDE_SR_BSY)
        printf("Busy ");
    if (status & IDE_SR_DRDY)
        printf("Drive-ready ");
    if (status & IDE_SR_DF)
        printf("Drive-write-fault ");
    if (status & IDE_SR_DSC)
        printf("Drive-seek-complete ");
    if (status & IDE_SR_DRQ)
        printf("Data-request-ready ");
    if (status & IDE_SR_CORR)
        printf("Corrected-data ");
    if (status & IDE_SR_IDX)
        printf("Index ");
    if (status & IDE_SR_ERR)
        printf("Error ");
    printf("%d\n", status);

    if (status & IDE_SR_ERR)
    {
        uint8_t error = MEM(IDE_ERROR);
        printf("Error code: %d - ", error);
        if (error & ATA_ER_AMNF)
            printf("  Address mark not found\n");
        if (error & ATA_ER_TK0NF)
            printf("  Track 0 not found\n");
        if (error & ATA_ER_ABRT)
            printf("  Aborted command\n");
        if (error & ATA_ER_MCR)
            printf("  Media change request\n");
        if (error & ATA_ER_BBK)
            printf("  Bad block detected\n");
        if (error & ATA_ER_UNC)
            printf("  Uncorrectable data error\n");
        if (error & ATA_ER_MC)
            printf("  Media change request\n");
        if (error & ATA_ER_IDNF)
            printf("  ID not found\n");
    }
}


static int IDE_wait_for_device_ready()
{
    uint8_t status;
    uint32_t timeout = 0;

#ifdef ATA_DEBUG
    printf("IDE_wait_for_device_ready: \n");
#endif

    while (((status = MEM(IDE_STATUS)) & IDE_SR_DRDY) == 0 && timeout++ < ATA_TIMEOUT)
    {
#ifdef ATA_DEBUG
        IDE_print_status_byte(status);
#endif
    }

#ifdef ATA_DEBUG
    if (timeout >= ATA_TIMEOUT)
    {   
        printf("\nIDE_wait_for_device_ready() timed out\n");
    }
#endif

    return (timeout < ATA_TIMEOUT);    
}


static int IDE_wait_for_data_ready()
{
    uint8_t status;
    uint32_t timeout = 0;

#ifdef ATA_DEBUG
    printf("IDE_wait_for_data_ready: \n");
    uint8_t old_status = 0;
#endif

    while (((status = MEM(IDE_STATUS)) & IDE_SR_DRQ) == 0 && timeout++ < ATA_TIMEOUT)
    {
#ifdef ATA_DEBUG
        if (status != old_status)
        {
            IDE_print_status_byte(status);
            old_status = status;
        }
#endif
    }

#ifdef ATA_DEBUG
    if (timeout >= ATA_TIMEOUT)
    {
        printf("\nIDE_wait_for_data_ready() timed out\n");
    }
#endif
     
    return (timeout < ATA_TIMEOUT);  
}


void IDE_read_sector(uint16_t *buf, uint32_t lba)
{
#ifdef ATA_DEBUG
    printf("\nIDE_read_sector() - LBA: %u\n", lba);
#endif
    // Select master drive
    MEM(IDE_DRIVE_SEL) = 0xE0;

    // NOTE: IDE sector count starts at 1 (not true in LBA mode?)
    // NOTE: limited to 24 bit LBA addressing (~8GB)
    MEM(IDE_SECTOR_COUNT) = 1;
    MEM(IDE_SECTOR_START) = (uint8_t)(lba & 0xFF);
    MEM(IDE_LBA_MID) = (uint8_t)((lba >> 8) & 0xFF);
    MEM(IDE_LBA_HIGH) = (uint8_t)((lba >> 16) & 0xFF);
    
    // Send the read sector command
    MEM(IDE_COMMAND) = IDE_CMD_READ_SECTOR;
    if (IDE_wait_for_data_ready()) {
        read_sector(buf);
    }
}

void IDE_device_info(uint16_t *buf)
{
    printf("Reading IDE device info...\r\n");
    MEM(IDE_COMMAND) = IDE_CMD_IDENTIFY;
    if (IDE_wait_for_data_ready()) {
        read_sector(buf);
    }

    // The device info data is stored in the buffer in big-endian format
    // so we need to swap the bytes to print the correct values

    // Model
    for (int i = 27; i <= 46; i++)
    {
        printf("%c%c", (char)((buf[i] & 0x00FF)), (char)((buf[i] & 0xFF00) >> 8));
    }

    printf("\r\n");

    // Version
    for (int i = 23; i <= 26; i++)
    {
        printf("%c%c", (char)((buf[i] & 0x00FF)), (char)((buf[i] & 0xFF00) >> 8));
    }

    printf("\r\n");

    // Serial number
    for (int i = 10; i <= 19; i++)
    {
        printf("%c%c", (char)((buf[i] & 0x00FF)), (char)((buf[i] & 0xFF00) >> 8));
    }

    printf("\r\n");
}

int IDE_reset()
{
#ifdef ATA_DEBUG
    printf("\nIDE_reset()\n");
#endif

    MEM(IDE_COMMAND) = IDE_CMD_RESET;
    return IDE_wait_for_device_ready();
}