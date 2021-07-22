#include <stdio.h>
#include "gpiofunc.h"
#include "bcm2835.h"						/* Include device specific declaration file here */

#define		INIT_PORT()	init_port()			/* Initialize MMC control port (CS=H, CLK=L, DI=H, DO=in) */
#define 	DLY_US(n)	bcm2835_delay(n)	/* Delay n microseconds */

#define		CS_H()		bcm2835_gpio_set(RPI_GPIO_P1_26)	/* Set MMC CS "high" */
#define 	CS_L()		bcm2835_gpio_clr(RPI_GPIO_P1_26)	/* Set MMC CS "low" */
#define 	CK_H()		bcm2835_gpio_set(RPI_GPIO_P1_23)	/* Set MMC SCLK "high" */
#define		CK_L()		bcm2835_gpio_clr(RPI_GPIO_P1_23)	/* Set MMC SCLK "low" */
#define 	DI_H()		bcm2835_gpio_set(RPI_GPIO_P1_19)	/* Set MMC DI "high" */
#define 	DI_L()		bcm2835_gpio_clr(RPI_GPIO_P1_19)	/* Set MMC DI "low" */
#define 	DO			bcm2835_gpio_lev(RPI_GPIO_P1_21)	/* Test for MMC DO ('H':true, 'L':false) */

/*-----------------------------------------------------------------------*/
/* Transmit bytes to the card (bitbanging)                               */
/*-----------------------------------------------------------------------*/

static void xmit_mmc (
               const uint8_t* buff,	/* Data to be sent */
               uint16_t bc				/* Number of bytes to send */
)
{
    uint8_t d;
    
    do {
        d = *buff++;	/* Get a byte to be sent */
        if (d & 0x80) DI_H(); else DI_L();	/* bit7 */
        CK_H(); CK_L();
        if (d & 0x40) DI_H(); else DI_L();	/* bit6 */
        CK_H(); CK_L();
        if (d & 0x20) DI_H(); else DI_L();	/* bit5 */
        CK_H(); CK_L();
        if (d & 0x10) DI_H(); else DI_L();	/* bit4 */
        CK_H(); CK_L();
        if (d & 0x08) DI_H(); else DI_L();	/* bit3 */
        CK_H(); CK_L();
        if (d & 0x04) DI_H(); else DI_L();	/* bit2 */
        CK_H(); CK_L();
        if (d & 0x02) DI_H(); else DI_L();	/* bit1 */
        CK_H(); CK_L();
        if (d & 0x01) DI_H(); else DI_L();	/* bit0 */
        CK_H(); CK_L();
    } while (--bc);
}



/*-----------------------------------------------------------------------*/
/* Receive bytes from the card (bitbanging)                              */
/*-----------------------------------------------------------------------*/

static void rcvr_mmc (
               uint8_t *buff,	/* Pointer to read buffer */
               uint16_t bc		/* Number of bytes to receive */
)
{
    uint8_t r;
    
    
    DI_H();	/* Send 0xFF */
    
    do {
        r = 0;	 if (DO) r++;	/* bit7 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;	/* bit6 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;	/* bit5 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;	/* bit4 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;	/* bit3 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;	/* bit2 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;	/* bit1 */
        CK_H(); CK_L();
        r <<= 1; if (DO) r++;	/* bit0 */
        CK_H(); CK_L();
        *buff++ = r;			/* Store a received byte */
    } while (--bc);
}


/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
int wait_ready (void)	/* 1:OK, 0:Timeout */
{
    uint8_t d;
    uint16_t tmr;
    
    
    for (tmr = 5000; tmr; tmr--) {	/* Wait for ready in timeout of 500ms */
        rcvr_mmc(&d, 1);
        if (d == 0xFF) break;
        DLY_US(100);
    }
    
    return tmr ? 1 : 0;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/
void GPIO_Deselect (void)
{
    CS_H();
}



/*-----------------------------------------------------------------------*/
/* Select the card and wait for ready                                    */
/*-----------------------------------------------------------------------*/
void  GPIO_Select (void)	/* 1:OK, 0:Timeout */
{
    CS_L();
    if (wait_ready() ) {
        //return 1;	/* OK */
    }
    else {
        GPIO_Deselect();
        //return 1;
    }
}

//-----------------------------------------------------------------------
void GPIO_InitPort( void ) {
    bcm2835_init();
    
    bcm2835_gpio_fsel(RPI_GPIO_P1_23, BCM2835_GPIO_FSEL_OUTP); // CLK
    bcm2835_gpio_clr(RPI_GPIO_P1_23);
    bcm2835_gpio_fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_OUTP); // MOSI
    bcm2835_gpio_set(RPI_GPIO_P1_19);
    bcm2835_gpio_fsel(RPI_GPIO_P1_26, BCM2835_GPIO_FSEL_OUTP); // CE1
    bcm2835_gpio_set(RPI_GPIO_P1_26);
    bcm2835_gpio_fsel(RPI_GPIO_P1_21, BCM2835_GPIO_FSEL_INPT); // MISO
}

//-----------------------------------------------------------------------
int GPIO_WriteByte( uint8_t buf ) {
    xmit_mmc( &buf, 1 );
    return 0;
}

//-----------------------------------------------------------------------
int GPIO_ReadByte(void) {
   
    uint8_t RecBuf = 0;
    
    rcvr_mmc( &RecBuf, 1 );

    //  printf("---> rcvr_mmc=%02X\n", RecBuf );
    

    return RecBuf;
}


