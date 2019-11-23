#include <stdint.h>
#include "Arduino.h"

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define WAIT 0
#define ARBITRATION 1
#define CTRL_EXTENDED_F 2
#define DATA_FRAME 3
#define REMOTE_FRAME 4
#define CRC 5
#define END_OF_FRAME 6
#define ERROR 7
#define CTRL_BASE_F 8
#define CTRL_F 9
#define ACK 10
#define INTERMISSION 11
#define OVERLOAD_FRAME 12
#define BUS_IDLE 13
#define WAIT_INIT 14

class decoder {

    public:
        void decode_message(uint8_t bit_atual, uint8_t bit_enviado);
        void initialize();
        void printData();
        decoder(/*uint8_t &flag_err,*/ HardwareSerial* print);
        uint8_t getHS();
        uint8_t getSendFlag();
        uint8_t getErrorflag();
        uint8_t getFlagACK();
        uint8_t getResetFlag();
        uint8_t getMountFrame();

    private:

        HardwareSerial* printer;

        uint8_t check_ok = 0;

        uint8_t notPrinted = 1;

        uint8_t mount_frame = 0;

        uint8_t resetFlagCntEncoder = 0;

        uint8_t flagACK = 0;

        uint8_t send_flag;

        uint8_t hard_sync;

        uint8_t flag_erro = 0;

        uint16_t ID_A = 0;
        uint32_t ID_B = 0;
        uint8_t IDE = 0;
        uint8_t RTR = 0;
        uint8_t SRR = 0;
        uint8_t bit_12 = 0;
        uint8_t DLC = 0;
        uint8_t bit_anterior = 0;
        // uint8_t bit_atual = 0;
        uint8_t next_bit_stuff = 0;
        uint8_t flag_bit_stuff = 0;
        uint8_t cnt_bit_0 = 0;
        uint8_t cnt_bit_1 = 0;

        uint8_t decoder_state;


        uint8_t count = 0;
        uint8_t count_arbitration = 0;
        uint8_t count_ctrl_f = 0;
        uint8_t count_ctrl_base_f = 0;
        uint8_t count_ctrl_base_ext = 0;
        uint8_t count_data = 0;
        uint8_t count_remote = 0;
        uint8_t count_crc = 0;
        uint8_t count_ack = 0;
        uint8_t count_eof = 0;
        uint8_t count_ifs = 0;
        uint8_t count_overload_0 = 0;
        uint8_t count_overload_1 = 0;
        uint8_t count_error_1 = 0;
        uint8_t count_error_0 = 0;
        uint8_t count_end_bits = 0;
        uint8_t count_bus_idle = 0;

        uint16_t frame_count;

        uint16_t crc_int = 0;
        // String crc_str = "";

        uint8_t err_permission = 0;

        uint8_t need_overload_frame = 0;
        uint8_t crc_err_flag = 0;

        uint64_t data_msg = 0;
        unsigned int buf_data[15];
        uint8_t cnt_field = 0;
        uint8_t aux_cnt = 0;

        //For CRC use
        const uint16_t crc_polinomial = 0x4599;
        uint16_t crc_seq;
        uint16_t crc_check;
        uint16_t crc_convert = 0;
        uint8_t crc_next = 0;

        void decoder_ms(uint8_t bit_atual, uint8_t bit_enviado);
        void check_crc(uint8_t bit_atual);
        void check_bit_stuffing(uint8_t bit_atual);
        void resetStates();
        void checkBit(uint8_t bit_atual, uint8_t bit_enviado);

};