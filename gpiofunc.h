#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdint.h>
#include "time.h"



extern void GPIO_InitPort( void );
extern int GPIO_WriteByte( uint8_t buf );
extern int GPIO_ReadByte(void);
extern void GPIO_Deselect (void);
extern void GPIO_Select(void);
#endif


