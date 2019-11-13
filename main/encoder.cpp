#include "encoder.h"

encoder::encoder(uint8_t &flag_err){
	this->erro_flag = &flag_err;
	resetStates();

	//TO-DO: Montar o frame logo de início
	// encoder_mws();
}

void encoder::resetStates() {
	for(int i = 0; i < 128; i++){
		enc_frame[i] = 0;
		enc_stuffframe[i] = 0;
	}
	sendFlag = 0;
	cnt_envio = 0;
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
	snd_cnt = 0;
	sample_point = 0;
	over_flag = 0;
	buf_crc = 0;
	crc_check = 0;
	crc_convert = 0;
	crc_next = 0;
}

void encoder::encoder_mws(int cnt = 0){
	if(*erro_flag == 1){
		enc_state = ERRO;
		*erro_flag = 0;
		enc_cnt = 0;
		encstuff_cnt = 0;
		cnt = 0;
	}
	if(over_flag == 1){
		enc_state = OVER;
		over_flag = 0;
		enc_cnt = 0;
		encstuff_cnt = 0;
		cnt = 0;
	}
	
	while(frame_build == 0){
		// cout << "Encoder while " << endl;
		
		// cout <<"Saindo bit stuff" << endl;
		switch(enc_state){
			
			case SOF:
					enc_frame[enc_cnt] = 0;
                    calculate_crc();
					enc_cnt++;
					// enc_stuffframe[encstuff_cnt] = 0;
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
					// enc_frame[enc_cnt] = bitRead();
					calculate_crc();
					// enc_stuffframe[encstuff_cnt] = buf_id[cnt];
					bit_stuf();
					// cout << "i: " << i << endl;
					// cout << "enc_cnt: " << enc_cnt << endl;
					enc_cnt++;
					encstuff_cnt++;
					cnt--;
				} else {
					// exit(1);
					cnt = 0;
					// printFrame(enc_cnt, enc_frame);
					if(ide == 1){
						enc_frame[enc_cnt] = 1;//seting SRR
						calculate_crc();
						// enc_stuffframe[encstuff_cnt]= 1;
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
					enc_frame[enc_cnt] = bitRead(buf_id_2, cnt);
					calculate_crc();
					enc_stuffframe[encstuff_cnt] = bitRead(buf_id_2, cnt);
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
					calculate_crc();
					enc_stuffframe[encstuff_cnt] = 0;
					bit_stuf();
				}else{
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
				break;
			
			case IDE:
				enc_frame[enc_cnt] = ide;						//grava o IDE
				calculate_crc();
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
					calculate_crc();
					enc_stuffframe[encstuff_cnt] = 0;
					bit_stuf();
					enc_cnt++;
					encstuff_cnt++;
					enc_state = CONTROL;
					cnt = 3;
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
					// printFrame(enc_cnt, enc_frame);
					// printFrame(encstuff_cnt, enc_stuffframe);
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

void encoder::calculate_crc() {
    crc_next =  bit_atual ^ bitRead(buf_crc, 14);
    buf_crc = buf_crc << 1;//Shift left de 1
    bitClear(buf_crc, 0);
    if(crc_next){
        buf_crc = buf_crc ^ crc_polinomial;
    }
}

uint8_t encoder::enviaBit() {
	uint8_t a = enc_stuffframe[cnt_envio];
	cnt_envio++;
	return a;
}

uint8_t encoder::canSendMsg() { //Função que retorna se pode ou não enviar mensagem, 
	return sendFlag;
}

void encoder::bit_stuf(){
	
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