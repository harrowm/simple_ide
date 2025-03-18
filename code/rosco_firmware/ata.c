/*
 * vim: set et ts=4 sw=4
 *------------------------------------------------------------
 *                                  ___ ___ _
 *  ___ ___ ___ ___ ___       _____|  _| . | |_
 * |  _| . |_ -|  _| . |     |     | . | . | '_|
 * |_| |___|___|___|___|_____|_|_|_|___|___|_,_|
 *                     |_____|       firmware v2
 * ------------------------------------------------------------
 * Copyright (c) 2021 Ross Bamford & Contributors
 * 
 * Updates for simple_ide interface by Malcolm Harrow, Mar 2025
 * with credit to:
 *  https://github.com/crmaykish/mackerel-68k/blob/master/firmware/hello.c
 * 
 * MIT License
 *
 * Main low-level implementation for firmware ATA PIO
 * ------------------------------------------------------------
 */

#include <stdint.h>
#include <stdbool.h>
#include "ata.h"
#include "machine.h"

#ifdef ATA_DEBUG
static void IDE_print_status_byte(uint8_t status);
#endif

static int IDE_reset();
static int IDE_wait_for_device_ready();
static int IDE_wait_for_data_ready();   

// These dont seem to be available in the firmware at this stage
void *memset(void *s, int c, int n) {
    unsigned char *p = s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

char *strncpy(char *dest, const char *src, int n) {
    int i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}


static uint8_t selected_drive = ATA_NODEVICE;  /* no drive by default */
static char selected_modelnum[41] = "<No device>";


static int IDE_wait_for_device_ready()
{
    uint8_t status;
    uint32_t timeout = 0;

#ifdef ATA_DEBUG
    FW_PRINT_C("IDE_wait_for_device_ready: \r\n");
    uint8_t oldstatus = 0;
#endif

    while (((status = MEM(IDE_STATUS)) & ATA_SR_DRDY) == 0 && timeout++ < ATA_TIMEOUT)
    {
#ifdef ATA_DEBUG
        if (status != oldstatus)
        {
            IDE_print_status_byte(status);
            oldstatus = status;
        }
#endif
    }

#ifdef ATA_DEBUG
    if (timeout >= ATA_TIMEOUT)
    {   
        FW_PRINT_C("\nIDE_wait_for_device_ready() timed out\r\n");
    }
#endif

    return (timeout < ATA_TIMEOUT);    
}


static int IDE_wait_for_data_ready()
{
    uint8_t status;
    uint32_t timeout = 0;

#ifdef ATA_DEBUG
    FW_PRINT_C("IDE_wait_for_device_ready: \r\n");
#endif

    while (((status = MEM(IDE_STATUS)) & ATA_SR_DRQ) == 0 && timeout++ < ATA_TIMEOUT)
    {
#ifdef ATA_DEBUG
        IDE_print_status_byte(status);
#endif
    }

#ifdef ATA_DEBUG
    if (timeout >= ATA_TIMEOUT)
    {
        FW_PRINT_C("\nIDE_wait_for_data_ready() timed out\r\n");
    }
#endif
     
    return (timeout < ATA_TIMEOUT);  
}


#ifdef ATA_DEBUG
extern void print_unsigned(uint32_t, uint8_t);

void hexdump(uint8_t *buf, int len)
{
    for (int i = 0; i < len; i += 16)
    {
        print_unsigned(i, 16);
        for (int j = 0; j < 16; j++)
        {
            if (i + j < len) {
                print_unsigned(buf[i + j], 16);
                FW_PRINT_C(" ");
            } else
                FW_PRINT_C("   ");
        }
        FW_PRINT_C(" ");
        for (int j = 0; j < 16; j++)
        {
            if (i + j < len)
            {
                uint8_t c = buf[i + j];
                if (c >= 32 && c <= 126) {
                    char temp[2] = {c, 0};
                    FW_PRINT_C(temp);
                } else
                    FW_PRINT_C(".");
            }
        }
        FW_PRINT_C("\r\n");
    }
}

void IDE_print_status_byte(uint8_t status)
{
    if (status & ATA_SR_BSY)
        FW_PRINT_C("Busy-");
    if (status & ATA_SR_DRDY)
        FW_PRINT_C("Drive ready-");
    if (status & ATA_SR_DF)
        FW_PRINT_C("Drive write fault-");
    if (status & ATA_SR_DSC)
        FW_PRINT_C("Drive seek complete-");
    if (status & ATA_SR_DRQ)
        FW_PRINT_C("Data request ready-");
    if (status & ATA_SR_CORR)
        FW_PRINT_C("Corrected data-");
    if (status & ATA_SR_IDX)
        FW_PRINT_C("Index-");
    if (status & ATA_SR_ERR)
        FW_PRINT_C("Error-");
    FW_PRINT_C("\r\n");

    if (status & ATA_SR_ERR)
    {
        uint8_t error = MEM(IDE_ERROR);
        if (error & ATA_ER_BBK)
            FW_PRINT_C("Bad block-");
        if (error & ATA_ER_UNC)
            FW_PRINT_C("Uncorrectable data-");
        if (error & ATA_ER_MC)
            FW_PRINT_C("Media changed-");
        if (error & ATA_ER_IDNF)
            FW_PRINT_C("ID not found-");
        if (error & ATA_ER_MCR)
            FW_PRINT_C("Media change request-");
        if (error & ATA_ER_ABRT)
            FW_PRINT_C("Command aborted-");
        if (error & ATA_ER_TK0NF)
            FW_PRINT_C("Track 0 not found-");
        if (error & ATA_ER_AMNF)
            FW_PRINT_C("Address mark not found-");
        FW_PRINT_C("*\r\n");
    }
}
#endif


int ata_read_sector(uint16_t *buf, uint32_t lba)
{
#ifdef ATA_DEBUG
    FW_PRINT_C("\nata_read_sector() - LBA: ");
    print_unsigned(lba, 16);
    FW_PRINT_C("\r\n");
#endif

    // NOTE: IDE sector count starts at 1 (not true in LBA mode?)
    // NOTE: limited to 24 bit LBA addressing (~8GB)
    MEM(IDE_SECTOR_COUNT) = 1;
    MEM(IDE_SECTOR_START) = (uint8_t)(lba & 0xFF);
    MEM(IDE_LBA_MID) = (uint8_t)((lba >> 8) & 0xFF);
    MEM(IDE_LBA_HIGH) = (uint8_t)((lba >> 16) & 0xFF);

    // Select master drive and top bits of LBA
    MEM(IDE_DRIVE_SEL) = (ATA_DEVICE0_DRIVE | (uint8_t) (((lba >> 24) & 0x0F)));
    
    // Send the read sector command
    MEM(IDE_COMMAND) = IDE_CMD_READ_SECTOR;
    if (!IDE_wait_for_data_ready())
        return 1;
        
    // Get the data
    for (uint16_t i = 0; i < 256; i++)
    {
        buf[i] = MEM16(IDE_DATA);
    }
    return 0;
}


int ata_write_sector(uint16_t *buf, uint32_t lba)
{
#ifdef ATA_DEBUG
    FW_PRINT_C("\nata_write_sector() - LBA: ");
    print_unsigned(lba, 16);
    FW_PRINT_C("\r\n");
#endif


    // NOTE: IDE sector count starts at 1 (not true in LBA mode?)
    MEM(IDE_SECTOR_COUNT) = 1;
    MEM(IDE_SECTOR_START) = (uint8_t)(lba & 0xFF);
    MEM(IDE_LBA_MID) = (uint8_t)((lba >> 8) & 0xFF);
    MEM(IDE_LBA_HIGH) = (uint8_t)((lba >> 16) & 0xFF);

    // Select master drive and top bits of LBA
    MEM(IDE_DRIVE_SEL) = (ATA_DEVICE0_DRIVE | (uint8_t) (((lba >> 24) & 0x0F)));
    
    // Send the read sector command
    MEM(IDE_COMMAND) = IDE_CMD_WRITE_SECTOR;
    if (!IDE_wait_for_data_ready())
        return 1;

    // Send the data
    for (uint16_t i = 0; i < 256; i++)
    {
        MEM16(IDE_DATA) = buf[i];
    }
    return 0;
}


void ata_init() {
    FW_PRINT_C("Resetting IDE .. ");
    if (IDE_reset() != 0) {
        FW_PRINT_C("no IDE interface found\r\n");
        return;
    } else {
        FW_PRINT_C("done\r\n");
    }

    FW_PRINT_C("Initializing hard drives...\r\n");

    MEM(IDE_COMMAND) = IDE_CMD_IDENTIFY;
    if (IDE_wait_for_data_ready()) {
        uint16_t buf[256] = {0};
        for (uint16_t i = 0; i < 256; i++)
        {
            buf[i] = MEM16(IDE_DATA);
        }
        selected_drive = ATA_DEVICE0;

        // The device info data is stored in the buffer in big-endian format
        // so we need to swap the bytes to print the correct values
        char* ptr = selected_modelnum;
        memset(selected_modelnum, 0, sizeof(selected_modelnum));

        for (int16_t i = 27; i <= 46; i++)
        {
            *ptr++ = (char)((buf[i] & 0x00FF));
            *ptr++ = (char)((buf[i] & 0xFF00) >> 8);
        }
    } else {
        // No drive
        selected_drive = ATA_NODEVICE;
        strncpy(selected_modelnum, "<Not found>", sizeof(selected_modelnum)-1);
    }

    FW_PRINT_C("IDE Device 0: ");
    FW_PRINT_C(selected_modelnum);
    FW_PRINT_C("\r\n");
}

static int IDE_reset()
{
#ifdef ATA_DEBUG
    FW_PRINT_C("\nIDE_reset()\r\n");
#endif

    MEM(IDE_COMMAND) = IDE_CMD_RESET;
    return IDE_wait_for_device_ready();
}


/*********** Routines called from EFP functions */
uint32_t ATA_init(uint32_t drive, ATADevice *dev) {
    if (drive == ATA_DEVICE0 && selected_drive) {
        dev->device_num = ATA_DEVICE0;
        memset(dev->model_str, 0, sizeof(dev->model_str));
        strncpy(selected_modelnum, (const char *)dev->model_str, sizeof(dev->model_str)-1);
        return ATA_INIT_OK;
    }

    dev->device_num = ATA_NODEVICE;
    memset(dev->model_str, 0, sizeof(dev->model_str));
    strncpy(selected_modelnum, (const char *)dev->model_str, sizeof(dev->model_str)-1);
    return ATA_INIT_NODEVICE;
}

uint32_t ATA_read_sectors(uint8_t *buf, uint32_t lba, uint32_t num, ATADevice *dev) {
    if (dev->device_num != selected_drive || selected_drive != ATA_DEVICE0)
        return 0;

    for (uint32_t i = 0; i < num; i++) {
        if (!ata_read_sector((uint16_t*)buf, lba + i)) {
            return i;
        }
        buf += 512;
    }
    return num;
}

uint32_t ATA_write_sectors(uint8_t *buf, uint32_t lba, uint32_t num, ATADevice *dev) {
    if (dev->device_num != selected_drive || selected_drive != ATA_DEVICE0)
        return 0;

    for (uint32_t i = 0; i < num; i++) {
        if (!ata_write_sector((uint16_t*)buf, lba + i)) {
            return i;
        }
        buf += 512;
    }
    return num;
}

uint32_t ATA_ident(uint8_t *buf, ATADevice *dev) {
    uint16_t *buf16 = (uint16_t*)buf;

    MEM(IDE_COMMAND) = IDE_CMD_IDENTIFY;
    if (IDE_wait_for_data_ready()) {
        for (uint16_t i = 0; i < 256; i++)
        {
            buf16[i] = MEM16(IDE_DATA);
        }
        return ATA_DEVICE0;   
    }

    return ATA_NODEVICE;
}
