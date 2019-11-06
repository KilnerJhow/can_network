#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>
#include <string>


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
volatile int save_enc_cnt=0;
volatile int i=0;
volatile int rtr;
volatile int ide;
volatile int dlc;
volatile int actual_bit;
volatile int past_bit;
volatile int cnt_stuff=0;


bitset <250>enc_frame;
bitset <250> buf_msg2send;
bitset <11> buf_id;
bitset <18> buf_id2;
bitset <4> buf_dlc;
bitset <15> buf_crc;
bitset <64> buf_data;

/* run this program using the console pauser or add your own getch, system("pause") or input loop */
void readNstore();
void encoder_mws();
void snd_msg();

int main() {
	
	readNstore();
	encoder_mws();
   	snd_msg();
	
	
	return 0;
}

void readNstore(){
 string line;
    string id_a, id_b, rtr, ide, dlc, data;
    int dlc_int = 0;
    ifstream inFile;
    stringstream toHex;
    long long int data_msg;
    inFile.open("input.txt");
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
	 inFile.close();
}

void encoder_mws(){
	switch(enc_state){
		
		case SOF:
			enc_frame[enc_cnt]=0;
			enc_cnt++;
			enc_state=ID;
			break;
		
		case ID:
			for(i=0;i<11;i++){
			enc_frame[i+enc_cnt]=buf_id[i];
			}
			enc_cnt=enc_cnt+12;    //11 do ID + 1
			if(ide=1){
				enc_frame[enc_cnt]=1;//seting SRR
				enc_cnt++;
				enc_state=IDE;
			}else{
				enc_state=RTR;
			}
			break;
		
		case ID2:
			for(i=0;i<18;i++){
			enc_frame[i+enc_cnt]=buf_id2[i];
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
			enc_frame[i+enc_cnt]=buf_dlc[i]; //grava o dlc em binário
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
			enc_cnt=enc_cnt+8;
			save_enc_cnt=enc_cnt;
			enc_cnt=0;
			break;
			
		}
}
void snd_msg(){
	ofstream outFile;
	outFile.open("output.txt");
	
	  if(!outFile) {
        cout << "Problema na abertura do arquivo!";
        exit(1);
    }
	
	for(int i=0;i<save_enc_cnt;i++){
	
	outFile <<enc_frame[i];
}
	
	outFile.close();
	save_enc_cnt=0;
	enc_frame=0;
}
int bit_stuf(){
	if(actual_bit==past_bit){
		cnt_stuff++;
	}else{
		cnt_stuff=0;
	}		
	if(cnt_stuff>=4){
		return 1;
	}else{
		return 0;
	}
	
}
