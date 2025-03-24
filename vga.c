#define CHAR_BUFFER_BASE 0x09000000

int pixel_buffer_start; // global variable

// Define the car as a simple 4x4 pixel block
#define CAR_WIDTH 4
#define CAR_HEIGHT 4

// Car color (red)
#define CAR_COLOR 0xF800
// Background color (background color used in clear_screen)
#define BACKGROUND_COLOR 0xe88aff

// Function to plot a pixel at (x, y) with a color
void plot_pixel(int x, int y, short int line_color)
{
    volatile short int *one_pixel_address;
    one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);
    *one_pixel_address = line_color;
}
void write_string(int x, int y, char* text) {
  volatile char* character_buffer = (char*)CHAR_BUFFER_BASE;
  int offset = (y << 7) + x;  // (y * 128) + x

  // Copy the string to the character buffer
  while (*text) {
    *(character_buffer + offset) = *text;
    text++;
    offset++;
  }
}

// Function to draw the car at a specific (x, y) position
void draw_car(int x, int y)
{
    // Draw a 4x4 car sprite at position (x, y)
    for (int dx = 0; dx < CAR_WIDTH; dx++) {
        for (int dy = 0; dy < CAR_HEIGHT; dy++) {
            plot_pixel(x + dx, y + dy, CAR_COLOR);
        }
    }
}

void clear_screen(){
	
	for (int x = 0; x <= 320; x++){
		for (int y = 0; y  <=240; y++){
			plot_pixel(x, y, BACKGROUND_COLOR);
			}
	}
}

// Function to erase the car from the previous position (x, y)
void erase_car(int x, int y)
{
    // Erase the 4x4 car sprite by setting it to the background color
    for (int dx = 0; dx < CAR_WIDTH; dx++) {
        for (int dy = 0; dy < CAR_HEIGHT; dy++) {
            plot_pixel(x + dx, y + dy, BACKGROUND_COLOR);
        }
    }
}



int main()
{
    volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
    pixel_buffer_start = *pixel_ctrl_ptr;
	clear_screen();
    int car_x = 160;  // Start car at the center of the screen (x-axis)
    int car_y = 220;  // Start car near the bottom of the screen

    draw_car(car_x, car_y);  // Draw the initial car

    int prev_car_x = car_x;  // Track previous car x position
	write_string(32, 5, "243 Project");
write_string(28, 15, "START"); write_string(40, 15, "TAP");
write_string(28, 25, "STOP"); write_string(40, 25, "TAP TAP");
write_string(28, 35, "LEFT"); write_string(40, 35, "BEEP");
write_string(28, 45, "RIGHT"); write_string(40, 45, "BEEP BEEP");
    while (1) {
        // Erase the car from its previous position
        erase_car(prev_car_x, car_y);

        // Move the car to the right (with wrap around)
        car_x++;
        if (car_x > 320 - CAR_WIDTH) {
            car_x = 0;  // Wrap around to the left side
        }

        // Draw the car at the new position
        draw_car(car_x, car_y);

        // Update the previous car position
        prev_car_x = car_x;

        // Simple delay to slow down the animation
        for (volatile int i = 0; i < 1000000; i++);
    }

    return 0;
}
