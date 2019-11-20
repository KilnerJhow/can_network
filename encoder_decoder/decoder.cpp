#include <iostream>
#include <fstream>
#include <bitset>

using namespace std;

#define WAIT 0
#define ARBITRATION 1
#define CTRL_EXTENDED_F 2
#define DATA_FRAME 3
#define REMOTE_FRAME 4
#define CRC 5
#define END_OF_FRAME 6
#define ERROR 7
#define CTRL_BASE_F 8
#define CTRL_F 9
#define ACK 10
#define INTERMISSION 11
#define OVERLOAD_FRAME 12
#define BUS_IDLE 13


volatile int decoder_state = BUS_IDLE;


volatile int count = 0;
volatile int count_arbitration = 0;
volatile int count_ctrl_f = 0;
volatile int count_ctrl_base_f = 0;
volatile int count_ctrl_base_ext = 0;
volatile int count_data = 0;
volatile int count_remote = 0;
volatile int count_crc = 0;
volatile int count_ack = 0;
volatile int count_eof = 0;
volatile int count_ifs = 0;
volatile int count_overload_0 = 0;
volatile int count_overload_1 = 0;
volatile int count_error_1 = 0;
volatile int count_error_0 = 0;
volatile int count_end_bits = 0;
volatile int count_bus_idle = 0;

volatile int ID_A = 0;
volatile int ID_B = 0;
volatile int IDE = 0;
volatile int RTR = 0;
volatile int SRR = 0;
volatile int bit_12 = 0;
volatile int DLC = 0;
volatile int bit_anterior = 0;
volatile int bit_atual = 0;
volatile int next_bit_stuff = 0;
volatile int flag_bit_stuff = 0;
volatile int cnt_bit_0 = 0;
volatile int cnt_bit_1 = 0;

uint16_t crc_int = 0;


int aux_cnt = 0;
unsigned int buf_[15];
int cnt_field = 0;

volatile int err_permission = 0;

volatile int need_overload_frame = 0;
volatile int crc_err_flag = 0;

volatile int64_t data_msg = 0;

volatile int aux = 1;

bitset <250> buf;
bitset <250> frame;
volatile int frame_count = 0;
// bitset <15> crc;

//For CRC use
const uint16_t crc_polinomial = 0x4599;
bitset <15> crc_seq;
bitset <15> crc_check;
uint16_t crc_convert = 0;
int crc_next = 0;


void calculate_crc(int);
void check_crc(int);
void decode_message();
void decoder_ms();
void check_bit_stuffing();
void resetStates();

int main() {
    string line;
    ifstream inFile;
    for(int i = 0; i < 15; i++) {
        buf_[i] = 0;
        cout << buf[i];
    }
    cout << endl;

    aux_cnt = 0;
    cout << "Aux cnt: " << aux_cnt << endl;

    inFile.open("in.txt");

    if(!inFile) {
        cout << "Problema na abertura do arquivo!";
        exit(1);
    }

    getline(inFile, line);

    for(int i = 0; i < line.size(); i++){
        // buf[i] = (int)line.at(i) - '0';
        bit_atual = ((int)line.at(i) - '0') & 1;
        // cout << "Bit atual: " << bit_atual << "   " << endl;
        decode_message();
    }

    cout << "Frame: " << endl;
    for(int i = frame_count - 1; i >= 0; i--) {
        cout << frame[i];
    }
    cout << endl;

    inFile.close();
}

void decode_message() {
  
    // bit_atual = buf[count];
    // cout << "Bit atual: " << bit_atual << endl; 
    check_bit_stuffing();
    if(!flag_bit_stuff){
        // cout << " cnt igual: " << cnt_bit_igual << endl;
        decoder_ms();
    }

    if(next_bit_stuff) {
        flag_bit_stuff = 1;
        next_bit_stuff = 0;
    } else {
        flag_bit_stuff = 0;
    }

    if(decoder_state != BUS_IDLE) count++;
}

void decoder_ms() {
    frame = frame << 1; 
    frame[0] = bit_atual & 1;
    frame_count++;
    // cout << frame << endl;
    switch(decoder_state) {

        case BUS_IDLE:
            // cout << "Barramento livre count: " << count << endl;
            if(bit_atual == 0 && count == 0) {
                cout << "SOF recebido" << endl;
                // frame.reset();
                //hard sync
                decoder_state = ARBITRATION;
                err_permission = 1;
                check_crc(frame_count);
            } 
            break;

        case ARBITRATION:
            check_crc(frame_count);
            // cout << "Arbitration" << endl;
            if(count_arbitration <= 10) {
                ID_A = ID_A << 1 | (bit_atual & 1);
            }

            if(count_arbitration == 10) {
                decoder_state = CTRL_F;
                cout << "ID_A: 0x" << uppercase << hex << ID_A;
                cout << dec;
                // cout << "COUNT: " << dec << count << endl;
            }
            count_arbitration++;
            break;

        case CTRL_F:
            
            check_crc(frame_count);

            if(count_ctrl_f == 0) bit_12 = bit_atual;
            if(count_ctrl_f == 1) {
                IDE = bit_atual;
                if(IDE == 1) decoder_state = CTRL_EXTENDED_F;
                else decoder_state = CTRL_BASE_F;
            }
            count_ctrl_f++;
            break;

        case CTRL_BASE_F: 

            check_crc(frame_count);

            if(count_ctrl_base_f == 0) {
                //Lê bit r0
                RTR = bit_12;
                cout << " - RTR: " << RTR;
                cout << " - IDE: " << IDE << endl;;
                // cout << "Bit reservado: " << bit_atual << endl;
                if(RTR == 0) decoder_state = DATA_FRAME;
                else decoder_state = REMOTE_FRAME;
            }
            count_ctrl_base_f++;
            break;


        case CTRL_EXTENDED_F:

            check_crc(frame_count);
            
            if(count_ctrl_base_ext <= 17) {    
                // cout << "Bit lido: " << bit_atual << endl;
                ID_B = ID_B << 1 | (bit_atual & 1);                
            }
            
            if(count_ctrl_base_ext == 18){
                RTR = bit_atual;
            }

            if(count_ctrl_base_ext == 20) { //2 bits reservados
                // bitset <18> idb(ID_B);
                SRR = bit_12;
                cout << " - SRR: " << SRR;
                cout << " - RTR: " << RTR;
                cout << " - ID_B: 0x" << uppercase << hex << ID_B;
                cout << dec;
                if(RTR == 1) decoder_state = REMOTE_FRAME;
                else {
                    decoder_state = DATA_FRAME;
                }
            }
            count_ctrl_base_ext++;
            break;

        case REMOTE_FRAME:

            check_crc(frame_count);
            
            if(count_remote <= 3 ) {
                // cout << "dlc lido no bit: " << dec << count << endl;
                DLC = DLC << 1 | (bit_atual & 1);
            }

            if(count_remote == 3) { //21 do remote + 12 iniciais
                cout << " - DLC: " << DLC;
                cout << " - data: vazio" << endl;
                // cout << " RTR: " << RTR;
                decoder_state = CRC;
            }
            count_remote++;
            break;
        
        case DATA_FRAME:

            check_crc(frame_count);

            // cout << "Entrou no data frame" << endl;
            if(count_data <= 3 ) {
                // cout << "dlc lido no bit: " << dec << count << endl;
                DLC = DLC << 1 | (bit_atual & 1);

            }

            if(count_data == 3) {
                if(DLC > 8) {
                    cout << "DLC real: " << DLC << " " << endl;
                    DLC = 8;
                }
            }

            if( (count_data > 3 ) && (count_data <= (3 + DLC*8 ))) {
                
                // cout << "data lido no bit: " << dec << count << endl;
                data_msg = data_msg << 1 | (bit_atual & 1);
                if(aux_cnt == 7) {
                    int a = data_msg & 255;
                    cout << hex << a << endl;
                    buf_[cnt_field] = data_msg & 255;
                    cout << "Buf: " << buf_[cnt_field] << endl;
                    cnt_field++;
                    aux_cnt = 0;
                }
                else aux_cnt++;
            }

            
            if(count_data == (3 + DLC * 8)) {
                // cout << 
                // bitset <64> dlc (data_msg);
                // cout << "Saiu no bit: " << dec <<count << endl;
                // cout << "Buf: " << endl;
                for(int i = 0; i < cnt_field; i++) {
                    cout << buf_[i];
                }
                cout << endl;

                cout << "DLC: " << DLC;
                cout << " - data: " << hex << uppercase << data_msg << endl;
                // cout << dlc << endl;
                // exit(1);
                decoder_state = CRC;
                for(int i = frame_count; i >= 0; i--) {
                    calculate_crc(i);
                }            
                // cout << "CRC calculado: " << crc_seq << endl;
            }
            count_data++;
           
            break;

        case CRC:

            if(count_crc <= 14) {
                crc_int = crc_int << 1 | (bit_atual & 1);    
                check_crc(frame_count);         
            }

            if(count_crc == 14) {

                // for(int i = frame_count; i >= 0; i--) { //-1 devido ao crc delimiter
                //     check_crc(i);
                // }
                bitset <15> crc(crc_int);
                // cout << "CRC lido: " << crc << endl;
                // cout << "CRC check: " << crc_check << endl;
                // cout << "Frame: " << frame << endl;
                // cout << "Frame count: " << frame_count << endl;
                if(crc_check.any()) {
                    cout << "CRC erro!" << endl;
                    cout << "CRC check: " << crc_check << endl;
                    crc_err_flag = 1;
                    // exit(1);
                } else {
                    cout << "CRC ok!" << endl;
                }
                err_permission = 0;
            }

            // cout << "depois CRC: " << crc << endl;
            if(count_crc == 15) { //conta tbm o crc delimiter
                if(bit_atual == 0) {
                    cout << "Erro de CRC delimiter!" << endl;
                    decoder_state = ERROR;
                } else {
                    // cout << "CRC delimiter ok" << endl;
                    decoder_state = ACK;
                    
                }
            }
            count_crc++;
            

            break;

        case ACK:
            if(count_ack == 0) {
                if(bit_atual == 0)
                    cout << " - ACK OK" << endl;
                else {
                    cout << "Erro de ACK!" << endl;
                    decoder_state = ERROR;
                }
            }

            if(count_ack == 1) {    //ack delimiter
                if(crc_err_flag) {
                    decoder_state = ERROR;
                    cout << "Erro de CRC apos o ACK delimiter" << endl;
                } else if(bit_atual == 0) {
                    cout << "Erro de forma no ACK delimiter!" << endl;
                    decoder_state = ERROR;
                }
                else {
                    decoder_state = END_OF_FRAME;
                    // cout << "ACK delimiter ok" << endl;
                }
            }

            count_ack++;
            break;

        case END_OF_FRAME:

            if(count_eof == 6) {
                // cout << "Ate EOF" << endl;
                if(need_overload_frame) decoder_state = OVERLOAD_FRAME;
                else {
                    decoder_state = INTERMISSION;
                }
                //TO-DO: Retirar isso para ir para o Intermission
                aux = -1;
            }
            if(bit_atual == 0) {
                decoder_state = ERROR;
                cout << "Erro de forma no EOF!" << endl;
            }
            count_eof++;
            
            break;
        
        case INTERMISSION:
            // cout << "Intermission" << endl;
            if(bit_atual == 0 && count_ifs <= 1) {
                //diz ao encoder para escrever 6 0
                decoder_state = OVERLOAD_FRAME;
                cout << "Entrando em um frame de overload de intermission!" << endl;
                // cout << "count ifs: " << count_ifs << endl;
            }

            if(count_ifs == 2) {
                // cout << "Interframe space count: " << count_ifs << endl;
                decoder_state = BUS_IDLE;
                resetStates();
                cout << endl;
                // aux = -1;
            } else count_ifs++;

            break;

        case OVERLOAD_FRAME:
            if(count_overload_0 == 4) {
                cout << "Bits dominantes de overload recebidos" << endl;
                //diz ao encoder para escrever 6 bits dominantes
            }

            if(count_overload_1 == 7) {   //espera ler 8 bits um do barramento
                decoder_state = WAIT;
                cout << "Bits recessivos de overload recebidos" << endl;

                // aux = -1;
            }

            if(bit_atual == 1) {
                count_overload_1++;
            } else {
                count_overload_0++;
            }
            break;
        
        case ERROR:
            // escreve 6 bits 0 e depois 8 bits 1, não se importa com a leitura
            //após a transmissão de 6 bits 1 o encoder envia bits 1 e espera a leitura deles
            if(count_error_0 == 5){
                cout << "Bits dominantes recebidos" << endl;
            }

            if(count_error_1 == 7) {  //após a transmissão de 1 bit recessivo, ele conta mais 7 bits recessivos
                cout << "Bits recessivos recebidos, saindo do erro" << endl;
                decoder_state = WAIT;   //após o frame de erro, pode vir frames de overload
                // aux = -1;
            }
            if(bit_atual == 1) count_error_1++;
            else count_error_0++;

            break;

        case WAIT:
            //precisamos esperar 11 bits recessivos para saber que o barramento está em idle 
            //Esse estado sempre vem do erro ou do overload, logo 8 bits recessivos foram contados anteriormente
            //Precisamos contar apenas mais 3 bits recessivos para o barramento ficar em idle
            //Caso percebamos um bit 0 após os 8 bits recessivos, isso significa que é um overload frame
            if(count_bus_idle == 10) {
                decoder_state = BUS_IDLE;
                resetStates();
                aux = -1; 
                cout << "Bus idle" << endl;
            }

            if(bit_atual == 1) {
                count_bus_idle++;
            } else {
                decoder_state = OVERLOAD_FRAME;
                cout << "Entrando em um frame de overload a partir do wait!" << endl;
            }
            
            if(bit_atual == 0) decoder_state = OVERLOAD_FRAME; 

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
   
    if(err_permission) {
        if(bit_atual == 1) {
            cnt_bit_1++;

            if(cnt_bit_1 == 5) {
                next_bit_stuff = 1;
            }

            if(cnt_bit_1 == 6) {
                cout << "Erro bit stuff na pos: " << count + 1 << endl;
                decoder_state = ERROR;
                err_permission = 0;
            }

        } else {
            cnt_bit_1 = 0;
        }

        if(bit_atual == 0) {
            cnt_bit_0++;

            if(cnt_bit_0 == 5) {
                next_bit_stuff = 1;
            }

            if(cnt_bit_0 == 6) {
                cout << "Erro bit stuff na pos: " << count + 1 << endl;

                decoder_state = ERROR;
                err_permission = 0;
            }

        } else {
            cnt_bit_0 = 0;
        }
    }
}

void resetStates() {

    count = 0;
    count_arbitration = 0;
    count_ctrl_f = 0;
    count_ctrl_base_f = 0;
    count_ctrl_base_ext = 0;
    count_data = 0;
    count_remote = 0;
    count_crc = 0;
    count_ack = 0;
    count_eof = 0;
    count_ifs = 0;
    count_overload_0 = 0;
    count_overload_1 = 0;
    count_error_1 = 0;
    count_error_0 = 0;
    count_end_bits = 0;
    count_bus_idle = 0;

    ID_A = 0;
    ID_B = 0;
    IDE = 0;
    RTR = 0;
    SRR = 0;
    bit_12 = 0;
    DLC = 0;
    bit_anterior = 0;
    bit_atual = 0;
    next_bit_stuff = 0;
    flag_bit_stuff = 0;
    cnt_bit_0 = 0;
    cnt_bit_1 = 0;

    data_msg = 0;

    err_permission = 0;

    need_overload_frame = 0;
    crc_err_flag = 0;

    frame.reset();
    frame_count = 0;

    crc_seq.reset();
    crc_check.reset();

    // cout << "Estados resetados!" << endl;
}