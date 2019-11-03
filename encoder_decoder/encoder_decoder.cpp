#include <iostream>
#include <fstream>
#include <bitset>

using namespace std;

#define WAIT 0
#define ARBITRATION 1
#define CTRL_EXTENDED_F 2
#define DATA_FRAME_BASE 3
#define REMOTE_FRAME_BASE 4
#define DATA_FRAME_EXT 5
#define REMOTE_FRAME_EXT 6
#define ACK_SEND 7
#define INTERFRAME_SPACE 8
#define ERROR 9

volatile int decoder_state = 0;
volatile int count = 0;
volatile int IDE = 0;
volatile int RTR = 0;
volatile int DLC = 0;
volatile int bit_anterior = 0;
volatile int bit_atual = 0;
volatile int cnt_bit_stuffing = 0;

volatile int aux = 1;

bitset <250> buf;

void getID();
void decode_message();
void decoder_ms();
void check_error();

int main() {
    string line;
    ifstream inFile;
    
    inFile.open("in.txt");

    if(!inFile) {
        cout << "Problema na abertura do arquivo!";
        exit(1);
    }

    getline(inFile, line);

    for(int i = 0; i < line.size(); i++){
        buf[i] = (int)line.at(i) - '0';
    }

    cout << line << endl;

    while(aux > 0) {
        decode_message();
    }

    // while(getline(inFile, line)){
    //     cout << line << endl;
    //     for(int i = 0; i < line.size(); i++){
    //         buf[i] = (int)line.at(i) - '0';
    //     }
    //     decode_message();
    // }

    inFile.close();
}

void decode_message() {
    bit_atual = buf[count];
    cout << "Count: " << count << " state "<< decoder_state << endl;
    decoder_ms();
}

void decoder_ms() {

    //check_error();
    switch(decoder_state) {

        case WAIT:
            if(bit_atual == 0 && count == 0) {
                //hard sync
                decoder_state = ARBITRATION;
            } 
            break;

        case ARBITRATION:

            if(count == 11) {
                RTR = bit_atual;
            }

            if(count == 12) {
                getID();
                IDE = bit_atual;
                if(IDE == 1) decoder_state = CTRL_EXTENDED_F;
                else {
                    if(RTR == 1) decoder_state = REMOTE_FRAME_BASE;
                    else decoder_state = DATA_FRAME_BASE;
                }
            } else count++;
            break;

        case REMOTE_FRAME_BASE:
            if(count == 33) { //21 do remote + 12 iniciais
                
                decoder_state = ACK_SEND;
            } else count++;
            break;
        
        case DATA_FRAME_BASE:
            if(count == (42 + DLC)) { //20 do base + 12 iniciais + DLC
                
                decoder_state = ACK_SEND;
            } else count++;
            break;
        
        case CTRL_EXTENDED_F:
            if(count == 29) { // 17 do ID_B e 12 iniciais
                if(RTR == 1) decoder_state = REMOTE_FRAME_EXT;
                else decoder_state = DATA_FRAME_EXT;
            } else count ++;
            break;
        
        case REMOTE_FRAME_EXT:
            if(count == 21) {
                
                decoder_state = ACK_SEND;
            } else count++;
            break;
        
        case DATA_FRAME_EXT:
            if(count == (22 + DLC)) {
                
                decoder_state = ACK_SEND;
            } else count++;
            break;

        case ACK_SEND:
            if(count == 43) {
                
                decoder_state = INTERFRAME_SPACE;
            } else count++;
            break;
        
        case INTERFRAME_SPACE:
            if(count == 46) {
                
                decoder_state = WAIT;
                aux = -1;
            } else count++;
            break;
        
        case ERROR:
            // escreve 6 bits 0 e depois 8 bits 1, não se importa com a leitura
            break;
            
    }
}

void getID() {
    
    int b = 0;
    for(int i = 1; i < 12; i++) {
        b = b << 1 | buf[i];
    }

    cout <<"0x" << hex << b << endl;
}

void check_error() {

    if(bit_anterior == bit_atual){
        cnt_bit_stuffing++;       
        if(cnt_bit_stuffing == 5) decoder_state = ERROR;

    } else cnt_bit_stuffing = 0;

    bit_anterior = bit_atual;
}

