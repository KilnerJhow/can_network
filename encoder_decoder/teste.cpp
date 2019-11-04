#include <iostream>
#include <bitset>
#include <sstream>

using namespace std;

volatile int bit_anterior = 0;
volatile int bit_atual = 0;
volatile int cnt_bit_stuffing = 0;
volatile int aux = 1;

const uint16_t crc_polinomial = 0x4599;
bitset <15> crc_seq;
bitset <15> crc_check;
uint16_t crc_convert;
int crc_next = 0;

void calculate_crc(int);
void check_crc(int);
void check_error();

bitset <400> buf;

int main() {

    string a = "01100111001000010001010101010101010101010101010101010101010101010101010101010101010000000001010001";
    string line;

    for(int i = 0; i < a.size(); i++){
        buf[i] = (int)a.at(i) - '0';
        // calculate_crc(i);
        check_crc(i);
    }

    cout << "CRC: " << crc_check;

    // unsigned long int b = 0;

    // // ID_A
    // int x = 1, i = 1;
    // while (aux > 0) {
    //     bit_atual = buf[i];
    //     check_error();
    //     cout << "i: " << i << " bit atual: " << bit_atual  << " bit anterior: " << bit_anterior << " bit count stuffing: " << cnt_bit_stuffing << endl;
    //     i++;
    // }

    return 0;
}

void calculate_crc(int i) {
    crc_next = buf[i] ^ crc_seq[14];
    crc_seq = crc_seq << 1;//Shift left de 1
    crc_seq[0] = 0;
    crc_convert = (uint16_t) crc_seq.to_ulong();
    if(crc_next){
        crc_seq = crc_convert ^ crc_polinomial;
    }
}

void check_crc(int i) {
    crc_next = buf[i] ^ crc_check[14];
    crc_check = crc_check << 1;//Shift left de 1
    crc_check[0] = 0;
    crc_convert = (uint16_t) crc_check.to_ulong();
    if(crc_next){
        crc_check = crc_convert ^ crc_polinomial;
    }
}

void check_error() {

    if(bit_anterior == bit_atual){
        cnt_bit_stuffing++;       
        if(cnt_bit_stuffing == 5) {
            // decoder_state = ERROR;
            //cout << "Erro ocorrido, bit stuffing!" << endl;
            aux = -1;
        }
    } else {
        cnt_bit_stuffing = 0;
    }
    bit_anterior = bit_atual;
}
