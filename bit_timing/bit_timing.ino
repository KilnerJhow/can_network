#include <TimerOne.h>

//Ajeitar o window2 - Feito
//TO-DO: Fazer a função de geração "aleatório" dos erros
//Ler a variável A e ver se houve uma borda para ativar o soft sync
//Ter uma variável A que muda de acordo com uma certa quantidade de Tqs
//Por exemplo, A dura 16Tqs em um bit, depois 18 tqs e no 3 bit ocorre um 
//sync atrasado, forçando um soft sync


#define TQ_SIZE 6.25 // Tamanho de cada Tq, em ms
#define QT_TQ   16   // Quantidade de Tqs por bit
// #define COUNT_TQ 50000 //Tamanho do contador para cada Tq levando em conta os 16 MHz do Arduino.
#define BIT_SIZE 100 // Tamanho total do bit, em ms

#define SYNC 0
#define SEG_1 1
#define SEG_2 2

#define SJW 2

const int inputpin = 13;

const int time_segment1 = 8;
const int time_segment2 = 7;

volatile boolean aux = false;

unsigned int edge = 0;
unsigned char actual;
unsigned char past;

volatile int cnt_sync   = 0;
volatile int print = 0;
volatile int cnt_seg_1  = 1;
volatile int resync1    = 0;
volatile int cnt_seg_2  = 1;
volatile int resync2    = 0;
volatile int actual_state = SYNC;
volatile byte hard_sync = 0;
volatile byte soft_sync = 0;
volatile byte reset = 0;

volatile boolean occurr_soft_sync = false;
volatile boolean window2 = false;
volatile int writing_point = 0;
volatile int sampling_point = 0;

//variáveis para testes
#define state1 0
#define state2 1
#define state3 3

volatile int state_bit = 0;
volatile int A = 0;
volatile int valor_A = 0;
const int time_state_1 = 13;
const int time_state_2 = 13;
const int time_state_3 = 15;
volatile int cnt_state_1 = 1;
volatile int cnt_state_2 = 1;
volatile int cnt_state_3 = 1;
//Fim das variáveis para testes

void setup(){
    pinMode(inputpin, INPUT);
    Timer1.attachInterrupt(tq_ISR);
    Timer1.initialize(6250);
    Serial.begin(115200);
}

void loop() {
    noInterrupts();
    
    machine_state();
    
    sample();

    generateError();

    for (int i = 0; i < 5; i++) {

        Serial.print(actual_state);
        Serial.print(" ");

        Serial.print(A + 3);
        Serial.print(" ");
        
        Serial.print(edge + 5);
        Serial.print(" ");

        // Serial.print(valor_A + 5);
        // Serial.print(" ");

        Serial.print(sampling_point + 7);
        Serial.print(" ");


        // Serial.print(cnt_seg_1 + 11);
        //    Serial.print(writing_point + 7);
        // Serial.print(" ");

        Serial.print(soft_sync + 9);
        Serial.print(" ");

        Serial.print("\n");
    }

    resync();

    interrupts();

    

}

void sample() {

    if(sampling_point == 1) {
        valor_A = A;
    }

}

void resync() {
  
  if(soft_sync == 1) {
     switch(actual_state) {
        case SEG_1:
            resync1 = cnt_seg_1;
            // if(resync1 > SJW) resync1 = SJW;
            //occurr_soft_sync = true;
            soft_sync = 0;
            break;

        case SEG_2:
            resync2 = time_segment2 - cnt_seg_2;
            // if(resync2 > SJW) resync2 = SJW;
            window2 = true;
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
    sampling_point = 0;
    window2 = false;
    occurr_soft_sync = false;
}

void machine_state() {

  checksync(); //verifica a necessidade de sincronização
         
  switch (actual_state) {
        case SYNC:
            if(cnt_sync == 1) {
              actual_state = SEG_1;
              writing_point = 1;
              sampling_point = 0;
            }
            break;

        case SEG_1:
            if(cnt_seg_1 < (time_segment1  + resync1)) {
                writing_point = 0;
                sampling_point = 0;
            } else {
                actual_state = SEG_2;
                sampling_point = 1;
                writing_point = 0;
            }
            break;

        case SEG_2:
            if(!window2) {
                if(cnt_seg_2 < time_segment2 ) {
                    sampling_point = 0;
                    writing_point = 0;
                }
                else {
                    actual_state = SYNC;
                    resetStates();
                }
            } else {
                if(cnt_seg_2 < (time_segment2 - resync2)){
                    sampling_point = 0;
                    writing_point = 0;
                }
                else {
                    actual_state = SEG_1;
                    resetStates();
                }
            }
            break;

    }
}

void tq_ISR() {
    print = 1;
    aux = !aux;
    checkEdge();    //checa se houve mudanca entre 2 tq
    switch (actual_state) {

        case SYNC:
            cnt_sync++;
            break;

        case SEG_1:
            cnt_seg_1 ++;
            break;

        case SEG_2:
            cnt_seg_2++;
            break;
    
    }
 
    switch (state_bit) {
        case state1: 
            cnt_state_1++;
            break;

        case state2: 
            cnt_state_2++;
            break;

        case state3: 
            cnt_state_3++;
            break;
    }

}

void checkEdge(){
    // actual = digitalRead(inputpin);
    actual = A;

    if(past != actual){
        edge = 1;
    } else {
        edge = 0;
    }
    // past = actual;
    past = A;
}
  
void checksync() {
    if(edge == 1 && actual_state != SYNC){
        edge = 0;
        soft_sync = 1;
        //resync();
    }
}

void generateError() {

    switch(state_bit) {
        case state1:
            
            if(cnt_state_1 > time_state_1) {
                state_bit = state2;
                A = 1;
            }
            break;
            
        case state2:
            
            if(cnt_state_2 > time_state_2) {
                state_bit = state3;
                A = 0;
            }
            break;
            

        case state3:
            
            if(cnt_state_3 > time_state_3) {
                state_bit = state1;
                cnt_state_1 = 1;
                cnt_state_2 = 1;
                cnt_state_3 = 1;
                A = 1;
            }
            break;
    }

}