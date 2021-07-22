#include "spifunc.h"

//-------------------------------------------------------------------------
CSPIDEV     device;
//-------------------------------------------------------------------------
int IsHandleVaild( void ) {
    return ( device.fd == -1 ? -1 : 0 );
}
//-------------------------------------------------------------------------
// SPI Functions
//-------------------------------------------------------------------------
int SPI_DataTransfer( uint8_t value )
{
    int ret;
    uint8_t rx = 0;
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)&value,
        .rx_buf = (unsigned long)&rx,
        .len = 1,
        .delay_usecs    = device.delay,
        .speed_hz       = device.speed,
        .bits_per_word  = device.bits,
    };
    
    ret = ioctl( device.fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1){
        printf("\n can't send spi message \n ");
        abort();
    }
    else
        ret = rx;
    
    return ret;
}
//-------------------------------------------------------------------------
int SPI_SetTransferSpeed( void ) {
    /*
     * max speed hz
     */
    int ret = 0;
    
    ret = ioctl( device.fd, SPI_IOC_WR_MAX_SPEED_HZ, &device.speed);
    if (ret == -1) {
        // outMsg("can't set max speed hz\n" );
    }
    else {
        ret = ioctl( device.fd, SPI_IOC_RD_MAX_SPEED_HZ, &device.speed);
        if (ret == -1) {
            // outMsg("can't get max speed hz\n");
        }
    }
    
    return ret;
}
//-------------------------------------------------------------------------
int SPI_SetBitsPerWord( void ) {
    int ret = 0;
    ret = ioctl(device.fd, SPI_IOC_WR_BITS_PER_WORD, &device.bits);
    if (ret == -1) {
        // outMsg("can't set bits per word\n");
    }
    else {
        ret = ioctl(device.fd, SPI_IOC_RD_BITS_PER_WORD, &device.bits);
        if (ret == -1) {
            // outMsg("can't get bits per word\n");
        }
    }
    return ret;
}
//-------------------------------------------------------------------------
int SPI_SetMode( void ) {
    uint8_t retModeVal;
    
    if( IsHandleVaild() == -1 ) {
        // outMsg( "can't access device\n" );
        return -1;
    }
    
    if (ioctl( device.fd, SPI_IOC_WR_MODE, &device.mode ) == -1) {
        // outMsg("IOCTL::SPI_IOC_WR_MODE\n");
        return -1;
    }
    if (ioctl( device.fd, SPI_IOC_RD_MODE, &retModeVal ) == -1) {
        // outMsg("IOCTL::SPI_IOC_RD_MODE\n");
        return -1;
    }
    if (retModeVal != device.mode) {
        
        // outMsg( "retModeVal = %d, mode = %d\n", retModeVal, device.mode );
        return -1;
    }
    return 0;
}
//-------------------------------------------------------------------------
int SPI_WriteByte( uint8_t buf ) {
    
    int ret = 0;
    
    if( ( ret = IsHandleVaild() ) == 0 ) {
        ret = SPI_DataTransfer( buf );
    }
    
    if( ret < 0 ) {
        // outMsg( "Write Byte Fail!!\n" );
    }
    return ret;
}
//-------------------------------------------------------------------------
int SPI_Cs_Select()
{
    uint8_t mode = device.mode;
    
    device.mode |= SPI_CS_HIGH;
    
    int ret =  SPI_SetMode();
    
    device.mode = mode;
    
    return ret;
}
//-------------------------------------------------------------------------
int SPI_Cs_Deselect()
{
    uint8_t mode = device.mode;
    
    device.mode |= SPI_NO_CS;
    
    int ret =  SPI_SetMode();
    
    device.mode = mode;
    
    return ret;
}


//-------------------------------------------------------------------------
int Enabled_Default_Mode( uint8_t ModeType ) {
    
    if( ModeType == 1 )
        device.mode = SPI_MODE_1;
    else if( ModeType == 2 )
        device.mode = SPI_MODE_2;
    else if( ModeType == 3 )
        device.mode = SPI_MODE_3;
    else
        device.mode = SPI_MODE_0;
    
    return SPI_SetMode();
}
