#include "decoder.h"

decoder::decoder(/*uint8_t &flag_erro,*/ HardwareSerial* print) {
    printer = print;
    // this->flag_erro = &flag_erro;
    resetStates();
    decoder_state = WAIT_INIT;
}

void decoder::initialize(){
    printer->println("Decoder inicializado");
}

void decoder::decode_message(uint8_t bit_atual, uint8_t bit_enviado) {
  
    check_bit_stuffing(bit_atual);
    if(!flag_bit_stuff){
        decoder_ms(bit_atual, bit_enviado);
    }

    if(next_bit_stuff) {        
        
        flag_bit_stuff = 1;
        next_bit_stuff = 0;

    } else {
        flag_bit_stuff = 0;
    }

    if(decoder_state != BUS_IDLE) count++;
}

void decoder::decoder_ms(uint8_t bit_atual, uint8_t bit_enviado) {

    frame_count++;
    checkBit(bit_atual, bit_enviado);
    switch(decoder_state) {

        case BUS_IDLE:
            mount_frame = 0;
            resetFlagCntEncoder = 0;
            if(bit_atual == 0) {
                
                notPrinted = 1;
                // printer->println("Saindo do BUS_IDLE");
                decoder_state = ARBITRATION;
                err_permission = 1;
                hard_sync = 1;  //Passar hard sync para o bit_timing
                check_crc(bit_atual);
                notPrinted = 1;
            } 
            
            break;

        case ARBITRATION:
            hard_sync = 0;
            if(count_arbitration <= 10) {
                ID_A = ID_A << 1 | (bit_atual & 1);
                
                if(bit_atual != bit_enviado && win){
                    printer->println("Arbitracao perdida");
                    send_flag = 0;
                    win = 0;  
                } 
                check_crc(bit_atual);
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
                
                // printer->print("Bit 12: ");
                // printer->println(bit_12);

                if(bit_atual != bit_enviado && win){
                    send_flag = 0;
                    win = 0;  
                } 
                // check_ok = 1;
                check_crc(bit_atual);
            }
            if(count_ctrl_f == 1) {
                IDE = bit_atual;
                
                // printer->print("IDE: ");
                // printer->println(IDE);

                check_crc(bit_atual);
                if(IDE == 1) decoder_state = CTRL_EXTENDED_F;
                else decoder_state = CTRL_BASE_F;
            }
            count_ctrl_f++;
            break;

        case CTRL_BASE_F: 
 
            if(count_ctrl_base_f == 0) { //Lê bit r0
                
                check_crc(bit_atual);
                RTR = bit_12;

                // printer->print("RTR: ");
                // printer->println(RTR);

                if(RTR == 0) decoder_state = DATA_FRAME;
                else decoder_state = REMOTE_FRAME;
            }
            count_ctrl_base_f++;
            break;


        case CTRL_EXTENDED_F:
            
            if(count_ctrl_base_ext <= 20) {
                check_crc(bit_atual);
            }

            if(count_ctrl_base_ext <= 17) {    
                // cout << "Bit lido: " << bit_atual << endl;
                ID_B = ID_B << 1 | (bit_atual & 1);            
                if(bit_atual != bit_enviado && win){
                    send_flag = 0;
                    win = 0;  
                } 
            }
            
            // if(count_ctrl_base_ext == 17){
                // printer->print("ID_B: ");
                // printer->println(ID_B, HEX);
            // }

            if(count_ctrl_base_ext == 18){
                RTR = bit_atual;
                // printer->print("RTR: ");
                // printer->println(RTR);
                if(bit_atual != bit_enviado && win){
                    send_flag = 0;
                    win = 0;  
                } 
            }

            if(count_ctrl_base_ext == 20) { //2 bits reservados
                
                SRR = bit_12;
                // printer->print("SRR: ");
                // printer->println(SRR);
                if(RTR == 1) decoder_state = REMOTE_FRAME;
                else {
                    decoder_state = DATA_FRAME;
                }
                check_ok = 1;
            }
            count_ctrl_base_ext++;
            break;

        case REMOTE_FRAME:
            
            if(count_remote <= 3 ) {
                // cout << "dlc lido no bit: " << dec << count << endl;
                DLC = DLC << 1 | (bit_atual & 1);
                check_crc(bit_atual);
            }

            if(count_remote == 3) { //21 do remote + 12 iniciais
                decoder_state = CRC;
                // printer->print("CRC: ");
            }
            count_remote++;
            break;
        
        case DATA_FRAME:
            
            if(count_data <= 3 ) {
                // cout << "dlc lido no bit: " << dec << count << endl;
                DLC = DLC << 1 | (bit_atual & 1);
                check_crc(bit_atual);

            }

            if(count_data == 3) {
                
                // printer->print("DLC: ");
                // printer->print(DLC);
                // printer->println();

                if(DLC > 8) {
                    DLC = 8;
                }
            }

            if( (count_data > 3 ) && (count_data <= (3 + DLC*8 ))) {
                
                // cout << "data lido no bit: " << dec << count << endl;
                data_msg = data_msg << 1 | (bit_atual & 1);
                // printer->print("Count aux: ");
                // printer->println(aux_cnt);
                
                if(aux_cnt == 7) {
                    // int a = data_msg & 255;
                    // printer->print("A: ");
                    // printer->println(a);
                    buf_data[cnt_field] = data_msg & 255;
                    cnt_field++;
                    aux_cnt = 0;
                }
                else aux_cnt++;
                
                check_crc(bit_atual);
            }

            
            if(count_data == (3 + DLC * 8)) {
                // printer->print("Data: ");
                // for(int i = 0; i < cnt_field; i++) {
                    // printer->print(buf_data[i], HEX);
                // }
                // printer->println();
                // printData();
                decoder_state = CRC;
                // printer->print("CRC: ");
            }
            count_data++;
            break;

        case CRC:

            if(count_crc <= 14) {
                crc_int = crc_int << 1 | (bit_atual & 1);
                // printer->print(bit_atual);
                check_crc(bit_atual);         
            }

            if(count_crc == 14) {
                // printer->println();
                // printer->println(crc_str);
                // printer->print("CRC check: ");
                // printer->println(crc_check);
                if(crc_check != 0) {
                    crc_err_flag = 1;
                    // exit(1);
                } 
                err_permission = 0;
            }

            // cout << "depois CRC: " << crc << endl;
            if(count_crc == 15) { //conta tbm o crc delimiter
                // printer->println("CRC delimiter");
                if(bit_atual == 0) {
                    decoder_state = ERROR;
                    flag_erro = 1;
                    send_flag = 1;
                } else {
                    // cout << "CRC delimiter ok" << endl;
                    decoder_state = ACK;
                    flagACK = 1;
                    check_ok = 0;
                    
                }
            }
            count_crc++;
            

            break;

        case ACK:
            if(count_ack == 0) {
                flagACK = 0;
                if(bit_atual != 0) {
                    // printer->println("ACK error");
                    decoder_state = ERROR;
                    flag_erro = 1;
                    send_flag = 1;
                } else {
                    check_ok = 1;
                }
                // printer->println("ACK slot");
            }

            if(count_ack == 1) {    //ack delimiter
                // printer->println("ACK delimiter");
                if(crc_err_flag) {
                    
                    // printer->println("CRC error");
                    decoder_state = ERROR;
                    flag_erro = 1;
                    send_flag = 1;

                } else if(bit_atual == 0) {
                    
                    // printer->println("Erro de bit atual no ack delimiter");
                    decoder_state = ERROR;
                    flag_erro = 1;
                    send_flag = 1;

                }
                else {
                    // printer->println("TO EOF");
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
                    // printer->println("EOF");
                    decoder_state = INTERMISSION;
                }
            }
            if(bit_atual == 0) {
                decoder_state = ERROR;
                flag_erro = 1;
                send_flag = 1;
            }
            count_eof++;
            
            break;
        
        case INTERMISSION:
            if(bit_atual == 0 && count_ifs <= 1) {
                //diz ao encoder para escrever 6 0
                decoder_state = OVERLOAD_FRAME;
            }

            if(count_ifs == 2) {
                // cout << "Interframe space count: " << count_ifs << endl;
                // printer->println("Intermission");
                // printer->println("Indo para o BUS IDLE");
                
                decoder_state = BUS_IDLE;
                resetStates();
                err_permission = 0;
                resetFlagCntEncoder = 1;

                // printer->println("Frame recebido/enviado");

                // printer->print("CNT bit 0 no intermission: ");
                // printer->println(cnt_bit_0);
                // printer->print("CNT bit 1: ");
                // printer->println(cnt_bit_1);


                send_flag = 1;
            } else count_ifs++;

            break;

        case OVERLOAD_FRAME:

            check_ok = 0;
            flag_erro = 0;
            if(count_overload_0 == 5) {
                // printer->println("6 bits overload");
                //diz ao encoder para escrever 6 bits dominantes
            }

            if(count_overload_1 == 7) {   //espera ler 8 bits um do barramento
                decoder_state = WAIT;
                // printer->println("Saindo do overload");
                mount_frame = 1;
                flag_erro = 0;
                send_flag = 0;
                resetFlagCntEncoder = 1;
                // aux = -1;
            }

            if((count_overload_0 >= 5) && (bit_atual == 1)) {
                count_overload_1++;
            }
            else if(bit_atual == 0 && cnt_bit_0 < 5) {
                count_overload_0++;
            }
            break;
        
        case ERROR:
      
            flag_erro = 0;
            check_ok = 0;

            // escreve 6 bits 0 e depois 8 bits 1, não se importa com a leitura
            //após a transmissão de 6 bits 1 o encoder envia bits 1 e espera a leitura deles
            if(count_error_0 == 5){
                // printer->println("6 bits 0 recebidos no estado erro do decoder");
            }

            if(count_error_1 == 7) {  //após a transmissão de 1 bit recessivo, ele conta mais 7 bits recessivos
                // printer->println("8 bits 1 recebidos no estado erro do decoder");
                decoder_state = WAIT;   //após o frame de erro, pode vir frames de overload
                // printer->println("Flag de mount frame setada.");
                mount_frame = 1;
                flag_erro = 0;
                send_flag = 0;
                resetFlagCntEncoder = 1;
            }
            
            
            
            if((count_error_0 >= 5) && (bit_atual == 1)) {
                count_error_1++;
            }
            if(bit_atual == 0 && count_error_0 <= 5) {
                count_error_0++;
            }

            break;

        case WAIT:
            
            //Esse estado sempre vem do erro ou do overload, logo 8 bits recessivos foram contados anteriormente
            //Precisamos contar apenas mais 3 bits recessivos para o barramento ficar em idle
            //Caso percebamos um bit 0 após os 8 bits recessivos, isso significa que é um overload frame
            flag_erro = 0;
            mount_frame = 0;
            if(count_bus_idle == 2) {
                mount_frame = 0;
                decoder_state = BUS_IDLE;
                // printer->println("Saindo do WAIT");
                resetStates();
                flag_erro = 0;
                resetFlagCntEncoder = 1;
                send_flag = 1; //Habilita o encoder a enviar mensagens
                
                                    
            }

            if(bit_atual == 1) {
                count_bus_idle++;
            } else {
                decoder_state = OVERLOAD_FRAME;
            }
            
            if(bit_atual == 0) decoder_state = OVERLOAD_FRAME; 

            break;
        
        case WAIT_INIT:
            //precisamos esperar 11 bits recessivos para saber que o barramento está em idle 
            flag_erro = 0;
            mount_frame = 0;
            if(count_bus_idle == 10) {
                mount_frame = 0;
                decoder_state = BUS_IDLE;
                // printer->println("Saindo do WAIT INIT");
                resetStates();
                flag_erro = 0;
                resetFlagCntEncoder = 1;
                send_flag = 1; //Habilita o encoder a enviar mensagens
                
                                    
            }
            count_bus_idle++;            
    }
}

void decoder::printData(){
    if(decoder_state == INTERMISSION && notPrinted) {
        printer->println("\nRecebido: ");
        printer->print("ID_A: ");
        printer->print(ID_A, HEX);
        printer->print(" - RTR: ");
        printer->print(RTR);
        printer->print(" - IDE: ");
        printer->print(IDE);

        if(IDE == 1) {
            printer->print(" - ID_B: ");
            printer->print(ID_B, HEX);
        }

        printer->print(" - DLC: ");
        printer->print(DLC);
        printer->print(" - Data: ");
        if(RTR == 0){
            for(int i = 0; i < cnt_field; i++) {
                printer->print(buf_data[i], HEX);
            }
            printer->println();
        } else {
            printer->println("Vazio");
        }
        notPrinted = 0;
    }

}

void decoder::check_crc(uint8_t bit_atual) {
    crc_next =  bit_atual ^ bitRead(crc_check, 14);
    crc_check = crc_check << 1;//Shift left de 1
    bitClear(crc_check, 0);
    bitClear(crc_check, 15);
    if(crc_next){
        crc_check = crc_check ^ crc_polinomial;
    }
}

void decoder::check_bit_stuffing(uint8_t bit_atual) {
   
    if(err_permission) {
        if(bit_atual == 1) {
            cnt_bit_1++;

            if(cnt_bit_1 == 5) {
                next_bit_stuff = 1;
            }

            if(cnt_bit_1 == 6) {
                printer->println("Bit stuff 1 error");
                decoder_state = ERROR;
                err_permission = 0;
                flag_erro = 1;
                send_flag = 1;
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
                printer->println("Bit stuff 0 error");
                decoder_state = ERROR;
                err_permission = 0;
                flag_erro = 1;
                send_flag = 1;
            }

        } else {
            cnt_bit_0 = 0;
        }
    }
}

void decoder::resetStates() {

    // crc_str = "";
    
    win = 1;

    check_ok = 0;

    mount_frame = 0;

    resetFlagCntEncoder = 0;

    flagACK = 0;

    send_flag = 0;

    hard_sync = 0;

    flag_erro = 0;

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


    next_bit_stuff = 0;
    flag_bit_stuff = 0;
    cnt_bit_0 = 0;
    cnt_bit_1 = 0;

    data_msg = 0;

    err_permission = 0;

    need_overload_frame = 0;
    crc_err_flag = 0;

    frame_count = 0;
    crc_check = 0;

    for(int i = 0; i < 15; i++) {
        buf_data[i] = 0;
    }
    cnt_field = 0;
    aux_cnt = 0;
}

uint8_t decoder::getHS() {
    return this->hard_sync;
}

uint8_t decoder::getSendFlag() {
    return send_flag;
}

uint8_t decoder::getErrorflag(){
    return flag_erro;
}

uint8_t decoder::getFlagACK(){
    return flagACK;
}

uint8_t decoder::getResetFlag(){
    return resetFlagCntEncoder;
}

uint8_t decoder::getMountFrame(){
    return mount_frame;
}

void decoder::checkBit(uint8_t bit_atual, uint8_t bit_enviado){
    if(check_ok && send_flag){
        if(bit_atual != bit_enviado){
            printer->println("Erro de bit enviado");
            decoder_state = ERROR;
            flag_erro = 1;
            send_flag = 1;
        }

    }
}