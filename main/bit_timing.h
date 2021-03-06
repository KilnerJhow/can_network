#include <stdint.h>
#include "Arduino.h"

#define SYNC 0
#define SEG_1 1
#define SEG_2 2

#define SJW 4

class bit_timing {
    public:

        bit_timing(HardwareSerial *print);
        void initialize();
        void machine_state(uint8_t bit_atual);
        int sampling_point();
        int writing_point();
        void setHS(uint8_t hard_sync);
        void printFlag();
        void setEdge();

    private:

        int print_hard_sync = 0;

        void hardSync();
        void checksync();
        // void checkEdge();
        void resetStates();
        void resync();
        HardwareSerial* printer;

        uint8_t hard_sync;
        uint8_t reset;
        uint8_t soft_sync;
        int cnt_sync;
        int cnt_seg_1;
        int resync1;
        int cnt_seg_2;
        int resync2;
        int actual_state;
        const int time_segment1  = 5;
        const int time_segment2 = 5;
        uint8_t occurr_soft_sync;
        uint8_t occurr_hard_sync;
        uint8_t window2;
        uint8_t writing_point_;
        uint8_t sampling_point_;
        uint8_t past;
        uint8_t edge;
        uint8_t actual;

};
