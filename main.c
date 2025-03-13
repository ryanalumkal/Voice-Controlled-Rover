#include <methods.h>
#include <stdio.h>

#define AUDIO_BASE 0xFF203040
#define KEY_BASE 0xFF200050
#define TIMER_BASE 0xFF202000
#define SAMPLES 8000

struct audio_t {
        volatile unsigned int CONTROL; // Control register
        volatile unsigned char RARC;   // number of stuff in input right channel
        volatile unsigned char RALC;   // number of stuff in input left channel
        volatile unsigned char WSRC;   // number of stuff in output right channel
        volatile unsigned char WSLC;   // number of stuff in ouput left channel
        volatile unsigned int LEFTDATA;   // Left channel data
        volatile unsigned int RIGHTDATA;   // Right channel data
};

struct key_t {
    volatile unsigned int DATA;
    volatile unsigned int UNUSED;
    volatile unsigned int INTERRUPTMASK;
    volatile unsigned int EDGECAPTURE;
};

struct timer_t {
    volatile unsigned int STATUS;
    volatile unsigned int CONTROL;
    volatile unsigned int COUNTER_START_LOW;
    volatile unsigned int COUNTER_START_HIGH;
    volatile unsigned int COUNTER_SNAPSHOT_LOW;
    volatile unsigned int COUNTER_SNAPSHOT_HIGH;
};


int main(void){
    

}