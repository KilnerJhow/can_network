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
#define CONTROL_RESBIT 6
#define CONTROL 7
#define DATA 8
#define CRC 9
#define CRC_D 10
#define ACK 11
#define END 12

volatile int  enc_state = SOF;
volatile int enc_cnt=0;
volatile int encstuff_cnt=0;
volatile int save_enc_cnt=0;
volatile int i=0;
volatile int rtr;
volatile int ide;
volatile int dlc;
volatile int actual_bit;
volatile int past_bit;
volatile int cnt_stuff=0;
volatile int cnt_bit_1=0;
volatile int cnt_bit_0=0;
volatile int bit_atual=0;
volatile int save_encstuff_cnt=0;
volatile int frame_build=0;

bitset <250>enc_frame;
bitset <250>enc_stuffframe;
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
void bit_stuf();
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
    unsigned long long int data_msg;
	unsigned int id_a_hex;
	unsigned int id_b_hex;
    inFile.open("input.txt");
    getline(inFile, line);

    cout << line << endl;

    size_t pos = line.find("ID_A = ");
    // size_t inc = 0;
    string a = "ID_A = ";
    id_a = line.substr(pos + a.size(),3);

    a = "ID_B = ";
    pos = line.find("ID_B = ");
    // cout << "Pos id b: " << pos << endl;
    if(pos != string::npos) {
		id_b = line.substr(pos + a.size(), 5);
		toHex << id_b;
		toHex >> hex >> id_b_hex;
		toHex.clear();
	}
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
	toHex.clear();

	toHex << id_a;
	toHex >> hex >> id_a_hex;
	toHex.clear();

    cout << "ID_A: " << id_a << endl;
    cout << "ID_A hex: " << uppercase << hex << id_a_hex << dec << endl;
    cout << "ID_B: " << id_b << endl;
    cout << "ID_B hex: " << uppercase << hex << id_b_hex << dec << endl;
    cout << "RTR: " << rtr << endl;
    cout << "IDE: " << ide << endl;
    cout << "DLC: " << dlc << endl;
    cout << "Data: " << data << endl;
    cout << "Data inteiro: " << uppercase << hex << data_msg << endl;
	 inFile.close();
}

void encoder_mws(){
	while(frame_build=0){
		bit_stuf();
		switch(enc_state){
			
			case SOF:
				enc_frame[enc_cnt]=0;
				enc_cnt++;
				enc_stuffframe[encstuff_cnt]=0;
		        encstuff_cnt++;
				enc_state=ID;
				break;
			
			case ID:
				
				if(i<11){
				enc_frame[enc_cnt]=buf_id[i];
				enc_stuffframe[encstuff_cnt]=buf_id[i];
				enc_cnt++;
				encstuff_cnt++;
				i++;
				}else{
					i=0;
					if(ide=1){
						enc_frame[enc_cnt]=1;//seting SRR
						enc_stuffframe[encstuff_cnt]=1;
						enc_cnt++;
						encstuff_cnt++;
						enc_state=IDE;
					}else{
						enc_state=RTR;
					}
				}
					
				break;
			
			case ID2:
			
				if(i<18){
					enc_frame[enc_cnt]=buf_id2[i];
					enc_stuffframe[encstuff_cnt]=buf_id2[i];
					enc_cnt++;
					encstuff_cnt++;
					i++;
				}else{
					i=0;
					enc_state=RTR;
				}
				
				break;
			
			case RTR:
				if(rtr==0){
					enc_frame[enc_cnt]=0;
					enc_stuffframe[encstuff_cnt]=0;
				}else{
					enc_frame[enc_cnt]=1;
					enc_stuffframe[encstuff_cnt]=1;
				}
				enc_cnt++;
				encstuff_cnt++;
				if(ide==0){
					enc_state=IDE;
					}else{
					enc_state=CONTROL_RESBIT;	
					}
				break;
			
			case IDE:
				enc_frame[enc_cnt]=ide;						//grava o IDE
				enc_stuffframe[encstuff_cnt]=ide;
				enc_cnt++;
				encstuff_cnt++;
				if(ide==0){
					enc_state=CONTROL_RESBIT;
					}else{
					enc_state=ID2;	
					}
			
			case CONTROL_RESBIT:
				if(ide==0){
					enc_frame[enc_cnt]=1;		//caso normal 1 bits reservado
					enc_stuffframe[encstuff_cnt]=1;
					enc_cnt++;
					encstuff_cnt++;
					enc_state=CONTROL;
				}else{							//caso extendido 2 bits reservados
					if(i<2){
						enc_frame[enc_cnt]=1;
						enc_stuffframe[encstuff_cnt]=1;
						i++;
						enc_cnt++;
						encstuff_cnt++;
					}else{
						i=0;
						enc_state=CONTROL;
					}
				}
			case CONTROL:
				if(i<4){
					enc_frame[enc_cnt]=buf_dlc[i]; //grava o dlc em bin�rio
					enc_stuffframe[encstuff_cnt]=buf_dlc[i];
					i++;
					enc_cnt++;
					encstuff_cnt++;
				}else{
					i=0;
					enc_state=DATA;
				}
				
				break;
				
			case DATA:
				if(rtr=0){
					if(i<dlc*8){		
						enc_frame[enc_cnt]=buf_data[i];
						enc_stuffframe[encstuff_cnt]=buf_data[i];
						i++;
						enc_cnt++;
						encstuff_cnt++;
					}else{
						i=0;
						enc_state=CRC;
					}
				
				}else{
					enc_state=CRC;
				}
				
				break;	
			
			case CRC:
				//!!!!!CALCULAR CRC!!!!!
					if(i<15){
					enc_frame[enc_cnt]=buf_crc[i];
					enc_stuffframe[encstuff_cnt]=buf_crc[i];
					i++;
					enc_cnt++;
					encstuff_cnt++;
					}else{
						i=0;
						enc_state=CRC_D;
					}
				
					
				break;
			
			case CRC_D:
				enc_frame[enc_cnt]=1;		//crc delimiter must be 1
				enc_stuffframe[encstuff_cnt]=1;
				enc_cnt++;	
				encstuff_cnt++;
				enc_state=ACK;
				break;
			case ACK:
				if(i<2){
				enc_frame[enc_cnt]=1;			//ack transmiter must be 1
				enc_stuffframe[encstuff_cnt]=1;
				i++;
				enc_cnt++;
				encstuff_cnt++;
			}else{
				i=0;
				enc_state=END;
			}
				
				break;
			
			case END:
				
				if(i<7){
					enc_frame[enc_cnt]=1;
					enc_stuffframe[encstuff_cnt]=1;
					i++;
					enc_cnt++;
					encstuff_cnt++;
				}else{
				i=0;
				save_enc_cnt=enc_cnt;
				save_encstuff_cnt=encstuff_cnt;
				encstuff_cnt=0;
				enc_cnt=0;
				frame_build=1;	
				}			
				
				break;
				
			}
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
void bit_stuf(){
	/*if(actual_bit==past_bit){
		cnt_stuff++;
	}else{
		cnt_stuff=0;
	}		
	if(cnt_stuff>=4){
		return 1;
	}else{
		return 0;
	}*/
	
	if((enc_state!=ACK)and(enc_state!=END)and(enc_state!=CRC_D)){
		if(bit_atual == 1) {
	            cnt_bit_1++;
	
	            if(cnt_bit_1 == 5) {
	                enc_stuffframe[encstuff_cnt]=0;
	                encstuff_cnt++;
	            }
			} else {
	            cnt_bit_1 = 0;
	        }
	
	        if(bit_atual == 0) {
	            cnt_bit_0++;
	
	            if(cnt_bit_0 == 5) {
	                enc_stuffframe[encstuff_cnt]=1;
	                encstuff_cnt++;
	            } 
			} else {
	            cnt_bit_0 = 0;
	        }
	    }
	}   

