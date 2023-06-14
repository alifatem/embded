#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */


void PLL_init(void);
void myDelay(void);
void ATD0_init(void);
unsigned char ATD0_read(unsigned char);
void motor0_init(void);
void motor0_speed(unsigned char);
void SCI0_init(void);
void SCI0_Tx(unsigned char);


void main(void) {
  /* put your own code here */            
   
   
unsigned char myTempReading, myTempVoltage,myCurrentReading ,myCurrentVoltage,outsideTemp,actualCurrent,dcfanvoltage,consumed_power,speed,first_digit_power,second_digit_power,first_digit_temp,second_digit_temp,actualcurrentmA;






  PLL_init();
  SCI0_init();


  ATD0_init();
  motor0_init();
  
  motor0_speed(0);                                                                                       
 
  


	EnableInterrupts;


  for(;;) {
     myTempReading = ATD0_read(5);// the reading between 0-255
     myTempVoltage = (myTempReading*5000)/255; // myVoltage = (myReading/255)*5 (math is correct, but integer math will result in zeros all the time except when the reading is 255;
     outsideTemp=myTempVoltage/10;
     speed= ATD0_read(5); // vary the duty by reading the onboard POT
    speed = (speed *235)/255; //scale the 0-255 reading to 0-235 (motor speed limit, see calculations below)
    motor0_speed(speed);
     
     
     if(outsideTemp>18){
     //increase the speed of fan pwm
    motor0_speed(10);
     
     
     } else{
     motor0_speed(0);
     
     //decrease the speed of fan pwm
     }
     
     myCurrentReading=ATD0_read(6);
     myCurrentVoltage=(myCurrentReading*5000)/255;
     
     actualCurrent=(myCurrentVoltage-2500)/66;
     actualcurrentmA=actualCurrent*1000;
     
    
     dcfanvoltage=12000 ;
     
     
     
     consumed_power=actualCurrent*dcfanvoltage ;
     first_digit_power=consumed_power/10;
     second_digit_power=consumed_power%10;
     SCI0_Tx(first_digit_power+0x30);
     SCI0_Tx(second_digit_power+0x30);
     SCI0_Tx('\n');
     
     first_digit_temp=outsideTemp/10;
     second_digit_temp=outsideTemp%10 ;
     SCI0_Tx(first_digit_temp+0x30+'  '+second_digit_temp+0x30);
     SCI0_Tx('\n');
     
     
     
     
    
  
    _FEED_COP(); /* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}



void PLL_init(void){

SYNR = 2;
REFDV = 0; // at 8MHz Osc -->  48MHz PLL
PLLCTL = 0x60;// ON, Auto
while(!(CRGFLG & 0x08));//wait the lock BIT TO SET
CLKSEL = CLKSEL | 0x80; // select the PLL clk
}

void myDelay(void){
unsigned char j;
unsigned int k;

 for(j=0;j<15;j++){
   for(k=0; k<20000; k++){
     j=j;
     k=k;
   
   }
 
 }

}                 

void ATD0_init(void){
 ATD0CTL2 = 0xC0; //Power up, Fast Flag Clear
 myDelay();
 ATD0CTL3 = 0x20; //two conversion, non FIFO
 ATD0CTL4 = 0x85; // 8-bit conversiton, 2 AD clocks for stage 2, 2MHz at 24MHz E clk
 
}


unsigned char ATD0_read(unsigned char channelNo){
 ATD0CTL5 = 0xB0 | channelNo;//right justification, unsigned, single sequence, single channel

 while(!(ATD0STAT0 & 0x80));
 
 
 if(channelNo==5){
   return ATD0DR1L ;
 
  }

if(channelNo==6){
  return ATD0DR2L;
}

}


void SCI0_init(void){
    SCI0BDL = 156;
    SCI0BDH = 0; // 9600bps @ 24MHz E clk
    SCI0CR1 = 0; // No Loop, 8 data bits, no parity
    SCI0CR2 = 0x0C; // Enable Tx, Rx
      
}
void SCI0_Tx(unsigned char myByte){
  while(!(SCI0SR1 & 0x80));
  SCI0DRL = myByte;


}


void motor0_init(void){    //PWM0

  PWMCLK=0x01; // select SA
  PWMPOL=0x01; // Start high (1 polirity)// we are using a nMOS
  PWMPRCLK=0x07; //prescale the E-clock by 128 --> 24M/128 = 187.5 Khz
  PWMSCLA=0x04;//  prescale A by 8 (4*2) to get SA --> 187.5KHz/8 = 23.43KHz
  PWMCTL=0x0C; // select 8-bit mode and disable PWM in wait and freeze modes
//i.e each count will take this amount of time: (1/24MHz)*128*8 = 42.666 microseconds
  
  PWMPER0=235; // this will get us 10.02 ms period (10.02 ms =235 counts * 42.666 micro Second per count)
  PWMDTY0=235/2; // 50% duty cycle (initially)
  
  
  PWMCNT0=0x00;
  PWME=0x01; //enable PWM0 (connect your motor to PP0 i.e pin # 4)
  
}


void motor0_speed(unsigned char myspeed){
  PWMDTY0 = myspeed;

}