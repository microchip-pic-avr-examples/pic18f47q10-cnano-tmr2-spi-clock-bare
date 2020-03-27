/**
 * \file main.c
 *
 * \brief Main source file.
 *
 (c) 2020 Microchip Technology Inc. and its subsidiaries.
    Subject to your compliance with these terms, you may use this software and
    any derivatives exclusively with Microchip products. It is your responsibility
    to comply with third party license terms applicable to your use of third party
    software (including open source software) that may accompany Microchip software.
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE.
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 */

#pragma config WDTE = OFF           /* WDT operating mode->WDT Disabled */ 
#pragma config LVP = ON             /* Low voltage programming enabled, RE3 pin is MCLR */ 

#include <xc.h>
#include <stdint.h>

#define PPS_CONFIG_RC3_SPI_SCK      0x0F
#define PPS_CONFIG_RC4_SPI_SDI      0x14
#define PPS_CONFIG_RC5_SPI_SDO      0x10
#define Timer2Period                0xC7

static void CLK_init(void);
static void PPS_init(void);
static void PORT_init(void);
static void TMR2_init(void);
static void SPI1_init(void);
static void SPI1_slave1Select(void);
static void SPI1_slave1Deselect(void);
static void SPI1_slave2Select(void);
static void SPI1_slave2Deselect(void);
static uint8_t SPI1_exchangeByte(uint8_t data);

uint8_t writeData = 1;          /* Data that will be transmitted */
uint8_t receiveData;            /* Data that will be received */

static void CLK_init(void)
{
    OSCCON1 = _OSCCON1_NOSC1_MASK 
            | _OSCCON1_NOSC2_MASK;        /* HFINTOSC Oscillator */
    OSCFRQ = _OSCFRQ_FRQ1_MASK;           /* HFFRQ 4 MHz */
}

static void PPS_init(void)
{
    RC3PPS = PPS_CONFIG_RC3_SPI_SCK;               /* SCK channel on RC3 */
    SSP1DATPPS = PPS_CONFIG_RC4_SPI_SDI;           /* SDI channel on RC4 */
    RC5PPS = PPS_CONFIG_RC5_SPI_SDO;               /* SDO channel on RC5 */
}

static void PORT_init(void)
{
    /* Set RC6 and RC7 pins as digital */
    ANSELC = ~_ANSELC_ANSELC6_MASK & ~_ANSELC_ANSELC7_MASK;
    TRISC &= ~_TRISC_TRISC3_MASK;       /* SCK channel as output */
    TRISC |= _TRISC_TRISC4_MASK;        /* SDI channel as input */
    TRISC &= ~_TRISC_TRISC5_MASK;       /* SDO channel as output */
    TRISC &= ~_TRISC_TRISC6_MASK;       /* SS1 channel as output */
    TRISC &= ~_TRISC_TRISC7_MASK;       /* SS2 channel as output */
}

static void TMR2_init(void)
{
    /* TMR2 Clock source, HFINTOSC (00011) */
    T2CLKCON = _T2CLKCON_CS0_MASK | _T2CLKCON_CS1_MASK;
    /* T2PSYNC Not Synchronized; T2MODE Starts at T2ON = 1 and TMR2_ers = 0; T2CKPOL Rising Edge */
    T2HLT = 0x00; 
    /* TMR2ON on; T2CKPS Prescaler 1:1; T2OUTPS Postscaler 1:1 */
    T2CON |= _T0CON1_T0CS2_MASK;
    /* Set TMR2 period, PR2 to 50us */
    T2PR = Timer2Period;
    /* Clear the TMR2 interrupt flag */
    PIR4 &= ~_PIR4_TMR2IF_MASK;
}

static void SPI1_init(void)
{  
    /* SSP1ADD = 1 */
    SSP1ADD = _SSP1ADD_MSK0_MASK;
    /* Enable module, SPI Master Mode, TMR2 as clock source */
    SSP1CON1 = _SSP1CON1_SSPEN_MASK
             | _SSP1CON1_SSPM0_MASK
             | _SSP1CON1_SSPM1_MASK; 
}

static void SPI1_slave1Select(void)
{
    PORTC &= ~_PORTC_RC6_MASK;          /* Set SS1 pin value to LOW */
}

static void SPI1_slave1Deselect(void)
{
    PORTC |= _PORTC_RC6_MASK;           /* Set SS1 pin value to HIGH */
}

static void SPI1_slave2Select(void)
{
    PORTC &= ~_PORTC_RC7_MASK;          /* Set SS2 pin value to LOW */    
}

static void SPI1_slave2Deselect(void)
{
    PORTC |= _PORTC_RC7_MASK;           /* Set SS2 pin value to HIGH */    
}

static uint8_t SPI1_exchangeByte(uint8_t data)
{
    SSP1BUF = data;
    
    while(!(SSP1STAT & _SSP1STAT_BF_MASK))   /* Wait until data is exchanged */
    {
        ;
    }
    
    return SSP1BUF;
}

int main(void)
{
    CLK_init();
    PPS_init();
    PORT_init();
    TMR2_init();
    SPI1_init();
    
    while(1)
    {
        SPI1_slave1Select();
        receiveData = SPI1_exchangeByte(writeData);
        SPI1_slave1Deselect();
        SPI1_slave2Select();
        receiveData = SPI1_exchangeByte(writeData);
        SPI1_slave2Deselect();
    }
}
