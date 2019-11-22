#include "encoder.h"

encoder::encoder(/*uint8_t &flag_err,*/HardwareSerial* print){
	printer = print;
	for(int i = 0; i < 14; i++){
		if(i < 6) enc_frame_err[i] = 0;
		else enc_frame_err[i] = 1;
	}

	// this->erro_flag = &flag_err;
	resetStates();

	//TO-DO: Montar o frame logo de início
	// encoder_mws();
}

void encoder::initialize(){
	printer->println("Encoder inicializado");
	// for(int i = 0; i < 14; i++){
	// 	printer->print(enc_frame_err[i]);
	// }
	// printer->println();
}

void encoder::resetStates() {
	
	sendFlag = 0;
	cnt_envio = 0;
	cnt_err_envio = 0;
	enc_state = SOF;
	enc_cnt = 0;
	encstuff_cnt = 0;
	save_enc_cnt = 0;
	actual_bit = 0;
	past_bit = 0;
	cnt_stuff = 0;
	cnt_bit_1 = 0;
	cnt_bit_0 = 0;
	bit_atual = 0;
	save_encstuff_cnt = 0;
	frame_build = 0;
	sample_point = 0;
	over_flag = 0;
	erro_flag = 0;
	buf_crc = 0;
	crc_check = 0;
	crc_convert = 0;
	crc_next = 0;
}

void encoder::encoder_mws(int cnt = 0){
	/*if(erro_flag == 1){
		// printer->println("Entrando no estado de erro no encoder");
		enc_state = ERRO;
		erro_flag = 0;
		enc_cnt = 0;
		encstuff_cnt = 0;
		cnt = 0;
		frame_build = 0;
	} 
	if(over_flag == 1){
		enc_state = OVER;
		over_flag = 0;
		enc_cnt = 0;
		encstuff_cnt = 0;
		cnt = 0;
	}*/
	while(frame_build == 0){
		// cout << "Encoder while " << endl;
		
		// cout <<"Saindo bit stuff" << endl;
		switch(enc_state){
			
			case SOF:
					// printer->println("SOF");
					enc_frame[enc_cnt] = 0;
                    calculate_crc();
					enc_cnt++;
					enc_stuffframe[encstuff_cnt] = 0;
					bit_stuf();
					encstuff_cnt++;
					enc_state = ID;
					cnt = 10;
				break;
			
			case ID:
				
				if(cnt >= 0){

					enc_frame[enc_cnt] = bitRead(buf_id_1, cnt);
					enc_stuffframe[encstuff_cnt] = bitRead(buf_id_1, cnt);
					calculate_crc();
					bit_stuf();
					enc_cnt++;
					encstuff_cnt++;
					cnt--;

				} else {
					cnt = 0;

					if(ide == 1){
						
						printer->println("IDE = 1");
						enc_frame[enc_cnt] = 1;//seting SRR
						enc_stuffframe[encstuff_cnt] = 1;

						calculate_crc();
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
				// printer->println("ID2");
				if(cnt >= 0){
					enc_frame[enc_cnt] = bitRead(buf_id_2, cnt);
					enc_stuffframe[encstuff_cnt] = bitRead(buf_id_2, cnt);
					
					for(int i = 0; i < encstuff_cnt; i++){
						printer->print(enc_stuffframe[i]);
					}
					printer->println();	
					bit_stuf();
					calculate_crc();

					enc_cnt++;
					encstuff_cnt++;
					cnt--;

				} else {

					// printer->println("Frame no fim do ID2");
					// printFrameStuff();
					cnt = 0;
					enc_state = RTR;

					printer->print("Frame no fim do ID2: ");
					// printer->print("\t\t");
					for(int i = 0; i < encstuff_cnt; i++){
						printer->print(enc_stuffframe[i]);
					}
					printer->println();

				}
				
				break;
			
			case RTR:
			
				if(rtr == 0){
					enc_frame[enc_cnt] = 0;
					enc_stuffframe[encstuff_cnt] = 0; 
					calculate_crc();
					bit_stuf();
				} else {
					enc_frame[enc_cnt] = 1;
					calculate_crc();
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

				// printer->print("Frame no fim do rtr: ");
				// printer->print("\t\t");
				// for(int i = 0; i < enc_cnt; i++){
				// 	printer->print(enc_stuffframe[i]);
				// }
				// printer->println();
				
				break;
			
			case IDE:
				enc_frame[enc_cnt] = ide;						//grava o IDE
				enc_stuffframe[encstuff_cnt] = ide;
				calculate_crc();
				bit_stuf();
				enc_cnt++;
				encstuff_cnt++;
				
				if(ide == 0){
				
					enc_state = CONTROL_RESBIT;

				}else{
				
					printer->println("INDO AO ID2");
					enc_state = ID2;
					cnt = 17;	

				}

				break;
			
			case CONTROL_RESBIT:
				if(ide == 0){
					enc_frame[enc_cnt] = 0;		//caso normal 1 bits reservado
					calculate_crc();
					enc_stuffframe[encstuff_cnt] = 0;
					bit_stuf();
					enc_cnt++;
					encstuff_cnt++;
					enc_state = CONTROL;
					cnt = 3;

					// printer->print("Frame no fim do control: ");
					// printer->print("\t");
					// for(int i = 0; i < enc_cnt; i++){
					// 	printer->print(enc_stuffframe[i]);
					// }
					// printer->println();

				} else {							//caso extendido 2 bits reservados
					if(cnt < 2){
						enc_frame[enc_cnt] = 1;
						calculate_crc();
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
					enc_frame[enc_cnt] = bitRead(buf_dlc, cnt); //grava o dlc em bin?rio
					calculate_crc();
					enc_stuffframe[encstuff_cnt] = bitRead(buf_dlc, cnt);
					bit_stuf();
					cnt--;
					enc_cnt++;
					encstuff_cnt++;

					// printer->print("Frame no fim do dlc: ");
					// printer->print("\t\t");
					// for(int i = 0; i < enc_cnt; i++){
					// 	printer->print(enc_stuffframe[i]);
					// }
					// printer->println();
					
				}else{
					cnt = dlc*8 - 1;
					enc_state = DATA;
				}
				
				break;
				
			case DATA:
				if(rtr == 0){
					if(cnt >= 0){		
						enc_frame[enc_cnt] = bitRead(buf_data, cnt);
						calculate_crc();
						enc_stuffframe[encstuff_cnt] = bitRead(buf_data, cnt);
						bit_stuf();
						cnt--;
						enc_cnt++;
						encstuff_cnt++;

						
						// cout <<"Data" << endl;
					}else{
						// printer->print("Frame no fim do data: ");
						// printer->print("\t\t");
						// for(int i = 0; i < enc_cnt; i++){
						// 	printer->print(enc_stuffframe[i]);
						// }
						// printer->println();
						cnt = 14;
						enc_state = CRC;
					}
					
				
				}else{
					enc_state=CRC;
					cnt = 14;
				}
				
				break;	
			
			case CRC:
					if(cnt >= 0){	
						enc_frame[enc_cnt] = bitRead(buf_crc, cnt);
						enc_stuffframe[encstuff_cnt] = bitRead(buf_crc, cnt);
						bit_stuf();
						cnt--;
						enc_cnt++;
						encstuff_cnt++;
					}else{
						// printer->println("Frame no fim do CRC: ");
						// for(int i = 0; i < enc_cnt; i++){
						// 	printer->print(enc_stuffframe[i]);
						// }
						// printer->println();
						// printer->print("CRC do encoder: ");
						// printer->println(buf_crc, BIN);
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
					cnt = 0;
					save_enc_cnt = enc_cnt;
					save_encstuff_cnt = encstuff_cnt;
					encstuff_cnt = 0;
					enc_cnt = 0;
					frame_build = 1;
					// enc_state = WAIT;

					
					// printer->println("Frame montado");

				}
				break;
		
			case ERRO:

				if(cnt < 6){
					enc_stuffframe[encstuff_cnt] = 0;
					encstuff_cnt++;
				} else if(cnt < 13) {
					enc_stuffframe[encstuff_cnt] = 1;
					encstuff_cnt++;
				} else {
					// printer->println("Frame de erro montado");
					// for(int i = 0; i < encstuff_cnt; i++) {
					// 	printer->print(enc_stuffframe[i]);
					// }
					// printer->println();
					enc_state=INTER;	
					cnt = 0;
				}

				cnt++;
				
				break;	
		
			case OVER:
				for(cnt=0;cnt<7;cnt++){
					enc_stuffframe[encstuff_cnt] = 1;
					encstuff_cnt++;
				}
				cnt=0;
				enc_state=INTER;	
				
				break;	
		}
			
	}
}

void encoder::setErrorFlag(uint8_t flag_err){
	if(flag_err) {
		erro_flag = 1;
		printer->println("Encoder flag de erro setada");
	} 
}

void encoder::setMountFrame(uint8_t flag){
	if(flag) {
		// printer->println("\nMontando frame");
		//mount_frame = 0;
		frame_build = 0;
		enc_state = SOF;
		resetStates();
		encoder_mws(0);
		// printer->println("Frame montado: ");
		// printFrameStuff();
		// printer->println();
		
	}
}

void encoder::printFrameNoStuff(){
	printer->println("Frame: ");
	for(int i = 0; i < save_enc_cnt; i++){
		printer->print(enc_frame[i]);
	}
	printer->println();
}

void encoder::printFrameStuff(){
	printer->println("Frame Stuff: ");
	for(int i = 0; i < save_encstuff_cnt; i++){
		printer->print(enc_stuffframe[i]);
	}
	printer->println();
}

void encoder::calculate_crc() {
    crc_next =  enc_frame[enc_cnt] ^ bitRead(buf_crc, 14);
    buf_crc = buf_crc << 1;//Shift left de 1
    bitClear(buf_crc, 0);
	bitClear(buf_crc, 15);
    if(crc_next){
        buf_crc = buf_crc ^ crc_polinomial;
    }
}

uint8_t encoder::enviaBit() {
	uint8_t a;
	if(erro_flag && cnt_err_envio < 14){
		a = enc_frame_err[cnt_err_envio];
		// printer-
		// printer->println(a);
		cnt_err_envio++;
		return a;
	} else if(cnt_envio < save_encstuff_cnt && !erro_flag) {
		a = enc_stuffframe[cnt_envio];
		// uint8_t a = enc_frame[cnt_envio];
		cnt_envio++;
		return a;
	}
	// printer->println("Retornando 1 a partir do enviaBit"); 
	return 1;
}

uint8_t encoder::canSendMsg() { //Função que retorna se pode ou não enviar mensagem,
	return sendFlag;
}

void encoder::bit_stuf(){
	
	if( (enc_state != ACK) && (enc_state!=END) && (enc_state!=CRC_D) ){
		
		if(enc_stuffframe[encstuff_cnt] == 1) {
			cnt_bit_1++;

			printer->print("CNT bit 1: ");
			printer->println(cnt_bit_1);
					

			if(cnt_bit_1 == 5) {
				printer->println("Adicionando 0");
				encstuff_cnt++;
				enc_stuffframe[encstuff_cnt] = 0;
				cnt_bit_1 = 0;
			}

		} else {
			cnt_bit_1 = 0;
		}

		if(enc_stuffframe[encstuff_cnt] == 0) {
			cnt_bit_0++;
			
			printer->print("CNT bit 0: ");
			printer->println(cnt_bit_0);

			if(cnt_bit_0 == 5) {
				printer->println("Adicionando 1");
				encstuff_cnt++;
				enc_stuffframe[encstuff_cnt] = 1;
				cnt_bit_1++;
				cnt_bit_0 = 0;
			} 

		} else {
			cnt_bit_0 = 0;
		}
	}
}

void encoder::setSendFlag(uint8_t flag){
	this->sendFlag = flag;
	if(sendFlag == 0) {	//Bloqueou o envio de mensagem, logo volta o contador para o ínicio do frame.
		cnt_envio = 0;
	}
}

void encoder::setResetFlag(uint8_t flag) {
	if(flag) {
		// printer->println("Contador resetado");
		cnt_envio = 0;
	}
}

void encoder::printDataToSend(){
	// int buf_data[15];
	uint8_t a = 0;
	int aux_cnt = 0;
	printer->print("ID_A: ");
	printer->print(buf_id_1, HEX);
	printer->print(" - RTR: ");
	printer->print(rtr);
	printer->print(" - IDE: ");
	printer->print(ide);

	if(ide == 1){
		printer->print(" - ID_B: ");
		printer->print(buf_id_2, HEX);
	}

	printer->print(" - DLC: ");
	printer->print(dlc);
	printer->print(" - Data: ");
	for(int i = dlc*8 - 1; i >= 0 ; i--){
		a = a << 1 | (bitRead(buf_data, i) & 1);
		if(aux_cnt == 7) {
			printer->print(a, HEX);
			aux_cnt = 0;
		}
		else aux_cnt++;
	}
	printer->println();
	
}