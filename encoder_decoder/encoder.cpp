#include <iostream>
#include <fstream>
#include <bitset>

using namespace std;

#define SOF 1
#define ID 2
#define ID2 3
#define RTR 4
#define IDE 5
#define CONTROL 6 
#define DATA 7
#define CRC 8
#define ACK 9
#define END 10

volatile int  enc_state = SOF;
volatile int enc_cnt=0;
volatile int i=0;
volatile int rtr;
volatile int ide;
volatile int dlc;


bitset <250>enc_frame;
bitset <250> buf_msg2send;
bitset <11> buf_id;
bitset <4> buf_dlc;
bitset <15> buf_crc;
bitset <64> buf_data;

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main() {
	
	 string line;
    ifstream inFile;
    
    inFile.open("input.txt");

    if(!inFile) {
        cout << "Problema na abertura do arquivo!";
        exit(1);
    }

    getline(inFile, line);

    for(int i = 0; i < line.size(); i++){
        buf_msg2send[i] = (int)line.at(i) - '0';
    }


    inFile.close();
	
	ofstream outFile;
	outFile.open("output.txt");
	
	  if(!outFile) {
        cout << "Problema na abertura do arquivo!";
        exit(1);
    }
	
	for(int i=0;i<buf_msg2send.size();i++){
	outFile <<buf_msg2send[i];
}
	outFile.close();
	return 0;
}
void encoder_mws(){
	switch(enc_state){
		
		case SOF:
			enc_frame[enc_cnt]=0;
			enc_cnt++;
			enc_state=ID;
			break;
		
		case ID:
			for(i = 0; i < 11; i++){
				enc_frame[i+enc_cnt]=buf_id[i];
			}
			enc_cnt=enc_cnt+12;    //11 do ID + 1
			if(ide = 1){
				enc_frame[enc_cnt]=1;//seting SRR
				enc_cnt++;
				enc_state=IDE;
			} else {
				enc_state=RTR;
			}
			break;
		
		case ID2:
			for(i = 0; i < 18; i++){
				enc_frame[i + enc_cnt] = buf_id[i];
			}
			enc_cnt=enc_cnt+19;    //18 do ID + 1
			enc_state=RTR;
			break;
		
		case RTR:
			if(rtr==0){
				enc_frame[enc_cnt]=0;
			}else{
				enc_frame[enc_cnt]=1;
			}
			enc_cnt++;
			if(ide==0){
				enc_state=IDE;
				}else{
				enc_state=CONTROL;	
				}
			break;
		
		case IDE:
			enc_frame[enc_cnt]=ide;						//grava o IDE
			enc_cnt++;
			if(ide==0){
				enc_state=CONTROL;
				}else{
				enc_state=ID2;	
				}
		
		case CONTROL:
			if(ide=0){
				enc_frame[enc_cnt]=1;		//caso normal 1 bits reservado
				enc_cnt++;
			}else{							//caso extendido 2 bits reservados
				enc_frame[enc_cnt]=1;
				enc_cnt++;
				enc_frame[enc_cnt]=1;
				enc_cnt++;
			}
			for(i=0;i<4;i++){
			enc_frame[i+enc_cnt]=buf_dlc[i]; //grava o dlc em bin�rio
			}
			enc_cnt=enc_cnt+4;				//4 do DLC + 1
			enc_state=DATA;
			break;
			
		case DATA:
			if(rtr=0){
			for(i=0;i<dlc*8-1;i++){
				enc_frame[i+enc_cnt]=buf_data[i];
			}
			enc_cnt=enc_cnt+dlc*8+1;				// dlc*8 bits do data +1
			}
			enc_state=CRC;
			break;	
		case CRC:
			//!!!!!CALCULAR CRC!!!!!
			for(i=0;i<15;i++){
				enc_frame[i+enc_cnt]=buf_crc[i];
			}
			enc_cnt=enc_cnt+16;				// 15 do crc +1
				enc_frame[enc_cnt]=1;		//crc delimiter must be 1
			enc_cnt++;	
			enc_state=ACK;
			break;
		
		case ACK:
			enc_frame[enc_cnt]=1;			//ack transmiter must be 1
			enc_cnt++;
			enc_frame[enc_cnt]=1;			//ack delimeter
			enc_cnt++;
			enc_state=END;
			break;
		
		case END:
			for(i=0;i<7;i++){
				enc_frame[i+enc_cnt]=1;
			}				
			enc_cnt=0;
			break;
			
		
	}
	
}


