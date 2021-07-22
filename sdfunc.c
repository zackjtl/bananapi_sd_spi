#include "sdfunc.h"


//----------------------------------------------------------------------------------
// Global Var
//----------------------------------------------------------------------------------
FILE* LogFileHandle      = NULL;
fp_SPIWriteByte SPIWrite = NULL;
fp_SPIReadByte  SPIRead  = NULL;
fp_GPIO_CS_CTRL GPIO_CSH = NULL;
fp_GPIO_CS_CTRL GPIO_CSL = NULL;
//----------------------------------------------------------------------------------
// Internal Common functions
//----------------------------------------------------------------------------------

static uint8_t g_SD_Type = SD_TYPE_V1;

void outDebugMsg(  const char * format, ...  )
{
#if DEBUG_MSG_OUT
    char msg[1024] = { 0x00 };
    va_list argptr;
    va_start(argptr, format);
    vsprintf( msg, format, argptr);
    va_end(argptr);
    printf( "%s", msg );
    fflush(stdout);
    
    #if ENALBED_DEBUG_LOG
    if( LogFileHandle != NULL ) {
        fseek( LogFileHandle, 0, SEEK_END );
        fwrite( msg, 1, strlen(msg), LogFileHandle );
    }
    else {
        LogFileHandle = fopen( "./testlog.txt", "wt+" );
        if( NULL == LogFileHandle )
        {
            outDebugMsg("\n Open Text Log file Error!!!\n");
            abort();
        }
    }
    #endif

#endif
}

int ext_bits(uint8_t *data, int msb, int lsb) {
    int bits = 0;
    int size = 1 + msb - lsb;
    int i = 0;
    for( i=0; i<size; i++) {
        int position = lsb + i;
        int byte = 15 - (position >> 3);
        int bit = position & 0x7;
        int value = (data[byte] >> bit) & 1;
        bits |= value << i;
    }
    return bits;
}

void IODelay(unsigned int millis)
{
    struct timespec sleeper;
    
    sleeper.tv_sec  = (time_t)(millis / 1000);
    sleeper.tv_nsec = (long)(millis % 1000) * 1000000;
    nanosleep(&sleeper, NULL);
}


//-------------------------------------------------------------------------
// Internal Functions
//-------------------------------------------------------------------------
int SDWriteByte( uint8_t buf ) {
    if( SPIWrite == NULL ) {
        outDebugMsg("\n Oh no ~~ SPI WriteByte function point not init!!\nPlease call SD_SetSPIWriteFunc() to init the SPI write interface frist!\n\n");
        abort();
    }
    return SPIWrite( buf );
}

int SDReadByte( void ) {
    if( SPIRead == NULL ) {
        return SPIWrite( 0xFF );
    }
    else
    {
        return SPIRead();
    }
}
//-------------------------------------------------------------------------
// SD Functions
//-------------------------------------------------------------------------
void SD_SetSPIWriteFunc( fp_SPIWriteByte func ) {
    SPIWrite = NULL;
    SPIWrite = func;
}
void SD_SetSPIReadFunc( fp_SPIReadByte func ) {
    SPIRead = NULL;
    SPIRead= func;
}
void SD_SetCSHighFunc( fp_GPIO_CS_CTRL func ) {
    GPIO_CSH = NULL;
    GPIO_CSH = func;
}
void SD_SetCSLowFunc( fp_GPIO_CS_CTRL func ) {
    GPIO_CSL = NULL;
    GPIO_CSL = func;
}

uint8_t WaitCardReady (void)
{
    uint8_t token = 0x00;
    
    uint8_t timeout = SD_COMMAND_TIMEOUT;
    
    SDReadByte();
    do {
        token = SDReadByte();
       // IODelay( 10 );
    } while ( (token != 0xFF) && ( (timeout--) != 0 ) );
    
    return token;
}

//-------------------------------------------------------------------------

uint8_t SD_SendCmd( uint8_t cmd, uint32_t arg, uint8_t *response ) {
    uint32_t i = 0;
    uint8_t ret = 0;
    
    if( GPIO_CSH != NULL ) GPIO_CSH();
    if( GPIO_CSL != NULL ) GPIO_CSL();

    uint8_t token = 0x00;
    
    uint8_t timeout = SD_COMMAND_TIMEOUT;
    

    if( cmd != 0 ) {
        //sleep( 3 );
        
        do {
            token = SDReadByte();
         //   token |= SDReadByte();
        } while ( (token != 0xFF) && ( (timeout--) != 0 ) );

    
        if( token != 0xFF ) {
            outDebugMsg( "CMD %d Fail, Because card not ready!!\n", cmd );
            return 0xFF;
        }
    }
    
    SDWriteByte( 0x40 | cmd );
    SDWriteByte( arg >> 24 );
    SDWriteByte( arg >> 16 );
    SDWriteByte( arg >> 8 );
    SDWriteByte( arg >> 0 );
    if( cmd == 0 || cmd == 8 ) {
        SDWriteByte( ( cmd == 8 ? 0x87 : 0x95 ) );
    }
    else {
       SDWriteByte( 0x01 );
    }
    
    if( cmd == 12 ) {
        SDReadByte();
    }
    
    for( i = 0; i < SD_COMMAND_TIMEOUT; i++ ) {
        ret = SDReadByte();
        if(!( ret & 0x80 ) ) {
             break;
        }
    }
    
    if( response != NULL ) {
        uint8_t j = 0;
        for( j = 0; j<4; j++ ) {
                    response[ j ] = SDReadByte();
        }
        outDebugMsg( "CMD %d ARG = 0x%08X response = 0x%02X %02X %02X %02X\n", cmd, arg, response[0], response[1], response[2], response[3] );
    }
    else {
        outDebugMsg( "CMD %d ARG = 0x%08X response = 0x%02X\n", cmd, arg, ret );
    
    }
    
    if( GPIO_CSH != NULL ) GPIO_CSH();
  
    SDReadByte(); //extra 8 CLK
    
    return ret;
}

//-------------------------------------------------------------------------
static uint8_t SD_OldStatus = 0x01;

int SD_CheckStatus( void )
{
    uint8_t status;
    uint8_t d;
    outDebugMsg( "aan" );
 
    SD_SendCmd( 13, 0, NULL);	/* Read card status */
    d = SDReadByte();		/* Receive following half of R2 */
  
    status = SD_OldStatus;
    if( !(status&0x01) ) {
        status = d;
         if( GPIO_CSL!=NULL )GPIO_CSL();
    }
    
    outDebugMsg( " \n SD_OldStatus = 0x%02X, Current Status =  0x%02X\n", SD_OldStatus, status );
   
    SD_OldStatus = status;
    
    return status;
}


//-------------------------------------------------------------------------
int SD_ReadData( uint8_t *buffer, uint16_t length ) {
    
    uint8_t token = 0x00;
    uint8_t timeout = SD_COMMAND_TIMEOUT;
    
    do {
        token = SDReadByte();
    } while ( (token != 0xFE) && ( (timeout--) != 0 ) );
    
    
    if( token!= 0xFE )
    {
        outDebugMsg( "Can not get START BLOCK TOKEN" );
        return -1;
    }
    
    uint16_t Idx;
    for( Idx = 0; Idx< length; Idx+=4) {
        buffer[ Idx ] = SDReadByte();
        buffer[ Idx+1 ] = SDReadByte();
        buffer[ Idx+2 ] = SDReadByte();
        buffer[ Idx+3 ] = SDReadByte();
    }
   
    SDReadByte();// checksum
    SDReadByte();

    
   // if( GPIO_CSH != NULL ) GPIO_CSH();

   // SDReadByte();

    return 0;
}
//-------------------------------------------------------------------------
int SD_WriteData( uint8_t StartToken, uint8_t *buffer, uint16_t length ) {
    
     if( WaitCardReady() != 0xFF ) {
        outDebugMsg( " Card not ready!!\n" );
        return  -1;
    }
    
    SDWriteByte( StartToken );
    
    if( StartToken != 0xFD ) {
    
        // write the data
        uint16_t Idx;
        for( Idx = 0; Idx < length; Idx +=2) {
            SDWriteByte( buffer[ Idx ] );
            SDWriteByte( buffer[ Idx+1 ] );
            //outDebugMsg( "Write Byte %d", Idx );
        }
    
        SDWriteByte( 0xFF );// checksum
        SDWriteByte( 0xFF );

        // check the repsonse token
        if(( SDReadByte() & 0x1F) != 0x05) {
            outDebugMsg( "Write Data not accepted, return with error !" );
            return -1;
        }
    }
    
    // wait for write to finish
    while( SDReadByte() == 0);
  
    return 0;
}
//-------------------------------------------------------------------------
// Extern Functions
//-------------------------------------------------------------------------
int SD_Init( void ) {
    uint32_t timeout;
    uint8_t ret;
    
    uint8_t response[4] = { 0x00 };
    
    if( GPIO_CSH != NULL ) GPIO_CSH();
    
    // Send 10 Byte data,  CS high and apply 74 more clock
    uint8_t i = 0;
    for( i=0; i<10; i++ ) {
        SDWriteByte( 0xFF );
    }
    
    if( GPIO_CSL != NULL ) GPIO_CSL();
   
    ret = SD_SendCmd( CMD_GO_IDLE_STATE, 0, NULL );
    if( ( ret == 0xFF ) || ( (ret & 0xF7) != 0x01 ) ) {
        outDebugMsg( "CMD0 Fail : Please check Card Exists or Is idle state\n" );
        return -1;
    }
    
    ret = SD_SendCmd( CMD_SEND_IF_COND, 0x01aa, response );
    if( (ret&R1_IDLE_STATE) != R1_IDLE_STATE ) {
        outDebugMsg( "CMD8 Fail : Please check Card Is idle state\n" );
        return -1;
    }
    
    if( response[2] == 0x01 && response[3] == 0xaa ) {
        timeout = SD_COMMAND_TIMEOUT;
        do {
            SD_SendCmd( 55, 0, NULL );
            ret = SD_SendCmd( 41, ( 1 << 30 ), NULL );
            timeout--;
        } while( (timeout != 0x00 ) && ( ret != 0 ) );
         
        if( timeout == 0 ) {
            outDebugMsg( "ACMD41 Fail : Please check your Card!!" );
            return -1;
        }
                   
        uint8_t OCR[4] = { 0x00 };
        if( SD_SendCmd( 58, 0, OCR ) == CMD_SUCESS ) {
            
            if(  (uint8_t)(OCR[0]&0x40) == 0x40) {
                g_SD_Type = SD_TYPE_V2HC;
                outDebugMsg("SD_Type = 0x%02X SDCARD_V2HC\n", OCR[0] );
            }
            else {
                g_SD_Type = SD_TYPE_V2;
                outDebugMsg("SD_Type = 0x%02X SDCARD_V2\n", OCR[0] );
            }
        }
    }
    else {
        outDebugMsg("SD_Type = SDCARD_V1\n");
        
    }
    
    return 0;
}

//-------------------------------------------------------------------------
int SD_ReadCSD( CSD *csd ) {
    
    memset( csd, 0x00, sizeof(CSD) );
    
    if(  SD_SendCmd( 9, 0, NULL ) != 0) {
        outDebugMsg("Didn't get a response from the disk\n");
        return -1;
    }
    
    uint8_t csdbuf[16] = { 0x00 };
    if( SD_ReadData( csdbuf, 16) != 0) {
        outDebugMsg(  "Couldn't read csd data from disk\n" );
        return -1;
    }
    
    int csd_structure = ext_bits(csdbuf, 127, 126);
    outDebugMsg( "CSD_STRUCT = %d\n", csd_structure);
    
    if(   csd_structure == 0 ) {
        int c_size = ext_bits(csdbuf, 73, 62);
        int c_size_mult = ext_bits(csdbuf, 49, 47);
        int read_bl_len = ext_bits(csdbuf, 83, 80);
    
        int block_len = 1 << read_bl_len;
        int mult = 1 << (c_size_mult + 2);
        int blocknr = (c_size + 1) * mult;
        int capacity = blocknr * block_len;
    
        capacity = capacity / 512;
    
    
        outDebugMsg( "sectors = %d\n", capacity*512);
        outDebugMsg( "capacity = %d bytes (%d MB)\n", capacity, capacity/2048);
        outDebugMsg( "block_len = %d\n", block_len);
        
        return capacity;
    }
    else {
        uint32_t c_size = (((uint32_t)csdbuf[7] & 0x3F) << 16) | ((uint32_t)csdbuf[8] << 8) | csdbuf[9];
        uint32_t capacity = ((uint32_t)(c_size + 1) * (uint32_t)(1024u)) - 1;
        return capacity;
    }
}
//-------------------------------------------------------------------------
int SD_SetBlockLength( uint32_t length ) {
    if( SD_SendCmd( 16, length, NULL ) != 0) {
        outDebugMsg("Set %d-byte block timed out\n", length );
        return -1;
    }
    return 0;
}
//-------------------------------------------------------------------------
int SD_ReadLba( uint32_t LBA, uint16_t SectorCnt, uint8_t *buffer ) {
    
    uint8_t Cmd         = ( SectorCnt == 1 ? 17 : 18 );
    uint16_t SectorIdx  = 0;
    uint32_t argv       = ( g_SD_Type == SD_TYPE_V2HC ? LBA : LBA * BASE_SECTOR_SIZE );
    int ret             = 0;
    
    //outDebugMsg("Wait 3sec to send CMD18\n");
    //sleep( 3 );
    if( SD_SendCmd( Cmd, argv, NULL) != 0 ) {
        outDebugMsg( "CMD%d block read fail!!\n", Cmd );
        return -1;
    }
    
    outDebugMsg( "Read Data .......... " );
    
    while( (SectorIdx < SectorCnt) && ( ret == 0 ) ) {
        ( SectorIdx % 2 ? outDebugMsg( " \\" ) :  outDebugMsg( " /" ) );
        
        uint8_t *ReadBuffer = buffer+( SectorIdx * BASE_SECTOR_SIZE );
        ret = SD_ReadData( ReadBuffer, BASE_SECTOR_SIZE );
        SectorIdx++;

        outDebugMsg( "\b\b" );
    }
    
    if( ret == 0 ) {
        outDebugMsg( "done\n" );
        if( Cmd == 18 ) {
            Cmd     = 12;
            argv    = 0;
            SD_SendCmd( 12, argv, NULL);
           //     outDebugMsg( "CMD%d block read fail!!\n", Cmd );
           //     return -1;
           // }
        }
    }
    else {
        outDebugMsg( "fail\n" );
    }
    
    if( GPIO_CSH != NULL ) GPIO_CSH();
    
    return ret;
}

//-------------------------------------------------------------------------
int SD_WriteLba( uint32_t LBA, uint16_t SectorCnt, uint8_t *buffer ) {
 
    SDReadByte();		/* Receive following half of R2 */
  
    uint8_t Cmd         = ( SectorCnt == 1 ? 24 : 25 );
    uint8_t StartToken  = ( Cmd == 24 ? 0xFE : 0xFC );
    uint16_t SectorIdx  = 0;
    uint32_t argv       = ( g_SD_Type == SD_TYPE_V2HC ? LBA : LBA * BASE_SECTOR_SIZE );
    int ret             = 0;
    
   // outDebugMsg("Wait 3sec to send CMD25\n");
   // sleep( 3 );
    if( SD_SendCmd( Cmd, argv, NULL) != 0 ) {
        outDebugMsg( "CMD%d block write fail!!\n", Cmd );
        return -1;
    }
    
    outDebugMsg( "Write Data .......... " );
    while( (SectorIdx < SectorCnt) && ( ret == 0 ) ) {
        
        ( SectorIdx % 2 ? outDebugMsg( " \\" ) :  outDebugMsg( " /" ) );

        uint8_t *WriteBuffer = buffer+( SectorIdx * BASE_SECTOR_SIZE );
        ret = SD_WriteData( StartToken, WriteBuffer, BASE_SECTOR_SIZE );
        SectorIdx++;

        outDebugMsg( "\b\b" );
    }
    
    if( ret == 0 ) {
        outDebugMsg( "done\n\n" );
        if( Cmd == 25 ) {
            // outDebugMsg( "send 0xFD multpile block write stop tran\n" );
            
            SD_WriteData( 0xFD, NULL, 0 ); //SDWriteByte( 0xFD );
        }
        
        SDReadByte();
    }
    else {
        outDebugMsg("fail\n\n");
        ret = -1;
    }
    
    if( GPIO_CSH != NULL ) GPIO_CSH();
  
    return ret;
}
