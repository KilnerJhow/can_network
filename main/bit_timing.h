#include <stdint.h>

#define SYNC 0
#define SEG_1 1
#define SEG_2 2

#define SJW 10

class bit_timing {
    public:
        uint8_t hard_sync;
        uint8_t soft_sync;
        uint8_t reset;
        bit_timing();
        void sample(uint8_t bit_atual);
        void resync();
        void resetStates();
        void hardSync();
        void machine_state();
        void checkEdge();
        void checksync();

    private:

        uint8_t bit_atual;
        int cnt_sync;
        int cnt_seg_1;
        int resync1;
        int cnt_seg_2;
        int resync2;
        int actual_state;
        const int time_segment1  = 8;
        const int time_segment2 = 7;
        uint8_t occurr_soft_sync;
        uint8_t window2;
        uint8_t writing_point;
        uint8_t sampling_point;
        uint8_t past;
        uint8_t edge;
        uint8_t actual;

};
