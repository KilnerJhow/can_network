#include <TimerOne.h>

#define TQ_SIZE 6.25 // Tamanho de cada Tq, em ms
#define QT_TQ   16   // Quantidade de Tqs por bit
// #define COUNT_TQ 50000 //Tamanho do contador para cada Tq levando em conta os 16 MHz do Arduino.
#define BIT_SIZE 100 // Tamanho total do bit, em ms

#define SYNC 0
#define SEG_1 1
#define SEG_2 2
#define WINDOW2 3

#define SJW 2

const int time_segment1 = 7;
const int time_segment2 = 7;

volatile boolean aux = false;

volatile int cnt_sync   = 0;
volatile int cnt_seg_1  = 1;
volatile int resync1    = 0;
volatile int cnt_seg_2  = 1;
volatile int resync2    = 0;

volatile byte window2 = 0;
volatile byte actual_state = SYNC;
volatile byte hard_sync = 0;
volatile byte soft_sync = 0;
volatile byte reset = 0;

volatile byte writing_point = 0;
volatile byte sampling_point = 0;

String serialRead = "";

void setup(){
    Timer1.attachInterrupt(tq_ISR);
    Timer1.initialize(6250);
    Serial.begin(115200);
}

void loop() {
  noInterrupts();
  machine_state();
  interrupts();
  
  for (int i = 0; i < 5; i++) {

    Serial.print(actual_state + 10);
    Serial.print(" ");
    
//    Serial.print(aux + 8);
//    Serial.print(" ");

    Serial.print(cnt_seg_1);
//    Serial.print(writing_point + 7);
    Serial.print(" ");
   
//    Serial.print(sampling_point + 9);
//    Serial.print(" ");

    Serial.print(soft_sync);
    Serial.print(" ");
    
    Serial.print("\n");
  }
  noInterrupts();
  resync();
  interrupts();
}

void resync() {

  if(soft_sync == 1) {
     switch(actual_state) {
        case SEG_1:
            resync1 = cnt_seg_1;
            if(resync1 > SJW) resync1 = SJW;
            soft_sync = 0;
            break;

        case SEG_2:
            resync2 = time_segment2 - cnt_seg_2;
            if(resync2 > SJW) resync2 = SJW;
            actual_state = WINDOW2;
            soft_sync = 0;
            break;
     }
  }
  
}


void resetStates() {
    cnt_sync = 0;
    cnt_seg_1  = 1;
    resync1    = 0;
    cnt_seg_2  = 1;
    resync2    = 0;
    writing_point = 0;
}

void machine_state() {
  switch (actual_state) {

        case SYNC:
            if(cnt_sync == 1) {
              actual_state = SEG_1;
              writing_point = 1;
            }
            break;

        case SEG_1:
            //if(cnt_seg_1 == 3) soft_sync = 1;
            if(cnt_seg_1 <= (time_segment1  + resync1)) {
              writing_point = 0;
            }
            else {
              actual_state = SEG_2;
              sampling_point = 1;
            }
            break;

        case SEG_2:
            if(cnt_seg_2 == 2) soft_sync = 1;
            if(cnt_seg_2 <= time_segment2 ) {
              sampling_point = 0;
            }
            else {
                actual_state = SYNC;
                resetStates();
            }
            break;

        case WINDOW2:
            if(cnt_seg_2 <= (time_segment2 - resync2));
            else {
                actual_state = SEG_1;
                resetStates();
            }
            break;
    }
}

void clock_() {
     aux = !aux;
}

void tq_ISR() {
    switch (actual_state) {

        case SYNC:
            cnt_sync++;
            break;

        case SEG_1:
            //if(cnt_seg_1 == 2) soft_sync = 1;
            cnt_seg_1 ++;
            break;

        case SEG_2:
            cnt_seg_2++;
            break;

        case WINDOW2:
            cnt_seg_2++;
            break;
    
  }
}