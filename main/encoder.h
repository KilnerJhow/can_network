#include <stdint.h>
#include "Arduino.h"

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))


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
#define ERRO 16


class encoder{

    public:
        encoder(HardwareSerial *print);
        void encoder_mws(int cnt);
        void initialize();
        uint8_t enviaBit();
        uint8_t canSendMsg();
        void printDataToSend();
        void setSendFlag(uint8_t flag);
        void setErrorFlag(uint8_t flag_err);
        void setResetFlag(uint8_t flag);
        void setMountFrame(uint8_t flag);
        void printFrameNoStuff();
        void printFrameStuff();

    private:

        HardwareSerial* printer;

        void calculate_crc();
        void bit_stuf();
        void resetStates();
        
        

        uint8_t mount_frame = 0;

        uint8_t sendFlag;

        uint8_t cnt_envio;
        uint8_t cnt_err_envio;
        uint16_t buf_id_1 = 0x0672; //0x0673
        uint32_t buf_id_2 = 0x3007A;
        uint8_t rtr = 0;
        uint8_t ide = 0;
        // uint8_t dlc = 0;
        uint8_t buf_dlc = 8;
        uint64_t buf_data = 0xAAAAAAAAAAAAAAAA; 
        // uint64_t buf_data = 0; 

        int enc_frame[150];
        int enc_stuffframe[250];
        int enc_frame_err[14];
        uint8_t enc_state = SOF;
        uint16_t enc_cnt = 0;
        uint16_t encstuff_cnt = 0;
        uint16_t save_enc_cnt = 0;
        uint8_t actual_bit;
        uint8_t past_bit;
        uint8_t cnt_stuff = 0;
        uint8_t cnt_bit_1 = 0;
        uint8_t cnt_bit_0 = 0;
        uint8_t bit_atual = 0;
        uint16_t save_encstuff_cnt = 0;
        uint16_t frame_build = 0;


        uint8_t sample_point = 0;
        uint8_t over_flag = 0;
        uint8_t erro_flag;

        const uint16_t crc_polinomial = 0x4599;
        uint16_t buf_crc = 0;
        uint16_t crc_check = 0;
        uint16_t crc_convert = 0;
        uint8_t crc_next = 0;

};