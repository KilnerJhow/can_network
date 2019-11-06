#include <iostream>
#include <bitset>
#include <sstream>
#include <fstream>
#include <string>

using namespace std;

volatile int bit_anterior = 0;
volatile int bit_atual = 0;
volatile int cnt_bit_stuffing = 0;
volatile int aux = 1;

volatile int cnt_bit_1 = 0;
volatile int cnt_bit_0 = 0;
volatile int next_bit_stuff = 0;
volatile int flag_bit_stuff = 0;
volatile int cnt_bit_stuff = 0;

const uint16_t crc_polinomial = 0x4599;
bitset <15> crc_seq;
bitset <15> crc_check;
uint16_t crc_convert;
int crc_next = 0;

void calculate_crc(int);
void check_crc(int);
void check_bit_stuffing();



int main() {

    // string a = "0000010000011111000001000001000001000001001111101111101111101111101111101111101111101111101111101111101111101111101111101110101100001011111011111111";
    string line;
    string id_a, id_b, rtr, ide, dlc, data;
    int dlc_int = 0;
    ifstream inFile;
    stringstream toHex;
    uint64_t data_msg;
    inFile.open("teste.txt");
    getline(inFile, line);

    cout << line << endl;

    size_t pos = line.find("ID_A = ");
    // size_t inc = 0;
    string a = "ID_A = ";
    id_a = line.substr(pos + a.size(),3);

    a = "ID_B = ";
    pos = line.find("ID_B = ");
    cout << "Pos id b: " << pos << endl;
    if(pos != string::npos) id_b = line.substr(pos + a.size(), 4);
    else id_b = "None";

    // inc = a.size();

    pos = line.find("RTR = ");
    a = "RTR = ";
    rtr = line.substr(pos + a.size(), 1);

    pos = line.find("IDE = ");
    a = "IDE = ";
    ide = line.substr(pos + a.size(), 1);

    pos = line.find("DLC = ");
    a = "DLC = ";
    dlc = line.substr(pos + a.size(), 1);

    dlc_int = (int) dlc[0] - '0';
    dlc_int = dlc_int * 2;


    pos = line.find("DATA = ");
    a = "DATA = ";
    data = line.substr(pos + a.size(), dlc_int);

    toHex << data;
    toHex >> hex >> data_msg;

    cout << "ID_A: " << id_a << endl;
    cout << "ID_B: " << id_b << endl;
    cout << "RTR: " << rtr << endl;
    cout << "IDE: " << ide << endl;
    cout << "DLC: " << dlc << endl;
    cout << "Data: " << data << endl;
    cout << "Data inteiro: " << data_msg << endl;

    bitset <11> id_1;
    int id_ = 0x449;
    bitset <11> tmp(id_);
    id_1 = tmp;
    cout << id_1 << endl;
    // bitset <148> buf;
    // bitset <126> frame;

    // for(int i = 0; i < a.size(); i++){
    //     buf[i] = (int)a.at(i) - '0';
    //     bit_atual = buf[i];
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

// void check_crc(int i) {
//     crc_next = buf[i] ^ crc_check[14];
//     crc_check = crc_check << 1;//Shift left de 1
//     crc_check[0] = 0;
//     crc_convert = (uint16_t) crc_check.to_ulong();
//     if(crc_next){
//         crc_check = crc_convert ^ crc_polinomial;
//     }
// }

void check_bit_stuffing() {
   
    if(bit_atual == 1) {
        cnt_bit_1++;

        if(cnt_bit_1 == 5) {
            next_bit_stuff = 1;
        }

        if(cnt_bit_1 == 6) {
            cout << "Erro bit stuff" << endl;
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
            cout << "Erro bit stuff" << endl;
        }

    } else {
        cnt_bit_0 = 0;
    }
    
}
