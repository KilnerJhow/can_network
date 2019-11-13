#include <TimerOne.h>
#include "bit_timing.h"
#include "decoder.h"
#include "encoder.h"



uint8_t flag_err = 0;
uint8_t bit_enviado = 1;
uint8_t bit_atual;
 


// decoder dec(flag_err);
bit_timing bt(&Serial);
decoder dec(flag_err);
encoder enc(flag_err);

void setup(){
    Serial.begin(115200);

    Timer1.attachInterrupt(tq_ISR);
    Timer1.initialize(50000);


}


void loop() {

}

void tq_ISR() {

    //Se Writing Point = 1
    if(bt.writing_point()) {
        if(enc.canSendMsg()) {
            bit_enviado = enc.enviaBit();
            // writeBus();
        }
    }

    if(bt.sampling_point()) {
        // bit_atual = readBus();
        dec.decode_message(bit_atual, bit_enviado);
    }

    enc.setSendFlag(dec.getSendFlag());

    bt.setHS(dec.getHS());
    //Chamar funções do bit timing

    //Se o sample point = 1, chama o decoder

    //Caso haja hard_sync, chama o hard_sync do bit timing e seta o hard_sync para o próximo bit.
}
