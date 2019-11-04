#include <iostream>
#include <fstream>
#include <bitset>

using namespace std;

#define WAIT 0
#define ARBITRATION 1
#define CTRL_EXTENDED_F 2
#define DATA_FRAME 3
#define REMOTE_FRAME 4
// #define DATA_FRAME_EXT 5
// #define REMOTE_FRAME_EXT 6
#define CRC 7
#define END_OF_FRAME 8
#define ERROR 9
#define CTRL_BASE_F 10
#define CTRL_F 11
// #define CRC_EXT 12
#define ACK 13
// #define ACK_EXT 14
// #define END_OF_FRAME_EXT 15
#define INTERFRAME_SPACE 16

volatile int decoder_state = WAIT;


volatile int count = 0;
volatile int count_arbitration = 0;
volatile int count_ctrl_f = 0;
volatile int count_ctrl_base_f = 0;
volatile int count_ctrl_base_ext = 0;
volatile int count_data = 0;
volatile int count_remote = 0;
// volatile int count_data_ext = 0;
// volatile int count_remote_ext = 0;
volatile int count_crc = 0;
// volatile int count_crc_ext = 0;
volatile int count_ack = 0;
// volatile int count_ack_ext = 0;
volatile int count_eof = 0;
// volatile int count_eof_ext = 0;
volatile int count_ifs = 0;


volatile int ID_A = 0;
volatile int ID_B = 0;
volatile int IDE = 0;
volatile int RTR = 0;
volatile int SRR = 0;
volatile int bit_12 = 0;
volatile int DLC = 0;
volatile int bit_anterior = 0;
volatile int bit_atual = 0;
volatile int cnt_bit_stuffing = 0;
volatile int flag_bit_stuff = 0;
volatile int cnt_bit_igual = 0;

uint16_t crc_int = 0;

volatile int err_permission = 0;

volatile int add_to_frame = 0; //usado pra multiplicar dlc*8
volatile int add_ext = 0; //usado pra multiplicar dlc*8 para frame ext

volatile int64_t data_msg = 0;

volatile int aux = 1;

int crc_index = 14;

int first = 1;

bitset <250> buf;
bitset <110> frame;
volatile int frame_count = 0;
// bitset <15> crc;

//For CRC use
const uint16_t crc_polinomial = 0x4599;
bitset <15> crc_seq;
bitset <15> crc_check;
uint16_t crc_convert;
int crc_next = 0;


void calculate_crc(int);
void check_crc(int);
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
  
    bit_atual = buf[count];
    if(!flag_bit_stuff){
        decoder_ms();
    }
    check_bit_stuffing();
    count++;
}

void decoder_ms() {
    frame = frame << 1; 
    frame[0] = bit_atual & 1;
    frame_count++;
    // cout << frame << endl;
    switch(decoder_state) {

        case WAIT:
            if(bit_atual == 0 && count == 0) {
                //hard sync
                decoder_state = ARBITRATION;
                err_permission = 1;
            } 
            break;

        case ARBITRATION:
            // cout << "Arbitration" << endl;
            if(count_arbitration <= 10) {
                ID_A = ID_A << 1 | (bit_atual & 1);
            }

            if(count_arbitration == 10) {
                decoder_state = CTRL_F;
                cout << "ID_A: 0x" << hex << ID_A << endl;
                // cout << "COUNT: " << dec << count << endl;
            }
            count_arbitration++;
            break;

        case CTRL_F:

            if(count_ctrl_f == 0) bit_12 = bit_atual;
            if(count_ctrl_f == 1) {
                IDE = bit_atual;
                cout << "IDE: " << IDE << endl;
                if(IDE == 1) decoder_state = CTRL_EXTENDED_F;
                else decoder_state = CTRL_BASE_F;
            }
            count_ctrl_f++;
            break;

        case CTRL_BASE_F: 
 
            if(count_ctrl_base_f == 0) {
                //Lê bit r0
                RTR = bit_12;
                cout << "RTR: " << RTR << endl;
                cout << "Bit reservado: " << bit_atual << endl;
                if(RTR == 0) decoder_state = DATA_FRAME;
                else decoder_state = REMOTE_FRAME;
            }
            count_ctrl_base_f++;
            break;

        case REMOTE_FRAME:
            
            if(count_remote <= 3 ) {
                // cout << "dlc lido no bit: " << dec << count << endl;
                DLC = DLC << 1 | (bit_atual & 1);
            }

            if(count_remote == 3) { //21 do remote + 12 iniciais
                cout << "DLC: " << DLC;
                cout << " data: vazio" << endl;
                // cout << " RTR: " << RTR;
                decoder_state = CRC;
                add_to_frame = 0;
            }
            count_remote++;
            break;
        
        case DATA_FRAME:
            if(count_data <= 3 ) {
                // cout << "dlc lido no bit: " << dec << count << endl;
                DLC = DLC << 1 | (bit_atual & 1);

            }

            if( (count_data > 3 ) && (count_data <= (3 + DLC*8 ))) {
                
                // cout << "data lido no bit: " << dec << count << endl;
                data_msg = data_msg << 1 | (bit_atual & 1);
            }

            
            if(count_data == (3 + DLC * 8)) {
                // cout << 
                // bitset <64> dlc (data_msg);
                // cout << "Saiu no bit: " << dec <<count << endl;
                cout << "DLC: " << DLC;
                cout << " data: " << hex << uppercase << data_msg << endl;
                add_to_frame = DLC*8;   
                // cout << dlc << endl;
                // exit(1);
                decoder_state = CRC;
                for(int i = frame_count; i >= 0; i--) {
                    calculate_crc(i);
                }            
                cout << "CRC calculado: " << crc_seq << endl;
            }
            count_data++;
            break;
        
        case CTRL_EXTENDED_F:
            
            if(count_ctrl_base_ext <= 17) {    
                // cout << "Bit lido: " << bit_atual << endl;
                ID_B = ID_B << 1 | (bit_atual & 1);                
            }
            
            if(count_ctrl_base_ext == 18){
                RTR = bit_atual;
            }

            if(count_ctrl_base_ext == 20) { //2 bits reservados
                // bitset <18> idb(ID_B);
                cout << " RTR: " << RTR << endl;
                cout << "ID_B: 0x" << hex << ID_B << endl;
                cout << dec;
                // cout << "ID_B: " << hex <<ID_B << endl;
                // exit(1);
                if(RTR == 1) decoder_state = REMOTE_FRAME;
                else decoder_state = DATA_FRAME;
            }
            count_ctrl_base_ext++;
            break;

        case CRC:
            // cout << "Count: " << dec <<  count << endl;
            if(count_crc <= 14) {
                crc_int = crc_int << 1 | (bit_atual & 1);
                // bitset <15> crc(crc_int);
                // cout << crc << endl;                
            }

            if(count_crc == 14) {

                for(int i = frame_count; i >= 0; i--) { //-1 devido ao crc delimiter
                    check_crc(i);
                }
                bitset <15> crc(crc_int);
                cout << "CRC lido: " << crc << endl;
                // cout << "CRC check: " << crc_check << endl;
                // cout << "Frame: " << frame << endl;
                if(crc_check.any()) {
                    cout << "CRC erro!" << endl;
                    exit(1);
                } else {
                    cout << "CRC ok!" << endl;
                }
            }

            // cout << "depois CRC: " << crc << endl;
            if(count_crc == 15) { //conta tbm o crc delimiter
                
                decoder_state = ACK;
                err_permission = 0;
            }
            count_crc++;
            

            break;

        case ACK:
            if(count_ack == 0) {
                if(bit_atual == 0)
                    cout << "ACK OK" << endl;
                else 
                    cout << "ACK ERROR" << endl;
            }

            if(count_ack == 1) {
                decoder_state = END_OF_FRAME;
            }

            count_ack++;
            break;

        case END_OF_FRAME:
            if(count_eof == 7) {
                decoder_state = WAIT;
                cout << "Ate EOF" << endl;
                aux = -1;
            }
            count_eof++;
            break;
        
        case INTERFRAME_SPACE:

            if(count_ifs == 3) {
                cout << "Interframe space" << endl;
                aux = -1;
            }
            count_ifs++;

            break;

        
        case ERROR:
            // escreve 6 bits 0 e depois 8 bits 1, não se importa com a leitura
            aux = -1;
            break;
            
    }
}

void calculate_crc(int i) {
    crc_next = frame[i] ^ crc_seq[14];
    crc_seq = crc_seq << 1;//Shift left de 1
    crc_seq[0] = 0;
    crc_convert = (uint16_t) crc_seq.to_ulong();
    if(crc_next){
        crc_seq = crc_convert ^ crc_polinomial;
    }
}

void check_crc(int i) {
    crc_next = frame[i] ^ crc_check[14];
    crc_check = crc_check << 1;//Shift left de 1
    crc_check[0] = 0;
    crc_convert = (uint16_t) crc_check.to_ulong();
    if(crc_next){
        crc_check = crc_convert ^ crc_polinomial;
    }
}

void check_bit_stuffing() {

    // cout << "cnt bit igual: " << cnt_bit_igual << endl;

    if(err_permission) {


        if(bit_anterior == bit_atual){

            cnt_bit_igual++; 

        } else {

            cnt_bit_igual = 0;
            flag_bit_stuff = 0;

        }
        // cout << "Bit atual: " << bit_atual<< endl;
        // cout << "Bit anterior: " << bit_anterior<< endl;
        // cout << "Contador de bit igual: " << cnt_bit_igual << endl;
    }

    if(cnt_bit_igual == 4) { //5 bits iguais vieram, o próximo é bit stuffing
        cnt_bit_stuffing++;
        flag_bit_stuff = 1;
    }        

    if(cnt_bit_igual == 5) {
        decoder_state = ERROR;
        cout << "Erro!" << endl;
    }

    bit_anterior = bit_atual;
}