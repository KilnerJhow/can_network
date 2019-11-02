#include <iostream>
#include <bitset>

using namespace std;

int main() {

    string a = "000001111000100100010001";
    string line;
    bitset <100> buf;

    for(int i = 0; i < a.size(); i++){
        buf[i] = (int)a.at(i) - '0';
    }

    // b[2] = (int) a.at(0) - '0';
    // b[1] = (int) a.at(1) - '0';
    // b[0] = (int) a.at(2) - '0';

    bitset <3> c(7);

    cout << buf[0] << endl;
    cout << buf << endl;
    cout << c << endl;

    return 0;
}
