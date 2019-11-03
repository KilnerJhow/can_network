#include <iostream>
#include <bitset>
#include <sstream>

using namespace std;

volatile int bit_anterior = 0;
volatile int bit_atual = 0;
volatile int cnt_bit_stuffing = 0;
volatile int aux = 1;

void check_error();

int main() {

    string a = "0100010010011111100000100000111110";
    string line;
    bitset <400> buf;

    for(int i = 0; i < a.size(); i++){
        buf[i] = (int)a.at(i) - '0';
    }

    unsigned long int b = 0;

    // ID_A
    int x = 1, i = 1;
    while (aux > 0) {
        bit_atual = buf[i];
        check_error();
        cout << "i: " << i << " bit atual: " << bit_atual  << " bit anterior: " << bit_anterior << " bit count stuffing: " << cnt_bit_stuffing << endl;
        i++;
    }

    return 0;
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
