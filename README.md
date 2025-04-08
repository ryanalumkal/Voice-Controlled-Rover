# De1-SoC Voice Controlled Rover

## Project Description
This project integrates voice command recognition with the DE1-SoC FPGA board to control a physical car. Using a microphone, the system captures audio commands in the form of beeps, which are mapped to specific actions. The audio signals undergo processing through various signal analysis techniques, including DC offset removal, normalization, envelope calculation, and a moving average filter to accurately count beeps. These processed commands are then used to drive the car’s motors, enabling actions such as starting, stopping, and turning left and right. The VGA screen displays command prompts, while LED0 indicates active recording. This project showcases real-time signal processing and hardware interaction, utilizing embedded systems for hands-free vehicle control.

## Block Diagram

![Block Diagram](Block_Diagram.jpg)