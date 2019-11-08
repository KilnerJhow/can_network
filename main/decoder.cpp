#include "decoder.h"

decoder::decoder() {
    resetStates();
}

void decoder::decode_message() {
  
    // bit_atual = buf[count];
    // cout << "Bit atual: " << bit_atual << endl; 
    check_bit_stuffing();
    if(!flag_bit_stuff){
        // cout << " cnt igual: " << cnt_bit_igual << endl;
        decoder_ms();
    }

    if(next_bit_stuff) {
        flag_bit_stuff = 1;
        next_bit_stuff = 0;
    } else {
        flag_bit_stuff = 0;
    }

    if(decoder_state != BUS_IDLE) count++;
}

void decoder::decoder_ms() {
    // frame = frame << 1; 
    // frame[0] = bit_atual & 1;
    frame_count++;
    // cout << frame << endl;
    switch(decoder_state) {

        case BUS_IDLE:
            // cout << "Barramento livre count: " << count << endl;
            if(bit_atual == 0 && count == 0) {
                // frame.reset();
                //hard sync
                decoder_state = ARBITRATION;
                err_permission = 1;
                check_crc();
            } 
            break;

        case ARBITRATION:
            // cout << "Arbitration" << endl;
            if(count_arbitration <= 10) {
                ID_A = ID_A << 1 | (bit_atual & 1);
                check_crc();
            }

            if(count_arbitration == 10) {
                decoder_state = CTRL_F;
                // cout << "COUNT: " << dec << count << endl;
            }
            count_arbitration++;
            break;

        case CTRL_F:

            if(count_ctrl_f == 0) {
                bit_12 = bit_atual;
                check_crc();
            }
            if(count_ctrl_f == 1) {
                IDE = bit_atual;
                check_crc();
                if(IDE == 1) decoder_state = CTRL_EXTENDED_F;
                else decoder_state = CTRL_BASE_F;
            }
            count_ctrl_f++;
            break;

        case CTRL_BASE_F: 
 
            if(count_ctrl_base_f == 0) {
                //Lê bit r0
                check_crc();
                RTR = bit_12;
                if(RTR == 0) decoder_state = DATA_FRAME;
                else decoder_state = REMOTE_FRAME;
            }
            count_ctrl_base_f++;
            break;


        case CTRL_EXTENDED_F:
            
            if(count_ctrl_base_ext <= 20) {
                check_crc();
            }

            if(count_ctrl_base_ext <= 17) {    
                // cout << "Bit lido: " << bit_atual << endl;
                ID_B = ID_B << 1 | (bit_atual & 1);            
            }
            
            if(count_ctrl_base_ext == 18){
                RTR = bit_atual;
            }

            if(count_ctrl_base_ext == 20) { //2 bits reservados
                // bitset <18> idb(ID_B);
                SRR = bit_12;
                if(RTR == 1) decoder_state = REMOTE_FRAME;
                else {
                    decoder_state = DATA_FRAME;
                }
            }
            count_ctrl_base_ext++;
            break;

        case REMOTE_FRAME:
            
            if(count_remote <= 3 ) {
                // cout << "dlc lido no bit: " << dec << count << endl;
                DLC = DLC << 1 | (bit_atual & 1);
            }

            if(count_remote == 3) { //21 do remote + 12 iniciais
                decoder_state = CRC;
            }
            count_remote++;
            break;
        
        case DATA_FRAME:
            // cout << "Entrou no data frame" << endl;
            if(count_data <= 3 ) {
                // cout << "dlc lido no bit: " << dec << count << endl;
                DLC = DLC << 1 | (bit_atual & 1);
                check_crc();

            }

            if(count_data == 3) {
                if(DLC > 8) {
                    DLC = 8;
                }
            }

            if( (count_data > 3 ) && (count_data <= (3 + DLC*8 ))) {
                
                // cout << "data lido no bit: " << dec << count << endl;
                data_msg = data_msg << 1 | (bit_atual & 1);
                check_crc();
            }

            
            if(count_data == (3 + DLC * 8)) {
                decoder_state = CRC;
            }
            count_data++;
            break;

        case CRC:

            if(count_crc <= 14) {
                crc_int = crc_int << 1 | (bit_atual & 1);
                check_crc();         
            }

            if(count_crc == 14) {
                if(crc_check != 0) {
                    crc_err_flag = 1;
                    // exit(1);
                } 
                err_permission = 0;
            }

            // cout << "depois CRC: " << crc << endl;
            if(count_crc == 15) { //conta tbm o crc delimiter
                if(bit_atual == 0) {
                    decoder_state = ERROR;
                } else {
                    // cout << "CRC delimiter ok" << endl;
                    decoder_state = ACK;
                    
                }
            }
            count_crc++;
            

            break;

        case ACK:
            if(count_ack == 0) {
                if(bit_atual != 0)
                    decoder_state = ERROR;
                
            }

            if(count_ack == 1) {    //ack delimiter
                if(crc_err_flag) {
                    decoder_state = ERROR;
                } else if(bit_atual == 0) {
                    decoder_state = ERROR;
                }
                else {
                    decoder_state = END_OF_FRAME;
                    // cout << "ACK delimiter ok" << endl;
                }
            }

            count_ack++;
            break;

        case END_OF_FRAME:

            if(count_eof == 6) {
                // cout << "Ate EOF" << endl;
                if(need_overload_frame) decoder_state = OVERLOAD_FRAME;
                else {
                    decoder_state = INTERMISSION;
                }
            }
            if(bit_atual == 0) {
                decoder_state = ERROR;
            }
            count_eof++;
            
            break;
        
        case INTERMISSION:
            // cout << "Intermission" << endl;
            if(bit_atual == 0 && count_ifs <= 1) {
                //diz ao encoder para escrever 6 0
                decoder_state = OVERLOAD_FRAME;
                // cout << "count ifs: " << count_ifs << endl;
            }

            if(count_ifs == 2) {
                // cout << "Interframe space count: " << count_ifs << endl;
                decoder_state = BUS_IDLE;
                resetStates();
                // aux = -1;
            } else count_ifs++;

            break;

        case OVERLOAD_FRAME:
            if(count_overload_0 == 4) {
                //diz ao encoder para escrever 6 bits dominantes
            }

            if(count_overload_1 == 7) {   //espera ler 8 bits um do barramento
                decoder_state = WAIT;

                // aux = -1;
            }

            if(bit_atual == 1) {
                count_overload_1++;
            } else {
                count_overload_0++;
            }
            break;
        
        case ERROR:
            // escreve 6 bits 0 e depois 8 bits 1, não se importa com a leitura
            //após a transmissão de 6 bits 1 o encoder envia bits 1 e espera a leitura deles
            if(count_error_0 == 5){
            }

            if(count_error_1 == 7) {  //após a transmissão de 1 bit recessivo, ele conta mais 7 bits recessivos
                decoder_state = WAIT;   //após o frame de erro, pode vir frames de overload
                // aux = -1;
            }
            if(bit_atual == 1) count_error_1++;
            else count_error_0++;

            break;

        case WAIT:
            //precisamos esperar 11 bits recessivos para saber que o barramento está em idle 
            //Esse estado sempre vem do erro ou do overload, logo 8 bits recessivos foram contados anteriormente
            //Precisamos contar apenas mais 3 bits recessivos para o barramento ficar em idle
            //Caso percebamos um bit 0 após os 8 bits recessivos, isso significa que é um overload frame
            if(count_bus_idle == 10) {
                decoder_state = BUS_IDLE;
                resetStates();
            }

            if(bit_atual == 1) {
                count_bus_idle++;
            } else {
                decoder_state = OVERLOAD_FRAME;
            }
            
            if(bit_atual == 0) decoder_state = OVERLOAD_FRAME; 

            break;
            
    }
}

void decoder::check_crc() {
    crc_next =  bit_atual ^ bitRead(crc_check, 14);
    crc_check = crc_check << 1;//Shift left de 1
    bitClear(crc_check, 0);
    if(crc_next){
        crc_check = crc_check ^ crc_polinomial;
    }
}

void decoder::check_bit_stuffing() {
   
    if(err_permission) {
        if(bit_atual == 1) {
            cnt_bit_1++;

            if(cnt_bit_1 == 5) {
                next_bit_stuff = 1;
            }

            if(cnt_bit_1 == 6) {
                decoder_state = ERROR;
                err_permission = 0;
            }

        } else {
            cnt_bit_1 = 0;
        }

        if(bit_atual == 0) {
            cnt_bit_0++;

            if(cnt_bit_0 == 5) {
                next_bit_stuff = 1;
            }

            if(cnt_bit_0 == 6) {
                decoder_state = ERROR;
                err_permission = 0;
            }

        } else {
            cnt_bit_0 = 0;
        }
    }
}

void decoder::resetStates() {

    count = 0;
    count_arbitration = 0;
    count_ctrl_f = 0;
    count_ctrl_base_f = 0;
    count_ctrl_base_ext = 0;
    count_data = 0;
    count_remote = 0;
    count_crc = 0;
    count_ack = 0;
    count_eof = 0;
    count_ifs = 0;
    count_overload_0 = 0;
    count_overload_1 = 0;
    count_error_1 = 0;
    count_error_0 = 0;
    count_end_bits = 0;
    count_bus_idle = 0;

    ID_A = 0;
    ID_B = 0;
    IDE = 0;
    RTR = 0;
    SRR = 0;
    bit_12 = 0;
    DLC = 0;
    bit_anterior = 0;
    bit_atual = 0;
    next_bit_stuff = 0;
    flag_bit_stuff = 0;
    cnt_bit_0 = 0;
    cnt_bit_1 = 0;

    data_msg = 0;

    err_permission = 0;

    need_overload_frame = 0;
    crc_err_flag = 0;

    // frame.reset();
    frame_count = 0;
    crc_check = 0;
    // crc_seq.reset();
    // crc_check.reset();

    // cout << "Estados resetados!" << endl;
}