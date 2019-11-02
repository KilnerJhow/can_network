#include <iostream>
#include <fstream>
#include <bitset>

using namespace std;

#define WAIT 0
#define ARBITRATION 1
#define CTRL_BASE_F 2
#define CTRL_EXTENDED_F 3
#define DATA_FRAME_BASE 4
#define REMOTE_FRAME_BASE 5
#define DATA_FRAME_EXT 6
#define REMOTE_FRAME_EXT 7
#define ACK_SEND 8
#define INTERFRAME_SPACE 9
#define ERROR 10

volatile int decoder_state = 0;
volatile int count = 0;
volatile int IDE = 0;
volatile int RTR = 0;
volatile int DLC = 0;
volatile int bit_anterior;
volatile int bit_atual;
volatile int cnt_bit_stuffing = 0;

bitset <200> buf;

int main() {
    string line;
    ifstream inFile;
    
    inFile.open("in.txt");

    if(!inFile) {
        cout << "Problema na abertura do arquivo!";
        exit(1);
    }

    while(getline(inFile, line)){
        cout << line << endl;
        for(int i = 0; i < line.size(); i++){
            buf[i] = (int)line.at(i) - '0';
        }
        decode_message();
    }

    inFile.close();
}

void decode_message() {
    
}

void decoder_ms() {

    check_error();
    switch(decoder_state) {
        case WAIT:
            if(count == 12) {
                count = 0;
                if(IDE == 1) decoder_state = CTRL_EXTENDED_F;
                else {
                    if(RTR == 1) decoder_state = REMOTE_FRAME_BASE;
                    else decoder_state = DATA_FRAME_BASE;
                }
            } else count++;
            break;

        case REMOTE_FRAME_BASE:
            if(count == 21) {
                count = 0;
                decoder_state = ACK_SEND;
            } else count++;
            break;
        
        case DATA_FRAME_BASE:
            if(count == (20 + DLC)) {
                count = 0;
                decoder_state = ACK_SEND;
            } else count++;
            break;
        
        case CTRL_EXTENDED_F:
            if(count == 17) {
                count = 0;
                if(RTR == 1) decoder_state = REMOTE_FRAME_EXT;
                else decoder_state = DATA_FRAME_EXT;
            } else count ++;
            break;
        
        case REMOTE_FRAME_EXT:
            if(count == 21) {
                count = 0;
                decoder_state = ACK_SEND;
            } else count++;
            break;
        
        case DATA_FRAME_EXT:
            if(count == (22 + DLC)) {
                count = 0;
                decoder_state = ACK_SEND;
            } else count++;
            break;

        case ACK_SEND:
            if(count == 1) {
                count = 0;
                decoder_state = INTERFRAME_SPACE;
            } else count++;
            break;
        
        case INTERFRAME_SPACE:
            if(count == 2) {
                count = 0;
                decoder_state = WAIT;
            } else count++;
            break;
        
        case ERROR:
            // escreve 6 bits 0 e depois 8 bits 1, não se importa com a leitura
            break;
            
    }
}

void check_error() {

    if(bit_anterior == bit_atual){
        cnt_bit_stuffing++;
    }

    if(cnt_bit_stuffing == 6) {
        decoder_state = ERROR;
    }

}