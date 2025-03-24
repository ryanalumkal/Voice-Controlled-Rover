#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define AUDIO_BASE 0xFF203040
#define SAMPLE_RATE 8000   // 8 kHz sampling
#define MAX_SAMPLES 16000  // 2 seconds of audio (2 * SAMPLE_RATE)
#define ENERGY_THRESHOLD 1000  // Energy threshold for speech detection

volatile int *audio_ptr = (int *)AUDIO_BASE;
int16_t audio_buffer[MAX_SAMPLES*2]; // Buffer to store audio
int16_t speech_buffer[MAX_SAMPLES*2]; // Buffer to store detected speech

// Function to calculate the energy of the audio signal for a specific range
int calculate_energy(int start_index, int end_index) {
    int energy = 0;
    for (int i = start_index; i < end_index; i++) {
        energy += audio_buffer[i] * audio_buffer[i];  // Sum of squares
    }
    return energy;
}

// Function to check if someone is starting to speak based on energy
int detect_start_of_speech(int start_index, int end_index) {
    int energy = calculate_energy(start_index, end_index);
    if (energy > ENERGY_THRESHOLD) {
        return 1; // Speech detected
    }
    return 0; // No speech detected
}

void capture_audio() {
    int i = 0;
    int speech_detected = 0;
    int speech_start_index = 0;

    printf("Recording for 2 seconds...\n");

    while (i < MAX_SAMPLES*2) {
        if ((*(audio_ptr + 1) & 0x000000FF) > 0) {  // Check if new sample available
            audio_buffer[i] = *(audio_ptr + 2);  // Left channel
            i++;
        }

        // Check for speech detection in real-time as audio is captured
        if (i > SAMPLE_RATE) {  // After 1 second, start checking for speech
            if (detect_start_of_speech(i - SAMPLE_RATE, i)) {  // Check energy of last 1 second chunk
                if (!speech_detected) {  // Only print the message once
                    printf("Speech detected in real-time!\n");
                    speech_detected = 1;
                    speech_start_index = i - SAMPLE_RATE;  // Mark the start index of speech
                }
            }
        }
    }

    if (speech_detected) {
        // Copy the detected speech to the speech_buffer from speech_start_index to the end of audio capture
        int j = 0;
        for (int k = speech_start_index; k < i; k++) {
            speech_buffer[j++] = audio_buffer[k];
        }

        printf("Speech detected! Processing...\n");
        // Now you can pass `speech_buffer` to your AI model or further processing
    } else {
        printf("No speech detected in 2 seconds.\n");
    }
}

int main() {
    while (1) {
        capture_audio();
        // pass speech_buffer to AI model for command detection
    }
    return 0;
}
