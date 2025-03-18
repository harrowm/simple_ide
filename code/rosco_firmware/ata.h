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
 * MIT License
 *
 * Main low-level defines and routines for firmware ATA PIO
 * ------------------------------------------------------------
 */

#ifndef __ROSCO_M68K_ATA_H
#define __ROSCO_M68K_ATA_H

// Get the value at a memory address
#define MEM(address) (*(volatile uint8_t *)(address))
#define MEM16(address) (*(volatile uint16_t *)(address))
#define MEM32(address) (*(volatile uint32_t *)(address))


#define ATA_DEVICE0                 0x0
#define ATA_NODEVICE                0xFF

#define ATA_DEVICE0_DRIVE           0xE0

#define ATA_TIMEOUT                 1000000

#define IDE_BASE 0xF20040
#define IDE_DATA IDE_BASE + 0x00
#define IDE_ERROR IDE_BASE + 0x02
#define IDE_SECTOR_COUNT IDE_BASE + 0x04
#define IDE_SECTOR_START IDE_BASE + 0x06
#define IDE_LBA_MID IDE_BASE + 0x08
#define IDE_LBA_HIGH IDE_BASE + 0x0A
#define IDE_DRIVE_SEL IDE_BASE + 0x0C
#define IDE_STATUS IDE_BASE + 0x0E
#define IDE_COMMAND IDE_BASE + 0x0E


// IDE commands
#define IDE_CMD_RESET 0x08
#define IDE_CMD_READ_SECTOR 0x20
#define IDE_CMD_WRITE_SECTOR 0x30
#define IDE_CMD_IDENTIFY 0xEC

#define ATA_SR_BSY                  0x80
#define ATA_SR_DRDY                 0x40
#define ATA_SR_DF                   0x20
#define ATA_SR_DSC                  0x10
#define ATA_SR_DRQ                  0x08
#define ATA_SR_CORR                 0x04
#define ATA_SR_IDX                  0x02
#define ATA_SR_ERR                  0x01

#define ATA_ER_BBK                  0x80
#define ATA_ER_UNC                  0x40
#define ATA_ER_MC                   0x20
#define ATA_ER_IDNF                 0x10
#define ATA_ER_MCR                  0x08
#define ATA_ER_ABRT                 0x04
#define ATA_ER_TK0NF                0x02
#define ATA_ER_AMNF                 0x01

typedef enum {
    ATA_INIT_OK,
    ATA_INIT_NODEVICE,
    ATA_INIT_GENERAL_FAILURE
} ATAInitStatus;

typedef struct {
    uint8_t     device_num;
    uint8_t     model_str[40];
} ATADevice;

uint32_t ATA_init(uint32_t drive, ATADevice *dev);
uint32_t ATA_read_sectors(uint8_t *buf, uint32_t lba, uint32_t num, ATADevice *dev);
uint32_t ATA_write_sectors(uint8_t *buf, uint32_t lba, uint32_t num, ATADevice *dev);
uint32_t ATA_ident(uint8_t *buf, ATADevice *dev);

#endif // __ROSCO_M68K_ATA_H
