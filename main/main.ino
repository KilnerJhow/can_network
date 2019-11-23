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

uint8_t sender = 1; //trasmissor = 1, receptor = 0

void setup(){

    pinMode(PIN_RX, INPUT);
    pinMode(PIN_TX, OUTPUT);

    digitalWrite(PIN_TX, bit_enviado);

    Serial.begin(115200);
    Serial.println("Inicializado");
    
    bt.initialize();
    enc.initialize();
    dec.initialize();


    enc.encoder_mws(0);
    enc.printFrameNoStuff();
    enc.printFrameStuff();

    Timer1.attachInterrupt(tq_ISR);
    Timer1.initialize(10000);

    if(sender) {
        enc.printDataToSend();
        Serial.println("Configurado no modo transmissor");
    } else {
        Serial.println("Configurado no modo receptor");
    }


}

void loop() {
    // if(!sender)
        dec.printData();
}

void writeBus() {
    digitalWrite(PIN_TX, bit_enviado);
}

uint8_t readBus(){
    return digitalRead(PIN_RX);
}

void tq_ISR() {

    bt.machine_state();
    //Se Writing Point = 1
    if(bt.writing_point()) {
        if(dec.getFlagACK() && !enc.canSendMsg()){

            Serial.println("Enviando ACK slot");
            bit_enviado = 0;
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
