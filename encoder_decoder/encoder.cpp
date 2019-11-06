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

volatile int enc_state = SOF;
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


const uint16_t crc_polinomial = 0x4599;
bitset <15> crc_seq;
bitset <15> crc_check;
uint16_t crc_convert = 0;
int crc_next = 0;
unsigned long long int crc_count = 0;

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
void calculate_crc(int i);
int main() {
	
	readNstore();
	// cout << "ID_A: " << hex << buf_id.to_ulong() << endl;
	// cout << "ID_B: " << hex << buf_id2.to_ulong() << endl;
	// cout << "DLC: " << hex << buf_dlc.to_ulong() << endl;
	// cout << "Data: " << hex << buf_data.to_ullong() << endl;
	cout << "DLC: " << dlc << endl;
	encoder_mws();
   	snd_msg();
	
	
	return 0;
}

void readNstore(){
 	string line;
    string id_a_str, id_b_str, rtr_str, ide_str, dlc_str, data_str;	
	unsigned long long int data_msg;
	unsigned int id_a_hex;
	unsigned int id_b_hex;
    int dlc_int = 0;
    ifstream inFile;
    stringstream toHex;
    inFile.open("input.txt");
    getline(inFile, line);

    // cout << line << endl;

    size_t pos = line.find("ID_A = ");
    // size_t inc = 0;
    string a = "ID_A = ";
    id_a_str = line.substr(pos + a.size(), 3);

    a = "ID_B = ";
    pos = line.find("ID_B = ");
    // cout << "Pos id b: " << pos << endl;
    if(pos != string::npos) {
		id_b_str = line.substr(pos + a.size(), 5);
		toHex << id_b_str;
		toHex >> hex >> id_b_hex;
		toHex.clear();
	}
    else id_b_str = "None";

    // inc = a.size();

    pos = line.find("RTR = ");
    a = "RTR = ";
    rtr_str = line.substr(pos + a.size(), 1);
	rtr = (int) rtr_str.at(0) - '0';
	//'8' - '0' 

    pos = line.find("IDE = ");
    a = "IDE = ";
    ide_str = line.substr(pos + a.size(), 1);
	ide = (int) ide_str.at(0) - '0';

    pos = line.find("DLC = ");
    a = "DLC = ";
    dlc_str = line.substr(pos + a.size(), 1);

    dlc_int = (int) dlc_str.at(0) - '0';
	dlc = dlc_int;
    dlc_int = dlc_int * 2;


    pos = line.find("DATA = ");
    a = "DATA = ";
    data_str = line.substr(pos + a.size(), dlc_int);

    toHex << data_str;
    toHex >> hex >> data_msg;
	toHex.clear();

	toHex << id_a_str;
	toHex >> hex >> id_a_hex;
	toHex.clear();

    // cout << "ID_A: " << id_a_str << endl;
    // cout << "ID_A hex: " << uppercase << hex << id_a_hex << dec << endl;
    // cout << "ID_B: " << id_b_str << endl;
    // cout << "ID_B hex: " << uppercase << hex << id_b_hex << dec << endl;
    // cout << "RTR: " << rtr_str << endl;
    // cout << "IDE: " << ide_str << endl;
    // cout << "DLC: " << dlc_str << endl;
	// cout << "DLC int: " << dlc_int/2 << endl;
    // cout << "Data: " << data_str << endl;
    // cout << "Data inteiro: " << uppercase << hex << data_msg << endl;
	inFile.close();

	bitset <11> tmp_id_a (id_a_hex);
	bitset <18> tmp_id_b (id_b_hex);
	bitset <4> tmp_dlc (dlc_int/2);
	bitset <64> tmp_msg (data_msg);

	buf_id = tmp_id_a;
	buf_id2 = tmp_id_b;
	buf_dlc = tmp_dlc;
	buf_data = tmp_msg;
}

void encoder_mws(){

	while(frame_build == 0){
		// cout << "Encoder while " << endl;
		bit_stuf();
		// cout <<"Saindo bit stuff" << endl;
		switch(enc_state){
			
			case SOF:
					enc_frame[enc_cnt]=0;
					enc_cnt++;
					enc_stuffframe[encstuff_cnt]=0;
					encstuff_cnt++;
					enc_state=ID;
					// cout << "SOF indo para ID" << endl;
				break;
			
			case ID:
				
				if(i<11){
					// cout << "ID" << endl;
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
					cout <<" para DATA" << endl;
					i=0;
					enc_state=DATA;
				}
				
				break;
				
			case DATA:
				if(rtr == 0){
					if(i < (dlc*8)){		
						enc_frame[enc_cnt]=buf_data[i];
						enc_stuffframe[encstuff_cnt]=buf_data[i];
						i++;
						enc_cnt++;
						encstuff_cnt++;

						calculate_crc(crc_count);
						// cout <<"Data" << endl;
					}else{
						i=0;
						enc_state = CRC;
						// cout << "CRC calculado: "<< buf_crc << endl;
					}
					crc_count++;
				
				}else{
					enc_state=CRC;
				}
				
				break;	
			
			case CRC:
				//!!!!!CALCULAR CRC!!!!!
					if(i < 15){	
						enc_frame[enc_cnt] = buf_crc[i];
						enc_stuffframe[encstuff_cnt] = buf_crc[i];
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
	outFile<< "Começando escrita" << endl;
	for(int i=0;i<save_encstuff_cnt;i++){
	
		outFile <<enc_stuffframe[i];
	}
	
	outFile.close();
	save_enc_cnt=0;
	enc_frame=0;
}

void calculate_crc(int i) {
    crc_next = enc_frame[i] ^ buf_crc[14];
    buf_crc = buf_crc << 1;//Shift left de 1
    buf_crc[0] = 0;
    crc_convert = (uint16_t) buf_crc.to_ulong();
    if(crc_next){
        buf_crc = crc_convert ^ crc_polinomial;
    }
}

void bit_stuf(){
	
	if((enc_state != ACK) && (enc_state!=END) && (enc_state!=CRC_D)){
		if(enc_stuffframe[encstuff_cnt] == 1) {
	            cnt_bit_1++;
	
	            if(cnt_bit_1 == 5) {
	                enc_stuffframe[encstuff_cnt]=0;
	                encstuff_cnt++;
	            }
			} else {
	            cnt_bit_1 = 0;
	        }
	
	        if(enc_stuffframe[encstuff_cnt] == 0) {
	            cnt_bit_0++;
	
	            if(cnt_bit_0 == 5) {
	                enc_stuffframe[encstuff_cnt]=1;
	                encstuff_cnt++;
	            } 
			} else {
	            cnt_bit_0 = 0;
	        }
	    }
		// cout << "Fim do bit stuff" << endl;
}   

