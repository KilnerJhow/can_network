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
#define WAIT 13
#define INTER 14
#define OVER 15

volatile int enc_state = SOF;
volatile int enc_cnt = 0;
volatile int encstuff_cnt = 0;
volatile int save_enc_cnt = 0;
// volatile int cnt = 0;
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
volatile int snd_cnt=0;
volatile int snd_state=SENDBIT;
volatile int sample_point=0;
volatile int over_flag=0;
volatile int erro_flag=0;
const uint16_t crc_polinomial = 0x4599;
bitset <15> crc_seq;
bitset <15> crc_check;
uint16_t crc_convert = 0;
int crc_next = 0;
unsigned long long int crc_count = 0;

// bitset <13>enc_frame;
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
void encoder_mws(int);
void snd_msg();
void bit_stuf();
void calculate_crc(int);
void printFrame(int k, bitset<250> bs);
int main() {
	// int cnt = 0;
	readNstore();
	// cout << "ID_A: " << hex << buf_id.to_ulong() << endl;
	// cout << "ID_B: " << hex << buf_id2.to_ulong() << endl;
	// cout << "DLC: " << hex << buf_dlc.to_ulong() << endl;
	// cout << "Data: " << hex << buf_data.to_ullong() << endl;
	cout << "DLC: " << dlc << endl;
	encoder_mws(0);
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
	cout << buf_id << endl;
	buf_id2 = tmp_id_b;
	buf_dlc = tmp_dlc;
	buf_data = tmp_msg;
}

void printFrame(int k, bitset<250> bs) {
	for(int i = 0; i < k; i++) {
		cout << bs[i];
	}
	cout << endl;
}

void encoder_mws(int cnt = 0){
	if(erro_flag==1){
		enc_state=ERRO;
		erro_flag=0;
		enc_cnt=0;
		encstuff_cnt=0;
		cnt=0;
	}
	if(over_flag==1){
		enc_state=OVER;
		over_flag==0;
		enc_cnt=0;
		encstuff_cnt=0;
		cnt=0;
	}
	
	while(frame_build == 0){
		// cout << "Encoder while " << endl;
		
		// cout <<"Saindo bit stuff" << endl;
		switch(enc_state){
			
			case SOF:
					enc_frame[enc_cnt] = 0;
					calculate_crc(enc_cnt);
					enc_cnt++;
					enc_stuffframe[encstuff_cnt] = 0;
					bit_stuf();
					encstuff_cnt++;
					enc_state = ID;
					cnt = 10;
					// cout << enc_frame << endl;
					// cout <<"Apos SOF" << endl;
					// cout << "SOF indo para ID" << endl;
				break;
			
			case ID:
				
				if(cnt >= 0){
					// cout << enc_frame << endl;
					enc_frame[enc_cnt] = buf_id[cnt];
					calculate_crc(enc_cnt);
					enc_stuffframe[encstuff_cnt] = buf_id[cnt];
					bit_stuf();
					// cout << "i: " << i << endl;
					// cout << "enc_cnt: " << enc_cnt << endl;
					enc_cnt++;
					encstuff_cnt++;
					cnt--;
				} else {
					// exit(1);
					cnt = 0;
					printFrame(enc_cnt, enc_frame);
					if(ide == 1){
						enc_frame[enc_cnt] = 1;//seting SRR
						calculate_crc(enc_cnt);
						enc_stuffframe[encstuff_cnt]= 1;
						bit_stuf();
						enc_cnt++;
						encstuff_cnt++;
						enc_state = IDE;
					}else{
						enc_state = RTR;
					}
				}
					
				break;
			
			case ID2:
			
				if(cnt >= 0){
					enc_frame[enc_cnt] = buf_id2[cnt];
					calculate_crc(enc_cnt);
					enc_stuffframe[encstuff_cnt] = buf_id2[cnt];
					bit_stuf();
					enc_cnt++;
					encstuff_cnt++;
					cnt--;
				}else{
					cnt = 0;
					enc_state=RTR;
				}
				
				break;
			
			case RTR:
				if(rtr == 0){
					enc_frame[enc_cnt] = 0;
					calculate_crc(enc_cnt);
					enc_stuffframe[encstuff_cnt] = 0;
					bit_stuf();
				}else{
					enc_frame[enc_cnt] = 1;
					calculate_crc(enc_cnt);
					enc_stuffframe[encstuff_cnt] = 1;
					bit_stuf();
				}
				enc_cnt++;
				encstuff_cnt++;
				if(ide == 0){
					enc_state = IDE;
				}else{
					enc_state = CONTROL_RESBIT;	
				}
				break;
			
			case IDE:
				enc_frame[enc_cnt] = ide;						//grava o IDE
				calculate_crc(enc_cnt);
				enc_stuffframe[encstuff_cnt] = ide;
				bit_stuf();
				enc_cnt++;
				encstuff_cnt++;
				if(ide == 0){
					enc_state = CONTROL_RESBIT;
				}else{
					enc_state = ID2;
					cnt = 17;	
				}
			
			case CONTROL_RESBIT:
				if(ide == 0){
					enc_frame[enc_cnt] = 0;		//caso normal 1 bits reservado
					calculate_crc(enc_cnt);
					enc_stuffframe[encstuff_cnt] = 0;
					bit_stuf();
					enc_cnt++;
					encstuff_cnt++;
					enc_state = CONTROL;
					cnt = 3;
				} else {							//caso extendido 2 bits reservados
					if(cnt < 2){
						enc_frame[enc_cnt] = 1;
						calculate_crc(enc_cnt);
						enc_stuffframe[encstuff_cnt] = 1;
						bit_stuf();
						cnt++;
						enc_cnt++;
						encstuff_cnt++;
					}else{
						cnt = 3;
						enc_state = CONTROL;
					}
				}
			case CONTROL:
				if(cnt >= 0){
					enc_frame[enc_cnt] = buf_dlc[cnt]; //grava o dlc em bin?rio
					calculate_crc(enc_cnt);
					enc_stuffframe[encstuff_cnt] = buf_dlc[cnt];
					bit_stuf();
					cnt--;
					enc_cnt++;
					encstuff_cnt++;
					
				}else{
					cnt = dlc*8 - 1;
					enc_state = DATA;
				}
				
				break;
				
			case DATA:
				if(rtr == 0){
					if(cnt >= 0){		
						enc_frame[enc_cnt] = buf_data[cnt];
						calculate_crc(enc_cnt);
						enc_stuffframe[encstuff_cnt] = buf_data[cnt];
						bit_stuf();
						cnt--;
						enc_cnt++;
						encstuff_cnt++;

						
						// cout <<"Data" << endl;
					}else{
						cnt = 14;
						enc_state = CRC;
						cout << "CRC calculado: "<< buf_crc << endl;
					}
					
				
				}else{
					enc_state=CRC;
					cnt = 14;
				}
				
				break;	
			
			case CRC:
					if(cnt >= 0){	
						enc_frame[enc_cnt] = buf_crc[cnt];
						enc_stuffframe[encstuff_cnt] = buf_crc[cnt];
						bit_stuf();
						cnt--;
						enc_cnt++;
						encstuff_cnt++;
					}else{
						cnt = 0;
						enc_state = CRC_D;
					}
				
					
				break;
			
			case CRC_D:
				enc_frame[enc_cnt] = 1;		//crc delimiter must be 1
				enc_stuffframe[encstuff_cnt] = 1;
				enc_cnt++;	
				encstuff_cnt++;
				enc_state = ACK;
				break;
			case ACK:
				
				if(cnt < 2){
					enc_frame[enc_cnt] = 1;			//ack transmiter must be 1
					enc_stuffframe[encstuff_cnt] = 1;
					cnt++;
					enc_cnt++;
					encstuff_cnt++;
				}else{
					cnt = 0;
					enc_state=END;
				}

				break;
			
			case END:
				
				if(cnt < 7){
					enc_frame[enc_cnt] = 1;
					enc_stuffframe[encstuff_cnt] = 1;
					cnt++;
					enc_cnt++;
					encstuff_cnt++;
				} else {
					
					cnt = 0;
					enc_state=INTER;	
				}			
				
				break;
			
			case INTER:
				if(cnt<3){
					enc_frame[enc_cnt] = 1;
					enc_stuffframe[encstuff_cnt] = 1;
					cnt++;
					enc_cnt++;
					encstuff_cnt++;
				}else{
					printFrame(enc_cnt, enc_frame);
					printFrame(encstuff_cnt, enc_stuffframe);
					cnt = 0;
					save_enc_cnt = enc_cnt;
					save_encstuff_cnt = encstuff_cnt;
					encstuff_cnt = 0;
					enc_cnt = 0;
					frame_build = 1;
					enc_state=WAIT;	
				}
			break;
		
			case ERRO:
				for(cnt=0;cnt<7;cnt++){
					enc_stuffframe[encstuff_cnt] = 1;
					encstuff_cnt++;
				}
				cnt=0;
				enc_state=INTER;	
				}
				break;	
		
			case OVER:
				for(cnt=0;cnt<7;cnt++){
					enc_stuffframe[encstuff_cnt] = 1;
					encstuff_cnt++;
				}
				cnt=0;
				enc_state=INTER;	
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
	outFile<< "Come�ando escrita" << endl;
	for(int i = 0; i < save_encstuff_cnt; i++){
	
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
	
	if( (enc_state != ACK) && (enc_state!=END) && (enc_state!=CRC_D) ){
		
		if(enc_stuffframe[encstuff_cnt] == 1) {
	            cnt_bit_1++;
	

	            if(cnt_bit_1 == 5) {
	                encstuff_cnt++;
	                enc_stuffframe[encstuff_cnt]=0;
					cnt_bit_1 = 0;
	            }

		} else {
			cnt_bit_1 = 0;
		}

		if(enc_stuffframe[encstuff_cnt] == 0) {
			cnt_bit_0++;
	
			if(cnt_bit_0 == 5) {
				encstuff_cnt++;
				enc_stuffframe[encstuff_cnt] =1;
				cnt_bit_0 = 0;
				// cout << "Bit stuffing no 0, pos: "<< dec << enc_cnt << endl;
			} 

		} else {
			cnt_bit_0 = 0;
		}
	}
		// cout << "Fim do bit stuff" << endl;
}   
