#ifndef __SDFUNC_H
#define __SDFUNC_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

typedef int (*fp_SPIWriteByte)( uint8_t);
typedef int (*fp_SPIReadByte)(void);
typedef void(*fp_GPIO_CS_CTRL)(void);

#define DEBUG_MSG_OUT       1
#define ENALBED_DEBUG_LOG   1

#define BASE_SECTOR_SIZE    512
#define SD_COMMAND_TIMEOUT 5000

#define SD_TYPE_V1          0
#define SD_TYPE_V2          1
#define SD_TYPE_V2HC        2
//------------------------------------------------------
#define CMD_GO_IDLE_STATE       0
#define CMD_SEND_IF_COND        8

#define CMD_SUCESS              0
#define R1_IDLE_STATE           (1 << 0)
#define R1_ERASE_RESET          (1 << 1)
#define R1_ILLEGAL_COMMAND      (1 << 2)
#define R1_COM_CRC_ERROR        (1 << 3)
#define R1_ERASE_SEQUENCE_ERROR (1 << 4)
#define R1_ADDRESS_ERROR        (1 << 5)
#define R1_PARAMETER_ERROR      (1 << 6)

typedef struct __CSDV1 {
    // byte 0
    unsigned reserved1 : 6;
    unsigned csd_ver : 2;
    // byte 1
    uint8_t taac;
    // byte 2
    uint8_t nsac;
    // byte 3
    uint8_t tran_speed;
    // byte 4
    uint8_t ccc_high;
    // byte 5
    unsigned read_bl_len : 4;
    unsigned ccc_low : 4;
    // byte 6
    unsigned c_size_high : 2;
    unsigned reserved2 : 2;
    unsigned dsr_imp : 1;
    unsigned read_blk_misalign :1;
    unsigned write_blk_misalign : 1;
    unsigned read_bl_partial : 1;
    // byte 7
    uint8_t c_size_mid;
    // byte 8
    unsigned vdd_r_curr_max : 3;
    unsigned vdd_r_curr_min : 3;
    unsigned c_size_low :2;
    // byte 9
    unsigned c_size_mult_high : 2;
    unsigned vdd_w_cur_max : 3;
    unsigned vdd_w_curr_min : 3;
    // byte 10
    unsigned sector_size_high : 6;
    unsigned erase_blk_en : 1;
    unsigned c_size_mult_low : 1;
    // byte 11
    unsigned wp_grp_size : 7;
    unsigned sector_size_low : 1;
    // byte 12
    unsigned write_bl_len_high : 2;
    unsigned r2w_factor : 3;
    unsigned reserved3 : 2;
    unsigned wp_grp_enable : 1;
    // byte 13
    unsigned reserved4 : 5;
    unsigned write_partial : 1;
    unsigned write_bl_len_low : 2;
    // byte 14
    unsigned reserved5: 2;
    unsigned file_format : 2;
    unsigned tmp_write_protect : 1;
    unsigned perm_write_protect : 1;
    unsigned copy : 1;
    unsigned file_format_grp : 1;
    // byte 15
    unsigned always1 : 1;
    unsigned crc : 7;
} CSDV1, *pCSDV1;

// CSD for version 2.00 cards
typedef struct __CSDV2 {
    // byte 0
    unsigned reserved1 : 6;
    unsigned csd_ver : 2;
    // byte 1
    uint8_t taac;
    // byte 2
    uint8_t nsac;
    // byte 3
    uint8_t tran_speed;
    // byte 4
    uint8_t ccc_high;
    // byte 5
    unsigned read_bl_len : 4;
    unsigned ccc_low : 4;
    // byte 6
    unsigned reserved2 : 4;
    unsigned dsr_imp : 1;
    unsigned read_blk_misalign :1;
    unsigned write_blk_misalign : 1;
    unsigned read_bl_partial : 1;
    // byte 7
    unsigned reserved3 : 2;
    unsigned c_size_high : 6;
    // byte 8
    uint8_t c_size_mid;
    // byte 9
    uint8_t c_size_low;
    // byte 10
    unsigned sector_size_high : 6;
    unsigned erase_blk_en : 1;
    unsigned reserved4 : 1;
    // byte 11
    unsigned wp_grp_size : 7;
    unsigned sector_size_low : 1;
    // byte 12
    unsigned write_bl_len_high : 2;
    unsigned r2w_factor : 3;
    unsigned reserved5 : 2;
    unsigned wp_grp_enable : 1;
    // byte 13
    unsigned reserved6 : 5;
    unsigned write_partial : 1;
    unsigned write_bl_len_low : 2;
    // byte 14
    unsigned reserved7: 2;
    unsigned file_format : 2;
    unsigned tmp_write_protect : 1;
    unsigned perm_write_protect : 1;
    unsigned copy : 1;
    unsigned file_format_grp : 1;
    // byte 15
    unsigned always1 : 1;
    unsigned crc : 7;
} CSDV2, *pCSDV2;

// union of old and new style CSD register
typedef union __CSD {
    CSDV1 v1;
    CSDV2 v2;
} CSD, *PCSD;

extern void SD_SetSPIWriteFunc( fp_SPIWriteByte func );
extern void SD_SetCSHighFunc( fp_GPIO_CS_CTRL func );
extern void SD_SetCSLowFunc( fp_GPIO_CS_CTRL func );

extern uint8_t SD_SendCmd( uint8_t cmd, uint32_t arg, uint8_t *response );
extern int SD_Init( void );
extern int SD_ReadCSD( CSD *csd );
extern int SD_SetBlockLength( uint32_t length );
extern int SD_ReadLba( uint32_t LBA, uint16_t SectorCnt, uint8_t *buffer );
extern int SD_WriteLba( uint32_t LBA, uint16_t SectorCnt, uint8_t *buffer );

#endif
