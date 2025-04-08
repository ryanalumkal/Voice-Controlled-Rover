// VGA OUTPUT
#define CHAR_BUFFER_BASE 0x09000000

int pixel_buffer_start;

int is_start_stop = 1;  // start by default
#define CAR_WIDTH 10
#define CAR_HEIGHT 10

int moving = 0;  // 0 for stop, 1 for start, 2 for right, 3 for left
int car_x = 160;
int car_y = 220;
int prev_car_x = 160;  // Track previous car x position
int prev_car_y = 220;  // Track previous car y position

#define CAR_COLOR 0xF800         // RED
#define WHEEL_COLOR 0x0000       // BLACK
#define WINDOW_COLOR 0x001F      // BLUE
#define BACKGROUND_COLOR 0x0000  // PURPLE
#define BOX_COLOR 0xFFFF         // White

// Draw a box on the screen
void draw_box(int x1, int y1, int x2, int y2, short int color) {
  for (int x = x1; x <= x2; x++) {
    plot_pixel(x, y1, color);
    plot_pixel(x, y2, color);
  }

  // Draw left and right borders
  for (int y = y1; y <= y2; y++) {
    plot_pixel(x1, y, color);
    plot_pixel(x2, y, color);
  }
}
void plot_pixel(int x, int y, short int line_color) {
  volatile short int *one_pixel_address;
  one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);
  *one_pixel_address = line_color;
}

// To help display text on the screen
void write_string(int x, int y, char *text) {
  volatile char *character_buffer = (char *)CHAR_BUFFER_BASE;
  int offset = (y << 7) + x;

  while (*text) {
    *(character_buffer + offset) = *text;
    text++;
    offset++;
  }
}

// Draw a car on the screen
void draw_car(int x, int y) {
  for (int dx = 0; dx < CAR_WIDTH; dx++) {
    for (int dy = 0; dy < CAR_HEIGHT; dy++) {
      plot_pixel(x + dx, y + dy, CAR_COLOR);
    }
  }

  // Car wheels
  plot_pixel(x + 2, y + 2, WHEEL_COLOR);
  plot_pixel(x + 7, y + 2, WHEEL_COLOR);
  plot_pixel(x + 2, y + 7, WHEEL_COLOR);
  plot_pixel(x + 7, y + 7, WHEEL_COLOR);

  // Car windows
  for (int dx = 3; dx < 7; dx++) {
    for (int dy = 3; dy < 5; dy++) {
      plot_pixel(x + dx, y + dy, WINDOW_COLOR);
    }
  }
}

// Clear the screen by filling it with the background color
void clear_screen() {
  for (int x = 0; x <= 320; x++) {
    for (int y = 0; y <= 240; y++) {
      plot_pixel(x, y, BACKGROUND_COLOR);
    }
  }
}

// Erase the car from the screen
void erase_car(int x, int y) {
  for (int dx = 0; dx < CAR_WIDTH; dx++) {
    for (int dy = 0; dy < CAR_HEIGHT; dy++) {
      plot_pixel(x + dx, y + dy, BACKGROUND_COLOR);
    }
  }
}

// MOTOR CONTROL
#define JP2 0xFF200070
#define LEDs 0xFF200000
#define TIMER_BASE 0xFF202000

volatile int *leds_ptr = (int *)LEDs;
volatile int *timer_ptr = (int *)TIMER_BASE;

struct PIT_t {
  volatile unsigned int DR;
  volatile unsigned int DIR;
};

struct PIT_t *pitp = ((struct PIT_t *)JP2);

void configTimer(int counter_delay) {
  *(timer_ptr + 0x2) = (counter_delay & 0xFFFF);
  *(timer_ptr + 0x3) = ((counter_delay >> 16) & 0xFFFF);
  *(timer_ptr + 1) = 0x6;
}

void delay() {
  while (((*timer_ptr) & 0x1) != 1);
  *timer_ptr = 0;
}

void control_motor(const char *command) {
  // START: BEEP + is_start_stop == 1
  if (command == "BEEP" && is_start_stop) {
    pitp->DR = 0b0110;
    is_start_stop = 0;
    // STOP: BEEP + is_start_stop == 0
  } else if (command == "BEEP" && !(is_start_stop)) {
    pitp->DR = 0b0000;
    is_start_stop = 1;
    // RIGHT: BEEP BEEP
  } else if (command == "BEEP BEEP") {
    pitp->DR = 0b0100;

    // RIGHT: BEEP BEEP BEEP
  } else if (command == "BEEP BEEP BEEP") {
    pitp->DR = 0b0010;
  }
  return;
}

// SIGNAL PROCESSING
#include <math.h>
#include <stdio.h>

#define KEYS 0xFF200050
#define AUDIO_BASE 0xFF203040
#define SAMPLING_RATE 8000
#define RECORD_TIME 1.5
#define BUFFER_SIZE 12000

#define AMPLITUDE_THRESHOLD 0.05
#define MIN_PEAK_DURATION 160
#define MIN_GAP_DURATION 400

// Preprocessing functions
void remove_dc_offset(float *signal, int length) {
  float sum = 0.0f;
  for (int i = 0; i < length; i++) {
    sum += signal[i];
  }
  float mean = sum / length;
  for (int i = 0; i < length; i++) {
    signal[i] -= mean;
  }
}
void moving_average_filter(float *signal, int length, int window) {
  float *temp = malloc(length * sizeof(float));
  for (int i = 0; i < length; i++) {
    float sum = 0.0f;
    int count = 0;
    for (int j = i - window; j <= i + window; j++) {
      if (j >= 0 && j < length) {
        sum += signal[j];
        count++;
      }
    }
    temp[i] = sum / count;
  }
  for (int i = 0; i < length; i++) {
    signal[i] = temp[i];
  }
  free(temp);
}

void normalize_signal(float *signal, int length) {
  float max_val = 0.0f;
  for (int i = 0; i < length; i++) {
    float abs_val = fabs(signal[i]);
    if (abs_val > max_val) max_val = abs_val;
  }
  if (max_val == 0.0f) return;
  for (int i = 0; i < length; i++) {
    signal[i] /= max_val;
  }
}

// Calculate signal envelope using absolute value and smoothing
void calculate_envelope(float *signal, float *envelope, int length) {
  for (int i = 0; i < length; i++) {
    envelope[i] = fabs(signal[i]);
  }
  moving_average_filter(envelope, length, 16);
}

// Counts the number of beep sounds in the audio signal
int count_beeps(float *signal, int length) {
  int beep_count = 0;
  int above_threshold_count = 0;
  int below_threshold_count = 0;
  int in_beep = 0;

  // Allocate memory for envelope
  float *envelope = malloc(length * sizeof(float));

  // Preprocessing pipeline
  remove_dc_offset(signal, length);
  normalize_signal(signal, length);
  calculate_envelope(signal, envelope, length);

  // Debug: Print signal statistics
  float max_env = 0.0f;
  float min_env = 1.0f;
  float sum_env = 0.0f;
  int above_threshold_samples = 0;

  // Calculate statistics
  for (int i = 0; i < length; i++) {
    if (envelope[i] > max_env) max_env = envelope[i];
    if (envelope[i] < min_env) min_env = envelope[i];
    sum_env += envelope[i];
    if (envelope[i] >= AMPLITUDE_THRESHOLD) above_threshold_samples++;
  }
  float avg_env = sum_env / length;

  // Beep detection using envelope
  for (int i = 0; i < length; i++) {
    if (envelope[i] >= AMPLITUDE_THRESHOLD) {
      above_threshold_count++;
      below_threshold_count = 0;

      if (!in_beep && above_threshold_count >= MIN_PEAK_DURATION) {
        in_beep = 1;
        beep_count++;
      }
    } else {
      below_threshold_count++;

      if (below_threshold_count > 5) {
        above_threshold_count = 0;
      }

      if (in_beep && below_threshold_count >= MIN_GAP_DURATION) {
        in_beep = 0;
      }
    }
  }

  free(envelope);
  return beep_count;
}

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
int audio_buffer[BUFFER_SIZE];
int audio_buffer_right[BUFFER_SIZE];
float float_buffer[BUFFER_SIZE];
int buffer_index = 0;
int has_recording = 0;

// Convert integer audio data to normalized float array
void convert_to_float(int *int_buffer, float *float_buffer, int length) {
  int max_val = 0;
  for (int i = 0; i < length; i++) {
    int abs_val = abs(int_buffer[i]);
    if (abs_val > max_val) max_val = abs_val;
  }

  // Convert to float and normalize
  if (max_val > 0) {
    for (int i = 0; i < length; i++) {
      float_buffer[i] = (float)int_buffer[i] / max_val;
    }
  }
}

enum State { WAIT_TO_RECORD, RECORDING, WAIT_TO_PLAYBACK, PLAYING };

// MOTOR  + VGA CONTROL
void motor_vga_control(const char **command) { control_motor(*command); }

void move_car() {
  // Erase the car from its previous position
  erase_car(prev_car_x, car_y);

  // Move the car to the right (with wrap around)
  car_x++;
  if (car_x > 320 - CAR_WIDTH) {
    car_x = 0;
  }

  // Draw the car at the new position
  draw_car(car_x, car_y);

  // Update the previous car position
  prev_car_x = car_x;

  // Simple delay to slow down the animation
  for (volatile int i = 0; i < 1000000; i++);
}

// MAIN
int main() {
  // SETUP
  // Vga setup
  volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
  pixel_buffer_start = *pixel_ctrl_ptr;
  clear_screen();
  int car_x = 160;
  int car_y = 220;

  draw_car(car_x, car_y);  // Draw the initial car

  int prev_car_x = car_x;  // Track previous car x position
  int prev_car_y = car_y;  // Track previous car y position

  // text
  draw_box(20, 20, 120, 42, BOX_COLOR);
  write_string(2, 2, "243 Project");
  write_string(2, 5, "START/STOP : BEEP");
  write_string(2, 7, "RIGHT      : BEEP BEEP");
  write_string(2, 9, "LEFT       : BEEP BEEP BEEP");

  // Motor Setup
  pitp->DIR = 0x000000FF;
  pitp->DR = 0x00000000;
  configTimer(2000000);
  *timer_ptr = 0x0;
  *leds_ptr = 0x0;
  const char *command = "PLACEHOLDER";

  // MAIN LOOP

  enum State current_state = WAIT_TO_RECORD;
  // Initialize key edge capture
  button->edgeCapture = 0;

  while (1) {
    switch (current_state) {
      case WAIT_TO_RECORD:
        move_car();
        // Wait for key press to start recording
        if (button->edgeCapture) {
          button->edgeCapture = 0xF;  // Clear the edge capture
          buffer_index = 0x0;         // Reset buffer index
          current_state = RECORDING;
        }
        break;

      case RECORDING:
        *leds_ptr = 1;
        if (buffer_index < BUFFER_SIZE) {
          // Wait for data to be available for reading
          while (audiop->rarc == 0) {
          }

          // Read audio data and store in buffer
          audio_buffer[buffer_index] = audiop->ldata;
          audio_buffer_right[buffer_index] = audiop->rdata;
          buffer_index++;
        } else {
          has_recording = 0x1;
          current_state = WAIT_TO_PLAYBACK;
          *leds_ptr = 0x0;
        }
        break;

      case WAIT_TO_PLAYBACK:
        *leds_ptr = 0x2;
        convert_to_float(audio_buffer, float_buffer, BUFFER_SIZE);
        *leds_ptr = 0x4;

        int beep_count = count_beeps(float_buffer, BUFFER_SIZE);

        // Store command type as an integer instead of string
        // 1=beep, 2=beepbeep, 3=beepbeepbeep, 0=unknown
        int command_type = 0;
        int led_pattern = 0b00000000;

        // Determine the command based on beep count
        if (beep_count == 1) {
          command_type = 1;  // beep
          command = "BEEP";
          led_pattern = 0b10000000;
        } else if (beep_count == 2) {
          command_type = 2;  // beepbeep
          led_pattern = 0b11000000;
          command = "BEEP BEEP";
        } else if (beep_count >= 3) {
          command_type = 3;  // beepbeepbeep
          led_pattern = 0b11100000;
          beep_count = 3;
          command = "BEEP BEEP BEEP";
        } else if (beep_count == 0) {
          led_pattern = 0b00011000;
          command = "NOT VALID";
        }

        printf("\nDetection Results:\n");
        printf("Beep count: %d\n", beep_count);
        if (command_type == 1 && is_start_stop) {
          printf("Command: START\n");
        } else if (command_type == 1 && !is_start_stop) {
          printf("Command: STOP\n");
        } else if (command_type == 2) {
          printf("Command: RIGHT\n");
        } else if (command_type == 3) {
          printf("Command: LEFT\n");
        } else {
          printf("Command: NOT VALID\n");
        }

        int delay = 20000;
        while (delay > 0) {
          *leds_ptr = led_pattern;
          delay--;
        }
        current_state = PLAYING;
        break;

      case PLAYING:
        *leds_ptr = 0x0;  // Turn off LED2 after playback is complete
        if (command != "NOT VALID") {
          motor_vga_control(&command);
        }
        current_state = WAIT_TO_RECORD;
        break;
    }
  }
  return 0;
}