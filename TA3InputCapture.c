// TA3InputCapture.c
// Runs on MSP432
// Use Timer A3 in capture mode to request interrupts on rising
// edges of P10.4 (TA3CCP0) and P8.2 (TA3CCP2) and call user
// functions.
// Use Timer A3 in capture mode to request interrupts on rising
// edges of P10.4 (TA3CCP0) and P10.5 (TA3CCP1) and call user
// functions.
// Daniel Valvano
// July 11, 2019

/* This example accompanies the book
   "Embedded Systems: Introduction to Robotics,
   Jonathan W. Valvano, ISBN: 9781074544300, copyright (c) 2019
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2019, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/

// external signal connected to P10.5 (TA3CCP1) (trigger on rising edge)
// external signal connected to P10.4 (TA3CCP0) (trigger on rising edge)

#include <stdint.h>
#include "msp.h"

void ta3dummy(uint16_t t){};   // dummy function

void (*CaptureTask0)(uint16_t time);//,uint16_t pwm);// user function
void (*CaptureTask1)(uint16_t time);//,uint16_t pwm);// user function
void (*CaptureTask3)(uint16_t time);//,uint16_t pwm);// user function
void (*CaptureTask4)(uint16_t time);//,uint16_t pwm);// user function


//------------TimerA3Capture_Init01------------
// Initialize Timer A3 in edge time mode to request interrupts on
// the rising edges of P10.4 (TA3CCP0) and P10.5 (TA3CCP1).  The
// interrupt service routines acknowledge the interrupt and call
// a user function.
// Input: task0 is a pointer to a user function called when P10.4 (TA3CCP0) edge occurs
//              parameter is 16-bit up-counting timer value when P10.4 (TA3CCP0) edge occurred (units of 0.083 usec)
//        task1 is a pointer to a user function called when P10.5 (TA3CCP1) edge occurs
//              parameter is 16-bit up-counting timer value when P10.5 (TA3CCP1) edge occurred (units of 0.083 usec)
// Output: none
// Assumes: low-speed subsystem master clock is 12 MHz
void TimerA3Capture_Init01(void(*task0)(uint16_t time),void(*task1)(uint16_t time)){
  CaptureTask3 = task0;
  CaptureTask4 = task1; // user function
  // initialize
  P10->SEL0 |= 0x30;
  P10->SEL1 &= ~0x30;
  P10->DIR &= ~0x30;// configure P10.4,5as TA3CCP0 check this against book.
  TIMER_A3->CTL &= ~0x0030;               // halt Timer A3
  // bits15-10=XXXXXX, reserved
  // bits9-8=10,       clock source to SMCLK
  // bits7-6=00,       input clock divider /1
  // bits5-4=00,       stop mode
  // bit3=X,           reserved
  // bit2=0,           set this bit to clear
  // bit1=0,           interrupt disable
  // bit0=0,           clear interrupt pending
  TIMER_A3->CTL = 0x0200;
  // bits15-14=01,     capture on rising edge
  // bits13-12=00,     capture/compare input on CCI0A
  // bit11=1,          synchronous capture source
  // bit10=X,          synchronized capture/compare input
  // bit9=X,           reserved
  // bit8=1,           capture mode
  // bits7-5=XXX,      output mode
  // bit4=1,           enable capture/compare interrupt
  // bit3=X,           read capture/compare input from here
  // bit2=X,           output this value in output mode 0
  // bit1=X,           capture overflow status
  // bit0=0,           clear capture/compare interrupt pending
  TIMER_A3->CCTL[0] = 0x4910;
  TIMER_A3->CCTL[1] = 0x4910;
  TIMER_A3->EX0 &= ~0x0007;       // configure for input clock divider /1
  NVIC->IP[3] = (NVIC->IP[3]&0xFFFFFF00)|0x00000040; // priority 2
// interrupts enabled in the main program after all devices initialized
  NVIC->ISER[0] = 0x0000C000; // enable interrupt 15?? in NVIC
  TIMER_A3->CTL |= 0x0024;        // reset and start Timer A3 in continuous up mode
  // bits15-10=XXXXXX, reserved
  // bits9-8=10,       clock source to SMCLK
  // bits7-6=00,       input clock divider /1
  // bits5-4=10,       continuous count up mode
  // bit3=X,           reserved
  // bit2=1,           set this bit to clear
  // bit1=0,           interrupt disable (no interrupt on rollover)
  // bit0=0,           clear interrupt pending
}
// examine system tick interrupt systick handler.
//void TimerA3Capture_Init(int(*task0)(uint16_t time),int(*task1)(uint16_t time)){
//  CaptureTask3 = task0;
//  CaptureTask4 = task1; // user function
//  // initialize
//
//  P10->SEL0 |= 0x30;
//  P10->SEL1 &= ~0x30;
//  P10->DIR &= ~0x30;// configure P10.4,5as TA3CCP0 check this against book. 0011 0000
//                   // make P10.4,5 in
//  TIMER_A3->CTL &= ~0x0030;               // halt Timer A3
//  // bits15-10=XXXXXX, reserved
//  // bits9-8=10,       clock source to SMCLK
//  // bits7-6=00,       input clock divider /1
//  // bits5-4=00,       stop mode
//  // bit3=X,           reserved
//  // bit2=0,           set this bit to clear
//  // bit1=0,           interrupt disable
//  // bit0=0,           clear interrupt pending
//  TIMER_A3->CTL = 0x0200;
//  // bits15-14=01,     capture on rising edge
//  // bits13-12=00,     capture/compare input on CCI0A
//  // bit11=1,          synchronous capture source
//  // bit10=X,          synchronized capture/compare input
//  // bit9=X,           reserved
//  // bit8=1,           capture mode
//  // bits7-5=XXX,      output mode
//  // bit4=1,           enable capture/compare interrupt
//  // bit3=X,           read capture/compare input from here
//  // bit2=X,           output this value in output mode 0
//  // bit1=X,           capture overflow status
//  // bit0=0,           clear capture/compare interrupt pending
//  TIMER_A3->CCTL[0] = 0x4910;
//  TIMER_A3->CCTL[1] = 0x4910;
//  TIMER_A3->EX0 &= ~0x0007;       // configure for input clock divider /1
//  NVIC->IP[3] = (NVIC->IP[3]&0xFFFFFF00)|0x00000040; // priority 2 // needs to be IP[3]
//// interrupts enabled in the main program after all devices initialized
//  NVIC->ISER[0] = 0x0000C000; // enable interrupt 15 and 14?? in NVIC   Figure this out.
//
//  TIMER_A3->CTL |= 0x0024;        // reset and start Timer A3 in continuous up mode
//  // bits15-10=XXXXXX, reserved
//  // bits9-8=10,       clock source to SMCLK
//  // bits7-6=00,       input clock divider /1
//  // bits5-4=10,       continuous count up mode
//  // bit3=X,           reserved
//  // bit2=1,           set this bit to clear
//  // bit1=0,           interrupt disable (no interrupt on rollover)
//  // bit0=0,           clear interrupt pending
//}

void TA3_0_IRQHandler(void){
  TIMER_A3->CCTL[0] &= ~0x0001;// acknowledge capture/compare interrupt 0
  (*CaptureTask3)(TA3CCR0);//,TIMER_A3->CCR[1]);         // execute user task with input TimerA3->CCR0
}



void TA3_N_IRQHandler(void){
    // write this for Lab 16
    if (TIMER_A3->CCTL[1]&0x0001){
        TIMER_A3->CCTL[1] &= ~0x0001;
        (*CaptureTask4)(TA3CCR1);//,TIMER_A3->CCR[2]);
    }

}


// old robot code

//------------TimerA3Capture_Init02------old robot version------
// Initialize Timer A3 in edge time mode to request interrupts on
// the rising edges of P10.4 (TA3CCP0) and P8.2 (TA3CCP2).  The
// interrupt service routines acknowledge the interrupt and call
// a user function.
// Input: task0 is a pointer to a user function called when P10.4 (TA3CCP0) edge occurs
//              parameter is 16-bit up-counting timer value when P10.4 (TA3CCP0) edge occurred (units of 0.083 usec)
//        task2 is a pointer to a user function called when P8.2 (TA3CCP2) edge occurs
//              parameter is 16-bit up-counting timer value when P8.2 (TA3CCP2) edge occurred (units of 0.083 usec)
// Output: none
// Assumes: low-speed subsystem master clock is 12 MHz
void TimerA3Capture_Init02(void(*task0)(uint16_t time), void(*task2)(uint16_t time)){
// old robot code
}
