#include <stdio.h>

#define KEYS 0xFF200050
#define AUDIO_BASE 0xFF203040

#define SAMPLING_RATE 8000
#define RECORD_TIME 1.5  // 2 seconds of recording
#define BUFFER_SIZE (SAMPLING_RATE * RECORD_TIME)
#define LEDs 0xFF200000
	
volatile int *leds_ptr = (int*) LEDs;

struct audio_t {
  volatile unsigned int control;
  volatile unsigned char rarc;
  volatile unsigned char ralc;
  volatile unsigned char wsrc;
  volatile unsigned char wslc;
  volatile unsigned int ldata;
  volatile unsigned int rdata;
};

struct key {
  volatile int data;
  volatile int control;
  volatile int interupt;
  volatile int edgeCapture;
};

struct audio_t *const audiop = ((struct audio_t *)AUDIO_BASE);
struct key *const button = ((struct key *)KEYS);

// Buffer to store recorded audio
int audio_buffer_left[BUFFER_SIZE];
int audio_buffer_right[BUFFER_SIZE];
int buffer_index = 0;
int has_recording = 0;

enum State { WAIT_TO_RECORD, RECORDING, WAIT_TO_PLAYBACK, PLAYING };

int main(void) {
  enum State current_state = WAIT_TO_RECORD;
	*leds_ptr = 0x0;

  // Initialize key edge capture
  button->edgeCapture = 0;

  while (1) {
    switch (current_state) {
      case WAIT_TO_RECORD:
        // Wait for key press to start recording
        if (button->edgeCapture) {
          button->edgeCapture = 0xF;  // Clear the edge capture
          buffer_index = 0x0;           // Reset buffer index
          current_state = RECORDING;
        }
        break;

      case RECORDING:
			*leds_ptr = 1;
        // Record audio for 2 seconds
        if (buffer_index < BUFFER_SIZE) {
          // Wait for data to be available for reading
          while (audiop->rarc == 0) {
          }

          // Read audio data and store in buffer
          audio_buffer_left[buffer_index] = audiop->ldata;
          audio_buffer_right[buffer_index] = audiop->rdata;
          buffer_index++;
        } else {
          has_recording = 0x1;
          current_state = WAIT_TO_PLAYBACK;
			*leds_ptr = 0x0;
        }
        break;

      case WAIT_TO_PLAYBACK:
        // Wait for key press to start playback
        if (button->edgeCapture) {
          button->edgeCapture = 0xF;  // Clear the edge capture
          buffer_index = 0;           // Reset buffer index for playback
          current_state = PLAYING;
        }
        break;

      case PLAYING:
		  *leds_ptr = 0x2;  // Turn on LED2 at the start of playback
		  if (buffer_index < BUFFER_SIZE && has_recording) {
			// Wait for space in the write FIFO
			  while (buffer_index < BUFFER_SIZE){
				while (audiop->wsrc == 0) { }
				  printf("%d,", audio_buffer_left[buffer_index]);
				// Write audio data from buffer
				audiop->ldata = audio_buffer_left[buffer_index];
				audiop->rdata = audio_buffer_right[buffer_index];
				buffer_index++;
			  }
		  } else {
			*leds_ptr = 0x0;  // Turn off LED2 after playback is complete
			current_state = WAIT_TO_RECORD;
		  }
		  break;
    }
  }

  return 0;
}



