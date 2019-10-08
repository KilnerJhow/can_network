#define TQ_SIZE 6.25 // Tamanho de cada Tq, em ms
#define QT_TQ   16   // Quantidade de Tqs por bit
#define COUNT_TQ 50000 //Tamanho do contador para cada Tq levando em conta os 16 MHz do Arduino.
#define BIT_SIZE 100 // Tamanho total do bit, em ms

#define SYNC 0
#define SEG_1 1
#define SEG_2 2
#define WINDOW2 3

const int time_segment1 = 7;
const int time_segment2 = 7;

class bit_timing {
    private:
        uint16_t cnt_sync;
        uint32_t cnt_seg_1;
        uint32_t resync1;
        uint32_t cnt_seg_2;
        uint32_t resync2;

        byte window2;
        byte actual_state;
        void machine_state();
    public:
        byte hard_sync;
        byte reset;
        bit_timing();
};
//Constructor
bit_timing::bit_timing() {
    cnt_sync = 0;
    cnt_seg_1 = 0;
    resync1 = 0;
    cnt_seg_2 = 0;
    resync2 = 0;
    actual_state = SYNC;
    window2 = 0;
    machine_state();
}

void bit_timing::machine_state(){

    switch (actual_state) {

        case SYNC:
            if(cnt_sync < COUNT_TQ) cnt_sync ++;
            else actual_state = SEG_1;
            break;

        case SEG_1:
            if(cnt_seg_1 < ((time_segment1 * COUNT_TQ) + resync1*COUNT_TQ)) cnt_seg_1 ++;
            else actual_state = SEG_2;
            break;

        case SEG_2:
            if(cnt_seg_2 < ((time_segment2 * COUNT_TQ))) cnt_seg_2++;
            else actual_state = SYNC;
            break;

        case WINDOW2:
            if(cnt_seg_2 < ((time_segment2 * COUNT_TQ) - resync2*COUNT_TQ)) cnt_seg_2++;
            else actual_state = SEG_1;
            break;

        default:
            break;  

    }
}
