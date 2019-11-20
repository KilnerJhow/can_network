#include <iostream>
#include <bitset>
#include <sstream>
#include <fstream>
#include <string>

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

using namespace std;

// volatile int bit_anterior = 0;
volatile int bit_atual = 0;
// volatile int cnt_bit_stuffing = 0;
// volatile int aux = 1;

// volatile int cnt_bit_1 = 0;
// volatile int cnt_bit_0 = 0;
// volatile int next_bit_stuff = 0;
// volatile int flag_bit_stuff = 0;
// volatile int cnt_bit_stuff = 0;

// const uint16_t crc_polinomial = 0x4599;
// bitset <15> crc_seq;
bitset <15> crc_check;
uint16_t crc_convert;
// int crc_next = 0;

void check_crc();
void check_bit_stuffing();

const uint16_t crc_polinomial = 0x4599;
uint16_t crc_seq = 0;
// uint16_t crc_convert = 0;
uint8_t crc_next = 0;

uint16_t buf_crc = 0;

void calculate_crc() {
    crc_next =  bit_atual ^ bitRead(buf_crc, 14);
    buf_crc = buf_crc << 1;//Shift left de 1
    bitClear(buf_crc, 0);
    bitClear(buf_crc, 15);
    if(crc_next){
        buf_crc = buf_crc ^ crc_polinomial;
    }
}

void check_crc() {
    crc_next = bit_atual ^ crc_check[14];
    crc_check = crc_check << 1;//Shift left de 1
    crc_check[0] = 0;
    crc_convert = (uint16_t) crc_check.to_ulong();
    if(crc_next){
        crc_check = crc_convert ^ crc_polinomial;
    }
}


bitset <150> buf;

int main() {

    string a = "011001110010000011110101010101010101010101010101010101010101010101010101010101100110011101";
   // string line;
    // string id_a, id_b, rtr, ide, dlc, data;
    // int dlc_int = 0;
    // ifstream inFile;
    // stringstream toHex;
    // uint64_t data_msg;
    // inFile.open("teste.txt");
    // getline(inFile, line);

    for(int i = 0; i < a.size(); i++){
        bit_atual = a.at(i) - '0';
        calculate_crc();
        check_crc();
    }
    bitset <16> c(buf_crc); 
    cout << "Buf: " << c <<endl;
    cout << "Check: " << crc_check.to_ulong() << endl;

    // bitset <126> frame;

    // int enc_stuffframe[128];

    // enc_stuffframe[0] = 1;
    // enc_stuffframe[1] = 0;
    // enc_stuffframe[2] = 1;
    // cout << "0: " << enc_stuffframe[0] << endl;
    // cout << enc_stuffframe[1] <<endl;
    // cout << enc_stuffframe[2] <<endl;
    /*
    for(int i = 0; i < a.size(); i++){
        buf[i] = (int)a.at(i) - '0';
        bit_atual = buf[i];
    }

    for(int i = 0; i < a.size(); i++) {
        calculate_crc(i);
    }
    cout << "CRC: " << crc_seq << endl;

    //     check_bit_stuffing();
    //     if(!flag_bit_stuff){
    //         frame = frame << 1; 
    //         frame[0] = bit_atual & 1;
    //     } else {
    //         cnt_bit_stuff++;
    //     }

    //     if(next_bit_stuff) {
    //         flag_bit_stuff = 1;
    //         next_bit_stuff = 0;
    //     }
    //     else {
    //         flag_bit_stuff = 0;
    //     }
    //     // calculate_crc(i);
    //     // check_crc(i);
    // }

    // // cout << "CRC: " << crc_check;
    // cout << frame << endl;
    // cout << buf << endl;
    // cout << cnt_bit_stuff << endl;

    // unsigned long int b = 0;

    // // ID_A
    // int x = 1, i = 1;
    // while (aux > 0) {
    //     bit_atual = buf[i];
    //     check_error();
    //     cout << "i: " << i << " bit atual: " << bit_atual  << " bit anterior: " << bit_anterior << " bit count stuffing: " << cnt_bit_stuffing << endl;
    //     i++;
    // }
    */
    return 0;
}

// void calculate_crc(int i) {
//     crc_next = buf[i] ^ crc_seq[14];
//     crc_seq = crc_seq << 1;//Shift left de 1
//     crc_seq[0] = 0;
//     crc_convert = (uint16_t) crc_seq.to_ulong();
//     if(crc_next){
//         crc_seq = crc_convert ^ crc_polinomial;
//     }
// }


// void check_bit_stuffing() {
   
//     if(bit_atual == 1) {
//         cnt_bit_1++;

//         if(cnt_bit_1 == 5) {
//             next_bit_stuff = 1;
//         }

//         if(cnt_bit_1 == 6) {
//             cout << "Erro bit stuff" << endl;
//         }

//     } else {
//         cnt_bit_1 = 0;
//     }

//     if(bit_atual == 0) {
//         cnt_bit_0++;

//         if(cnt_bit_0 == 5) {
//             next_bit_stuff = 1;
//         }

//         if(cnt_bit_0 == 6) {
//             cout << "Erro bit stuff" << endl;
//         }

//     } else {
//         cnt_bit_0 = 0;
//     }
    
// }
