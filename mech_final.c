#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "clock.h"
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "uart0.h"
#include "wait.h"

#define DATA   PORTB,0
#define PD_SCK PORTB,5
#define PUSH_BUTTON PORTB,1

void initHw() {
    initSystemClockTo40Mhz();
    enablePort(PORTB);

    selectPinDigitalInput(DATA);
    selectPinPushPullOutput(PD_SCK);

    selectPinDigitalInput(PUSH_BUTTON);
    enablePinPullup(PUSH_BUTTON);
}
void pulse() {
    setPinValue(PD_SCK, 1);
    waitMicrosecond(1);
    setPinValue(PD_SCK, 0);
//    waitMicrosecond(2);
}

#define WAIT_T 10
#define SAMPLES 25
int32_t getData() {
    /*
     * wait until DATA is low
     * loop 24 times
     *      pulse
     *      read
     *      shift left by 1;
     */
    int32_t data = 0;
    uint8_t i = 0;
    while(getPinValue(DATA) == 1);
    setPinValue(PD_SCK, 0);
    waitMicrosecond(WAIT_T);
    for(i = 0; i < 24; i++){

//        pulse();
        setPinValue(PD_SCK, 1);
        waitMicrosecond(1);
        if(getPinValue(DATA))
            data++;
        if(i==0)
        {
            if(data == 1)
            {
                data = 0xFF;
            }
            else
            {
                data = 0x00;
            }
        }
        data = data << 1;
//        uint8_t temp = getDataBit() & 0x01;
//        data = shift(data, temp);

        setPinValue(PD_SCK, 0);
        waitMicrosecond(WAIT_T);
    }
//    pulse();    //sets amplifier to 128 G
    setPinValue(PD_SCK, 1);
    waitMicrosecond(1);
//    data ^= 0x800000;
    setPinValue(PD_SCK, 0);
    waitMicrosecond(WAIT_T);
    return data;
}

int32_t average(void)
{
    int32_t total = 0;
    int i;
    for(i = 0; i < SAMPLES; i++)
    {
        total += getData();
    }
    total = (total / SAMPLES);
    return total;
}

void tare(int32_t* offset)
{
    int32_t temp = average();
    *offset = temp;
}

int32_t calculate(int32_t offset)
{
    int32_t ave;
    ave = average();
   // ave = ((ave - offset));
    ave = ((ave *.000041908) - offset);
    return ave;
}

void waitPbPress()
{
    while(getPinValue(PUSH_BUTTON) != 1);
}

//33547000 +- 999
int main(void)
{
    initHw();
    initUart0();
    char buffer[20];
    int32_t offset = 0;
   // tare(&offset);
    while(true) {
        if(getPinValue(PUSH_BUTTON) == 0)
        {
            tare(&offset);
        }
       //int32_t data = getData();
        int32_t data = calculate(offset);
        sprintf(buffer, "testdata = %d", data);
        putsUart0(buffer);
        putsUart0("\n");
       // waitMicrosecond(50000);
    }
    return 0;
}
