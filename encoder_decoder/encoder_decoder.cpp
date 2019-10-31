#include <iostream>
#include <fstream>

using namespace std;

#define WAIT 0
#define ARBITRATION 1
#define CTRL_BASE_F 2
#define CTRL_EXTENDED_F 3
#define DATA_FRAME_BASE 4
#define REMOTE_FRAME_BASE 5
#define DATA_FRAME_EXT 6
#define REMOTE_FRAME_EXT 7
#define ACK_SEND 8
#define INTERFRAME_SPACE 9

volatile int decoder_state = 0;
volatile int count = 0;
volatile int IDE = 0;
volatile int RTR = 0;
volatile int DLC = 0;

int main() {
    string line;
    ifstream inFile;
    inFile.open("in.txt");

    if(!inFile) {
        cout << "Problema na abertura do arquivo!";
        exit(1);
    }
    while(getline(inFile, line)){
        cout << line << endl;
    }

    char firstPos = line.at(0);
    
    int stream = (int)firstPos - '0';

    cout << firstPos << endl;
    cout << stream << endl;

    inFile.close();
}

void decoder_ms() {
    switch(decoder_state) {
        case WAIT:
            if(count == 12) {
                count = 0;
                if(IDE == 1) decoder_state = CTRL_EXTENDED_F;
                else {
                    if(RTR == 1) decoder_state = REMOTE_FRAME_BASE;
                    else decoder_state = DATA_FRAME_BASE;
                }
            } else count++;
            break;

        case REMOTE_FRAME_BASE:
            if(count == 21) {
                count = 0;
                decoder_state = ACK_SEND;
            } else count++;
            break;
        
        case DATA_FRAME_BASE:
            if(count == (20 + DLC))
            
    }
}