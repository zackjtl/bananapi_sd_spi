#include "global.h"
#include "spifunc.h"
#include "gpiofunc.h"
#include "sdfunc.h"

#define __RUN_GPIO_MODE__ 0


//----------------------------------------------------------------------------------
// Global Var
//----------------------------------------------------------------------------------
static const char *deviceName = "/dev/spidev0.0";
CSD         csdinfo;
FILE*       FileHandle  = NULL;
uint32_t    testTime    = 1;
//----------------------------------------------------------------------------------
// Internal Common functions
//----------------------------------------------------------------------------------
void pabort( const char *s )
{
    perror(s);
    abort();
    
}

void outMsg(  const char * format, ...  )
{
#define DEBUG_MSG_OUT 1
#if DEBUG_MSG_OUT
    char msg[1024] = { 0x00 };
    va_list argptr;
    va_start(argptr, format);
    vsprintf( msg, format, argptr);
    va_end(argptr);
    printf( "%s", msg );
    fflush(stdout);
    
    if( FileHandle != NULL ) {
        fseek( FileHandle, 0, SEEK_END );
        fwrite( msg, 1, strlen(msg), FileHandle );
    }
    
#endif
}


void DumpBuffer( uint8_t *buffer, int length,  const char * format, ...  )
{
    char msg[1024] = { 0x00 };
    va_list argptr;
    va_start(argptr, format);
    vsprintf( msg, format, argptr);
    va_end(argptr);
    printf( "%s\n", msg );
    
    int i = 0;
    for( i =0; i<length; i++  ) {
        printf( "%02X ", buffer[i] );
        
        if( i%16 == 15 )
            printf("\n");
    }
    if( length < 15 ) printf("\n");
    
}

void print_usage(const char *prog)
{
    printf("Usage: %s [-Dst]\n", prog);
    puts("  -D --device   device to use (default /dev/spidev1.1)\n"
         "  -s --speed    max speed (Hz)\n"
         "  -t --time     max test time (hrs)\n" );
    exit(1);
}


void parse_opts(int argc, char *argv[])
{
    while (1) {
        static const struct option lopts[] = {
            { "device",  1, 0, 'D' },
            { "speed",   1, 0, 's' },
            { "time",    1, 0, 't' },
            { "help",    0, 0, 'h' },
            { NULL, 0, 0, 0 },
        };
        int c;
    
        c = getopt_long(argc, argv, "D:s:t:h:", lopts, NULL);
    
        if (c == -1) {
             return; //break;
        }
        switch (c) {
            case 'D':
                deviceName = optarg;
 //               printf(" Device name = %s\n", optarg );
                break;
            case 's':
                device.speed = atoi(optarg);
  //              printf("Speed = %d\n", atoi(optarg) );
                break;
            case 't':
                testTime = atoi(optarg);
    //            printf("Test Time = %d hrs\n", atoi(optarg) );
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                break;
        }
    }
}


int capacity = 0;


//-------------------------------------------------------------------------
// Test Sample Code Start
//-------------------------------------------------------------------------
ACCESSARGV GetRandAddress( uint32_t Capacity )
{
    srand ( time(NULL) );
    
    ACCESSARGV Lba;
    

    uint32_t ZoneSize           = 4;
    uint32_t CapacityPerZone    = Capacity / ZoneSize;
    uint32_t TestZone           = (uint32_t)( rand()%ZoneSize );
    uint32_t StartAddr          = CapacityPerZone * TestZone;
    Lba.StartLba.Addr           = (uint32_t)(rand()%CapacityPerZone) + StartAddr;
    Lba.ReadCnt                 = (uint16_t)( rand()%128 + 1 );
    
    if( Lba.ReadCnt == 1 ) Lba.ReadCnt = 2;

    int diff                    = ( Lba.StartLba.Addr + Lba.ReadCnt ) - capacity;

    
    if( diff >0  )
    {
        Lba.StartLba.Addr = Lba.StartLba.Addr - diff;
   }
    
    return Lba;
}
//-------------------------------------------------------------------------
void CreateTestPattern( uint8_t *buffer, uint32_t length ) {
    char  Dir[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789~!@#$%^&*()_+-=`,./<>?;':""[]\\{}|";
    uint16_t DirLen = strlen( Dir );
    uint32_t i = 0;
    
    srand ( time(NULL) );
    
    for( i = 0; i < length; i++ ) {
        buffer[i] = Dir[ ( rand()%DirLen  ) ];
    }
}
//-------------------------------------------------------------------------
int DoReadWriteTest( void )
{
    int ret                 = 0;
    uint32_t PassCnt        = 0;
    uint32_t FailCnt        = 0;
    time_t start_t, end_t;
    double diff_t           = 0;
    uint32_t stopTime       = 3600 * ( testTime =  ( testTime == 0) ? 1 : testTime );
    uint8_t *WriteBuffer    = NULL;
    uint8_t *ReadBuffer     = NULL;
    WriteBuffer             = ALLOC_MEMORY( uint8_t, 512*128 );
    ReadBuffer              = ALLOC_MEMORY( uint8_t, 512*128);
    
    
    if( WriteBuffer == NULL || ReadBuffer == NULL ) {
        outMsg("\nCan't allocate Read / Write buffer!\n");
        return -1;
    }
    
    time(&start_t);
    
    end_t = start_t;
    
    while( 1 ) {
        
        system("clear");
        ACCESSARGV TestLba      = GetRandAddress( capacity/512 );
        uint32_t bufsize        = TestLba.ReadCnt * BASE_SECTOR_SIZE;

        diff_t = difftime(end_t, start_t);
        outMsg( "-------------------------------------------------------------------------------\n");
#if( __RUN_GPIO_MODE__ )
        outMsg( "Mode: Using GPIO control SPI Interface\n");
#else
        outMsg( "spi mode: %d\n", device.mode);
        outMsg( "bits per word: %d\n", device.bits);
        outMsg( "max speed: %d Hz (%d KHz)\n", device.speed, device.speed/1000);
#endif
        outMsg( "-------------------------------------------------------------------------------\n");
        outMsg( "Disk Capacity  = %d sector(s), %d MB\n", capacity, capacity/2048 );
        outMsg( "Start Lba Addr = 0x%08X | Sector Count = 0x%08X\n", TestLba.StartLba.Addr, TestLba.ReadCnt  );
        outMsg( "PASS Count %-3s = %10d | FAIL Count %-1s = %10d\n", " ", PassCnt," ", FailCnt );
        outMsg( "-------------------------------------------------------------------------------\n");
        outMsg( "Set Test time  %02d:%02d:%02d\n", (int)(stopTime/3600), (int)(fmod(stopTime,3600)/60), (int)fmod( stopTime,60 )  );
        outMsg( "Execution time %02d:%02d:%02d\n", (int)(diff_t/3600), (int)(fmod(diff_t,3600)/60), (int)fmod( diff_t,60 )  );
        outMsg( "\n" );
        
        if( (uint32_t)diff_t >= stopTime ) break;
         
        FILL_MEMORY( WriteBuffer, 0x00, bufsize );
        FILL_MEMORY( ReadBuffer, 0x00, bufsize );
        
        CreateTestPattern( WriteBuffer, bufsize );

        if( ( ret = SD_WriteLba( TestLba.StartLba.Addr, TestLba.ReadCnt, WriteBuffer ) ) == -1 ) {
            break;
        }
	
        if( ( ret = SD_ReadLba( TestLba.StartLba.Addr, TestLba.ReadCnt, ReadBuffer ) ) == -1 ) {
            break;
        }
    
        if( memcmp( WriteBuffer, ReadBuffer, bufsize ) == 0 ) {
            outMsg("\nData verify .......... ok!\n");
            PassCnt++;
        }
        else {
            outMsg("\nData verify .......... error!\n");
            FailCnt++;
        }

        int k = 0;
        outMsg( "\n\nWait ");
        for( k=0; k<3; k++  ) {
            outMsg( "." );
            sleep( 1 );
        }
        outMsg( "\n");
        
        time(&end_t);
    }
    
    FREE_MEMORY( WriteBuffer );
    FREE_MEMORY( ReadBuffer );
    
    return ret;
}
//-------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    
    int ret = 0;
   
#if( __RUN_GPIO_MODE__ )
    printf("GPIO");
    GPIO_InitPort();
    
    //---- SD Init ----
    SD_SetSPIWriteFunc( &GPIO_WriteByte );
    SD_SetSPIReadFunc( &GPIO_ReadByte );
    SD_SetCSHighFunc( &GPIO_Deselect );
    SD_SetCSLowFunc( &GPIO_Select );

#else
    parse_opts(argc, argv);

    device.fd = open(deviceName, O_RDWR );
    if (device.fd < 0) {
        outMsg("\ncan't open device\n");
        return -1;
    }
    device.mode     = 0;
    device.speed    = ( device.speed == 0 ) ? 25000 : device.speed;
    device.bits     = 8;
    
    //---- SPI Init ----
    if( Enabled_Default_Mode( device.mode  ) == -1 ) goto __FuncEND;
    if( SPI_SetTransferSpeed() == -1 ) goto __FuncEND;
    if( SPI_SetBitsPerWord() == -1 ) goto __FuncEND;

    //---- SD Init ----
    SD_SetSPIWriteFunc( &SPI_WriteByte );
#endif
    
    if( SD_Init() == -1 ) goto __FuncEND;
    if( (capacity = SD_ReadCSD( &csdinfo ) ) == -1 ) goto __FuncEND;
    if( SD_SetBlockLength( BASE_SECTOR_SIZE) == -1 ) goto __FuncEND;

    //---- Read/Write ----
    DoReadWriteTest();

    
    /*
    pid_t pid;
    pid = fork();
    
    if( pid < 0 ) {
        outMsg( "Fork Failed\n" );
    }
    else if(  pid == 0 ) {
        DoReadWriteTest();
    }
    else {
        wait( NULL );
        outMsg("\n\nTest Completed\n");
    }
    */
    
__FuncEND:
    
    fclose( FileHandle );

#if( !__RUN_GPIO_MODE__ )
    close( device.fd );
#endif
    return ret;
}
