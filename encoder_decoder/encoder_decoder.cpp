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
#define END_OF_FRAME_BASE 8
#define ERROR 9
#define CTRL_BASE_F 10
#define CTRL_F 11
#define CRC_EXT 12
#define ACK_BASE 13
#define ACK_EXT 14
#define END_OF_FRAME_EXT 15

volatile int decoder_state = WAIT;
volatile int count = 0;
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

volatile int crc_int = 0;

volatile int err_permission = 0;

volatile int add_base = 0; //usado pra multiplicar dlc*8
volatile int add_ext = 0; //usado pra multiplicar dlc*8 para frame ext

volatile int64_t data_msg = 0;

volatile int aux = 1;

int crc_index = 14;

int first = 1;

bitset <250> buf;
// bitset <15> crc;

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
    // cout << decoder_state << " ";
    // cout << dec << "Count: " << count << " buf: " << buf[count] << endl;
    // cout << dec << " bit stuffing count: " << cnt_bit_stuffing << endl;
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
                err_permission = 1;
            } 
            break;

        case ARBITRATION:

            if(count <= (11 + cnt_bit_stuffing) && !flag_bit_stuff) {
                ID_A = ID_A << 1 | (bit_atual & 1);
            }

            if(count == (11 + cnt_bit_stuffing)) {
                decoder_state = CTRL_F;
                cout << "ID_A: 0x" << hex << ID_A << endl;
                // cout << "COUNT: " << dec << count << endl;
            }

            // if(count == (13 + cnt_bit_stuffing) && !flag_bit_stuff) {
            //     bit_12 = bit_atual;
            // }

            // if(count == (14 + cnt_bit_stuffing) && !flag_bit_stuff) { 
                    
            //     cout << "ID_A: 0x" << hex << ID_A;
            //     // cout << " RTR: " << RTR;
            //     IDE = bit_atual;
            //     cout << " IDE: " << IDE;
            //     // exit(1);
            //     if(IDE == 1) {
            //         decoder_state = CTRL_EXTENDED_F;
            //         SRR = bit_12;
            //         cout << " SRR: " << SRR;
            //     }
            //     else {
            //         RTR = bit_12;
            //         cout << " RTR: " << RTR << endl;
            //         decoder_state = CTRL_BASE_F;
            //     }
                
            // } 
            break;

        case CTRL_F:

            if(count == (12 + cnt_bit_stuffing) && !flag_bit_stuff) bit_12 = bit_atual;
            if(count == (13 + cnt_bit_stuffing) && !flag_bit_stuff) {
                IDE = bit_atual;
                cout << "IDE: " << IDE;
                if(IDE == 1) decoder_state = CTRL_EXTENDED_F;
                else decoder_state = CTRL_BASE_F;
            }
            break;

        case CTRL_BASE_F: 
 
            if(count == (14 + cnt_bit_stuffing) && !flag_bit_stuff) {
                //Lê bit r0
                RTR = bit_12;
                cout << " RTR: " << RTR << endl;
                if(RTR == 0) decoder_state = DATA_FRAME_BASE;
                else decoder_state = REMOTE_FRAME_BASE;
            }
            break;

        case REMOTE_FRAME_BASE:
            
            if(count <= (18 + cnt_bit_stuffing) && !flag_bit_stuff) {
                // cout << "dlc lido no bit: " << dec << count << endl;
                DLC = DLC << 1 | (bit_atual & 1);

            }
            
            if(count == (18 + cnt_bit_stuffing)) { //21 do remote + 12 iniciais
                cout << "DLC: " << DLC << endl;
                cout << "data: vazio" << endl;
                // cout << " RTR: " << RTR;
                decoder_state = CRC_BASE;
                add_base = 0;
            }
            break;
        
        case DATA_FRAME_BASE:
            if(count <= (18 + cnt_bit_stuffing) && !flag_bit_stuff) {
                // cout << "dlc lido no bit: " << dec << count << endl;
                DLC = DLC << 1 | (bit_atual & 1);

            }

            if(count > (18 + cnt_bit_stuffing) && count <= (18 + DLC*8 + cnt_bit_stuffing) && !flag_bit_stuff) {
                
                // cout << "data lido no bit: " << dec << count << endl;
                data_msg = data_msg << 1 | (bit_atual & 1);
            }

            
            if(count == (18 + DLC * 8 + cnt_bit_stuffing)) {
                // cout << 
                // bitset <64> dlc (data_msg);
                // cout << "Saiu no bit: " << dec <<count << endl;
                cout << "DLC: " << DLC;
                cout << " data: " << hex << uppercase << data_msg << endl;
                add_base = DLC*8;   
                // cout << dlc << endl;
                // exit(1);
                decoder_state = CRC_BASE;            
            }
            break;
        
        case CTRL_EXTENDED_F:
            
            if(count <= (32 + cnt_bit_stuffing) && !flag_bit_stuff) {    
                // cout << "Bit lido: " << bit_atual << endl;
                ID_B = ID_B << 1 | (bit_atual & 1);                
            }
            
            if(count == (33 + cnt_bit_stuffing) && !flag_bit_stuff){
                RTR = bit_atual;
            }

            if(count == 35) { // 17 do ID_B e 12 iniciais
                bitset <18> idb(ID_B);
                cout << " RTR: " << RTR << endl;
                cout << "ID_B: " << ID_B << endl;
                // cout << "ID_B: " << hex <<ID_B << endl;
                // exit(1);
                if(RTR == 1) decoder_state = REMOTE_FRAME_EXT;
                else decoder_state = DATA_FRAME_EXT;
            }
            break;
        
        case REMOTE_FRAME_EXT:
            if(count <= (38 + cnt_bit_stuffing) && !flag_bit_stuff) {
                // cout << "dlc lido no bit: " << dec << count << endl;
                DLC = DLC << 1 | (bit_atual & 1);

            }
            
            if(count == (18 + cnt_bit_stuffing)) { //21 do remote + 12 iniciais
                cout << "DLC: " << DLC << endl;
                cout << "data: vazio" << endl;
                // cout << " RTR: " << RTR;
                decoder_state = CRC_EXT;
                add_ext = 0;
            }
            break;
        
        case DATA_FRAME_EXT:

            if(count <= (38 + cnt_bit_stuffing) && !flag_bit_stuff) {
                // cout << "dlc lido no bit: " << dec << count << endl;
                DLC = DLC << 1 | (bit_atual & 1);

            }

            if(count > (38 + cnt_bit_stuffing) && count <= (38 + DLC*8 + cnt_bit_stuffing) && !flag_bit_stuff) {
                
                // cout << "data lido no bit: " << dec << count << endl;
                data_msg = data_msg << 1 | (bit_atual & 1);
            }

            if(count == (38 + DLC*8 + cnt_bit_stuffing)) {
                
                add_ext = DLC*8;
                cout << "DLC: " << DLC;
                cout << " data: " << hex << uppercase << data_msg << endl; 
                decoder_state = CRC_EXT;
                // exit(1);
            }
            break;

        case CRC_BASE:

            if(count <= (33 + add_base + cnt_bit_stuffing) && !flag_bit_stuff) {
                crc_int = crc_int << 1 | (bit_atual & 1);
                
            }
            // cout << "depois CRC: " << crc << endl;
            if(count == (34 + add_base + cnt_bit_stuffing)) { //conta tbm o crc delimiter
                bitset <15> crc(crc_int);
                cout << "CRC: " << crc << endl;
                decoder_state = ACK_BASE;
                err_permission = 0;
            }

            break;
        
        case CRC_EXT:

            if(count <= (53 + add_ext + cnt_bit_stuffing) && !flag_bit_stuff) {
                crc_int = crc_int << 1 | (bit_atual & 1);
            }

            if(count == (54 + add_ext + cnt_bit_stuffing)) { //crc delimiter
                bitset <15> crc(crc_int);
                cout << "CRC: " << crc << endl;
                decoder_state = ACK_EXT;
                cout <<"To ACK" << endl;
                
                err_permission = 0;
                // exit(1);
            }
            break;

        case ACK_BASE:
            if(count == (35 + add_base + cnt_bit_stuffing)) {
                if(bit_atual == 0)
                    cout << "ACK OK" << endl;
                else 
                    cout << "ACK ERROR" << endl;
            }

            if(count == (36 + add_base + cnt_bit_stuffing)) {
                decoder_state = END_OF_FRAME_BASE;
            }
            break;

        case ACK_EXT:
            
            if(count == (55 + add_ext + cnt_bit_stuffing)) { //ACK slot
                // cout << "Count: " << dec << count << endl;
                if(bit_atual == 0)
                    cout << "ACK OK" << endl;
                else 
                    cout << "ACK ERROR" << endl;
            }

            if(count == (56 + add_ext+ cnt_bit_stuffing)) { //ACK delimiter
                decoder_state = END_OF_FRAME_EXT;
            }
            // exit(1);
            break;


        case END_OF_FRAME_BASE:
            if(count == (43 + add_base + cnt_bit_stuffing)) {
                decoder_state = WAIT;
                cout << "Ate EOF" << endl;
                aux = -1;
            }
            break;

        case END_OF_FRAME_EXT:
            if(count == (62 + add_ext + cnt_bit_stuffing)) {
                decoder_state = WAIT;
                cout << "Ate EOF" << endl;
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

    if(err_permission) {

        if(bit_anterior == bit_atual){

            cnt_bit_igual++; 

        } else {

            cnt_bit_igual = 0;
            flag_bit_stuff = 0;

        }
    }

    if(cnt_bit_igual == 4) { //5 bits iguais vieram, o próximo é bit stuffing
        cnt_bit_stuffing++;
        flag_bit_stuff = 1;
        // cout << "Bit stuff, count: " << count << endl;
        // cout << "Count bit stuff: " << cnt_bit_stuffing << endl;
    }        

    if(cnt_bit_igual == 5) {
        decoder_state = ERROR;
        cout << "Erro!" << endl;
    }

    bit_anterior = bit_atual;
}