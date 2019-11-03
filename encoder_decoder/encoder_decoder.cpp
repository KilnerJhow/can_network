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
#define CRC_BASE 7
#define END_OF_FRAME 8
#define ERROR 9

volatile int decoder_state = 0;
volatile int count = 0;
volatile int ID = 0;
volatile int IDE = 0;
volatile int RTR = 0;
volatile int DLC = 0;
volatile int bit_anterior = 0;
volatile int bit_atual = 0;
volatile int cnt_bit_stuffing = 0;
volatile int flag_bit_stuff = 0;
volatile int cnt_bit_igual = 0;

volatile int64_t data_can = 0;

volatile int aux = 1;

int crc_index = 14;

int first = 1;

bitset <250> buf;
bitset <15> crc;

void decode_message();
void decoder_ms();
void check_bit_stuffing();

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

    // cout << line << endl;

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
    // cout << dec << "bit stuffing count: " << cnt_bit_stuffing << endl;
    // cout << decoder_state << " ";
    // cout << dec << "Count: " << count << " buf: " << buf[count] << endl;
    bit_atual = buf[count];
    // cout << "Count: " << count << " state "<< decoder_state << endl;
    decoder_ms();
    check_bit_stuffing();
}

void decoder_ms() {
    
    switch(decoder_state) {

        case WAIT:
            if(bit_atual == 0 && count == 0) {
                //hard sync
                decoder_state = ARBITRATION;
            } 
            break;

        case ARBITRATION:

            if(count < (12 + cnt_bit_stuffing)) {
                if(!flag_bit_stuff) ID = ID << 1 | (bit_atual & 1);
            }

            if(count == (13 + cnt_bit_stuffing)) { 
                if(!flag_bit_stuff) RTR = bit_atual;
            }

            if(count == (14 + cnt_bit_stuffing)) {
                
                cout << "ID_A: 0x" << hex << ID;
                cout << " RTR: " << RTR;
                IDE = bit_atual;
                cout << " IDE: " << IDE << endl << endl;
                // exit(1);
                if(IDE == 1) {
                    decoder_state = CTRL_EXTENDED_F;
                }
                else {
                    if(RTR == 1) decoder_state = REMOTE_FRAME_BASE;
                    else decoder_state = DATA_FRAME_BASE;
                }
            } 
            break;

        case REMOTE_FRAME_BASE:
            if(count == (19 + cnt_bit_stuffing)) { //21 do remote + 12 iniciais
                DLC = 0;
                decoder_state = CRC_BASE;
            }
            break;
        
        case DATA_FRAME_BASE:
            if(count <= (18 + cnt_bit_stuffing)) {
                // cout << "counted";
                if(!flag_bit_stuff) DLC = DLC << 1 | (bit_atual & 1);
            }

            if(count <= (18 + DLC*8 + cnt_bit_stuffing)) {
                if(!flag_bit_stuff) data_can = data_can << 1 | (bit_atual & 1);
            }

            
            if(count == (18 + DLC * 8 + cnt_bit_stuffing)) {
                cout << "DLC: " << DLC;
                cout << " data: " << hex << uppercase << data_can << endl << endl;   
                decoder_state = CRC_BASE;            
            }
            break;
        
        case CTRL_EXTENDED_F:
            if(count == 29) { // 17 do ID_B e 12 iniciais
                if(RTR == 1) decoder_state = REMOTE_FRAME_EXT;
                else decoder_state = DATA_FRAME_EXT;
            }
            break;
        
        case REMOTE_FRAME_EXT:
            if(count == 21) {
                
                decoder_state = CRC_BASE;
            }
            break;
        
        case DATA_FRAME_EXT:
            if(count == (22 + DLC)) {
                
                decoder_state = CRC_BASE;
            }
            break;

        case CRC_BASE:

            // cout << 19 + DLC*8 + cnt_bit_stuffing;
            // cout << "antes CRC: " << crc << endl;
            if(count <= (33 + DLC*8 + cnt_bit_stuffing)) {
                
                if(!flag_bit_stuff){
                    crc[crc_index] = bit_atual & 1;
                    crc_index--;    
                } //else {
                //     cout << "Bit atual eh stuff" << endl;
                // }
                // cout << "depois CRC: " << crc << endl;
                // cout << "Index crc: " << crc_index << endl;
            }
            if(count == (33 + DLC*8 + cnt_bit_stuffing)) {
                cout << "CRC: " << crc << endl;
                decoder_state = END_OF_FRAME;
            }
            break;
        
        case END_OF_FRAME:
            if(count == (43 + DLC*8 + cnt_bit_stuffing)) {
                cout << "here" << endl;
                decoder_state = WAIT;
                aux = -1;
            }
            break;
        
        case ERROR:
            // escreve 6 bits 0 e depois 8 bits 1, não se importa com a leitura
            aux = -1;
            break;
            
    }
    
    count ++;
}
void check_bit_stuffing() {

    // cout << "cnt bit igual: " << cnt_bit_igual << endl;

    if(decoder_state != ERROR && decoder_state != END_OF_FRAME) {

        if(bit_anterior == bit_atual){

            cnt_bit_igual++; 

            // cout << bit_anterior;
            // cout << "Bit anterior: " << bit_anterior;
            // cout << " Bit atual: " << bit_atual
        } else {
            cnt_bit_igual = 0;
            flag_bit_stuff = 0;
        }
    }

    if(cnt_bit_igual == 4) { //5 bits iguais vieram, o próximo é bit stuffing
        cnt_bit_stuffing++;
        flag_bit_stuff = 1;
        // cout << "Bit stuff, state: " << decoder_state << endl;
    }        

    if(cnt_bit_igual == 5) {
        decoder_state = ERROR;
        cout << "Erro!" << endl;
    }

    

    bit_anterior = bit_atual;
}

