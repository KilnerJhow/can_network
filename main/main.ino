#include <TimerOne.h>
#include "bit_timing.h"
#include "decoder.h"

bit_timing bt;
decoder dec;

void setup(){
    
    pinMode(2, INPUT_PULLUP);//hard_sync
    pinMode(3, INPUT_PULLUP);//soft_sync
    Timer1.attachInterrupt(tq_ISR);
    Timer1.initialize(500000);
    Serial.begin(115200);
}


void loop() {

    // if(print == 1) {
    //     for (int i = 0; i < 5; i++) {

    //         Serial.print(actual_state);
    //         Serial.print(" ");

    //         Serial.print(aux + 3);
    //         // Serial.print(A + 3);
    //         Serial.print(" ");
            
    //         Serial.print(hard_sync + 5);
    //         Serial.print(" ");

    //         Serial.print(soft_sync + 7);
    //         Serial.print(" ");

    //         // Serial.print(valor_A + 5);
    //         // Serial.print(" ");

    //         Serial.print(sampling_point + 9);
    //         Serial.print(" ");

    //         // Serial.print(cnt_seg_1 + 11);
    //         Serial.print(writing_point + 11);
    //         Serial.print(" ");


    //         Serial.print("\n");
    //     }
    //     print = 0;
    // }

}

void readButton() {
    bt.soft_sync = !digitalRead(3);
    bt.hard_sync = !digitalRead(2);
}

void tq_ISR() {
    // print = 1;  

    // aux = !aux;
    //ocheckEdge();    //checa se houve mudanca entre 2 tq
    
    bt.machine_state();
    
    bt.sample(1);
    
    readButton();

    bt.resync();
}
