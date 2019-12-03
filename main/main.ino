#include <TimerOne.h>
#include "bit_timing.h"
#include "decoder.h"
#include "encoder.h"

#define PIN_RX 2
#define PIN_TX 3

uint8_t bit_enviado = 1;
uint8_t bit_atual = 1;

// decoder dec(flag_err);
bit_timing bt(&Serial);
decoder dec(&Serial);
encoder enc(&Serial);
int change = 0;

uint8_t print_bit_enviado = 0; //trasmissor = 1, receptor = 0
uint8_t flag_print_sendack = 0; //trasmissor = 1, receptor = 0

void setup(){

    pinMode(PIN_RX, INPUT);
    pinMode(PIN_TX, OUTPUT);
    pinMode(13, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(PIN_RX), bt_edge, CHANGE);

    digitalWrite(PIN_TX, bit_enviado);

    Serial.begin(115200);
    Serial.println("Inicializado");
    
    bt.initialize();
    enc.initialize();
    dec.initialize();


    enc.encoder_mws(0);
    enc.printDataToSend();
    // enc.printFrameNoStuff();
    // enc.printFrameStuff();

    Timer1.attachInterrupt(tq_ISR);
    Timer1.initialize(10000);

}

void loop() {
    // if(!sender)
    dec.printData();
    dec.printFlags();
    bt.printFlag();
    if(flag_print_sendack){
        flag_print_sendack =0;
        Serial.println("Enviando ack");
    }
    // if(print_bit_enviado) {
    //     Serial.print("Enviado: ");
    //     Serial.println(bit_enviado);
    //     print_bit_enviado = 0;
    // }
}

void writeBus() {
    digitalWrite(PIN_TX, bit_enviado);
}

uint8_t readBus(){
    return digitalRead(PIN_RX);
}

void tq_ISR() {

    bt.machine_state(readBus());
    //Se Writing Point = 1
    if(bt.writing_point()) {
        print_bit_enviado = 1;
        if(dec.getFlagACK() && !enc.canSendMsg()){

            // Serial.println("Enviando ACK slot");
            bit_enviado = 0;
            flag_print_sendack = 1;
            writeBus();

        } else if(enc.canSendMsg()) {
            // Serial.println("Enviando bit");
            bit_enviado = enc.enviaBit();
            writeBus();
            // Serial.print("E: ");
            // Serial.print(bit_enviado);

        }  else {
            
            bit_enviado = 1;
            writeBus();

        }
    }

    if(bt.sampling_point()) {
        bit_atual = readBus();

        // Serial.print(" R: ");
        // Serial.println(bit_atual);

        dec.decode_message(bit_atual, bit_enviado);
        bt.setHS(dec.getHS());
        
        enc.setSendFlag(dec.getSendFlag());

        enc.setResetFlag(dec.getResetFlag());

        enc.setErrorFlag(dec.getErrorflag());
        
        enc.setMountFrame(dec.getMountFrame());

    }


}

void bt_edge(){
    change = !change;
    bt.setEdge();
    digitalWrite(13, change);
}