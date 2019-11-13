#include "bit_timing.h"

bit_timing::bit_timing(HardwareSerial *print){
    printer = print;
    resetStates();
    actual_state = SYNC;
}

void bit_timing::teste(){
    // printer->println("Hello World from bitset");
}

void bit_timing::resync() {
  
  if(soft_sync == 1) {
     switch(actual_state) {
        case SEG_1:
            resync1 = cnt_seg_1;
            if(resync1 > SJW) resync1 = SJW;
            //occurr_soft_sync = true;
            //soft_sync = 0;
            break;

        case SEG_2:
            resync2 = time_segment2 - cnt_seg_2;
            if(resync2 > SJW) resync2 = SJW;
            window2 = true;
            //soft_sync = 0;
            break;
     }
  }
  
}

void bit_timing::resetStates() {
    cnt_sync = 0;
    cnt_seg_1  = 1;
    resync1    = 0;
    cnt_seg_2  = 1;
    resync2    = 0;
    writing_point_ = 0;
    sampling_point_ = 0;
    window2 = false;
    occurr_soft_sync = false;
}

void bit_timing::hardSync() {
    if(hard_sync == 1) {
        actual_state = SEG_1;
        resetStates();
    }
}

void bit_timing::setHS(uint8_t hard_sync){
    this->hard_sync = hard_sync;
}

void bit_timing::machine_state() {

  checksync(); //verifica a necessidade de sincronização

  switch (actual_state) {
        case SYNC:
            if(cnt_sync == 1) {
              actual_state = SEG_1;
              writing_point_ = 1;
              sampling_point_ = 0;
            }
            cnt_sync++;
            break;

        case SEG_1:
            hardSync();
            if(cnt_seg_1 < (time_segment1  + resync1)) {
                writing_point_ = 0;
                sampling_point_ = 0;
            } else {
                actual_state = SEG_2;
                sampling_point_ = 1;
                writing_point_ = 0;
            }
            cnt_seg_1 ++;
            break;

        case SEG_2:
            hardSync();
            if(!window2) {
                if(cnt_seg_2 < time_segment2 ) {
                    sampling_point_ = 0;
                    writing_point_ = 0;
                }
                else {
                    actual_state = SYNC;
                    resetStates();
                }
            } else {
                if(cnt_seg_2 < (time_segment2 - resync2)){
                    sampling_point_ = 0;
                    writing_point_ = 0;
                }
                else {
                    actual_state = SEG_1;
                    resetStates();
                    writing_point_ = 1;
                }
            }
            cnt_seg_2++;
            break;

    }
}

/*void bit_timing::checkEdge(){
    // actual = digitalRead(inputpin);
    actual = bit_atual;

    if(past != actual){
        edge = 1;
    } else {
        edge = 0;
    }
     past = actual;
//    past = bit_atual;
}*/
  
void bit_timing::checksync() {
    if(edge == 1 && actual_state != SYNC){
        edge = 0;
        soft_sync = 1;
        //resync();
    }
}

int bit_timing::sampling_point(){
    return sampling_point_;
}

int bit_timing::writing_point(){
    return writing_point_;
}

