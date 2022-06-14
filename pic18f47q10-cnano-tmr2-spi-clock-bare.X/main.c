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

#define Timer2Period       0xC7     /* Set TMR2 period, PR2 to 199 (50us) */

static void CLK_Initialize(void);
static void PPS_Initialize(void);
static void PORT_Initialize(void);
static void TMR2_Initialize(void);
static void SPI1_Initialize(void);
static void SPI1_client1Select(void);
static void SPI1_client1Deselect(void);
static void SPI1_client2Select(void);
static void SPI1_client2Deselect(void);
static uint8_t SPI1_exchangeByte(uint8_t data);

uint8_t writeData = 1;          /* Data that will be transmitted */
uint8_t receiveData;            /* Data that will be received */

static void CLK_Initialize(void)
{
    OSCCON1 = 0x60;             /* set HFINTOSC Oscillator */
    OSCFRQ  = 0x02;             /* set HFFRQ to 4 MHz */
}

static void PPS_Initialize(void)
{  
    RC3PPS = 0x0F;              /* SCK channel on RC3 */
    SSP1DATPPS = 0x14;          /* SDI channel on RC4 */
    RC5PPS = 0x10;              /* SDO channel on RC5 */
}

static void PORT_Initialize(void)
{
    ANSELC = 0x07;      /* Set RC6 and RC7 pins as digital */
    TRISC  = 0x17;      /* Set SCK, SDO, SS1, SS2 as output and SDI as input */
}

static void TMR2_Initialize(void)
{
    /* TMR2 Clock source, HFINTOSC (00011) */
    T2CLKCON = 0x03;
    /* T2PSYNC Not Synchronized, T2MODE Software control, T2CKPOL Rising Edge */
    T2HLT = 0x00;
    /* TMR2ON on; T2CKPS Prescaler 1:1; T2OUTPS Postscaler 1:1 */
    T2CON = 0x80;
    /* Set TMR2 period, PR2 to 199 (50us) */
    T2PR = Timer2Period;
    /* Clear the TMR2 interrupt flag */
    PIR4bits.TMR2IF = 0;
}

static void SPI1_Initialize(void)
{  
    /* SSP1ADD = 1 */
    SSP1ADD = 0x01;
    /* Enable module, SPI Host Mode, TMR2 as clock source */
    SSP1CON1 = 0x23;
}

static void SPI1_client1Select(void)
{
    LATCbits.LATC6 = 0;          /* Set SS1 pin value to LOW */
}

static void SPI1_client1Deselect(void)
{
    LATCbits.LATC6 = 1;          /* Set SS1 pin value to HIGH */
}

static void SPI1_client2Select(void)
{
    LATCbits.LATC7 = 0;          /* Set SS2 pin value to LOW */    
}

static void SPI1_client2Deselect(void)
{
    LATCbits.LATC7 = 1;           /* Set SS2 pin value to HIGH */    
}

static uint8_t SPI1_exchangeByte(uint8_t data)
{
    SSP1BUF = data;
    
    while(!PIR3bits.SSP1IF) /* Wait until data is exchanged */
    {
        ;
    }   
    PIR3bits.SSP1IF = 0;
    
    return SSP1BUF;
}

int main(void)
{
    CLK_Initialize();
    PPS_Initialize();
    PORT_Initialize();
    TMR2_Initialize();
    SPI1_Initialize();
    
    while(1)
    {
        SPI1_client1Select();
        receiveData = SPI1_exchangeByte(writeData);
        SPI1_client1Deselect();
        
        SPI1_client2Select();
        receiveData = SPI1_exchangeByte(writeData);
        SPI1_client2Deselect();
    }
}
