#include <TimerOne.h>

#define TQ_SIZE 6.25 // Tamanho de cada Tq, em ms
#define QT_TQ   16   // Quantidade de Tqs por bit
// #define COUNT_TQ 50000 //Tamanho do contador para cada Tq levando em conta os 16 MHz do Arduino.
#define BIT_SIZE 100 // Tamanho total do bit, em ms

#define SYNC 0
#define SEG_1 1
#define SEG_2 2
#define WINDOW2 3

const int time_segment1 = 7;
const int time_segment2 = 7;

volatile int cnt_seg_1  = 1;
volatile int resync1    = 0;
volatile int cnt_seg_2  = 1;
volatile int resync2    = 0;
volatile int current_counter    = 0;

volatile byte window2 = 0;
volatile byte actual_state = 0;
volatile byte hard_sync = 0;
volatile byte reset = 0;

void setup(){
    Timer1.Initialize(6250);
    Timer1.attachInterrupt(machine_state_ISR);
}

void loop() {
    current_counter = millis();
}

void machine_state_ISR() {
    switch (actual_state) {

        case SYNC:
            actual_state = SEG_1;
            break;

        case SEG_1:
            if(cnt_seg_1 < (time_segment1  + resync1)) cnt_seg_1 ++;
            else actual_state = SEG_2;
            break;

        case SEG_2:
            if(cnt_seg_2 < time_segment2 ) cnt_seg_2++;
            else actual_state = SYNC;
            break;

        case WINDOW2:
            if(cnt_seg_2 < (time_segment2 - resync2)) cnt_seg_2++;
            else actual_state = SEG_1;
            break;

        default:
            break;
    }
}