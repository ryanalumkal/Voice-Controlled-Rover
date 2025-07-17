# Project Description
This project integrates voice command recognition with the DE1-SoC FPGA board to control a physical car. Using a microphone, the system captures audio commands in the form of beeps, which are mapped to specific actions. The audio signals undergo processing through various signal analysis techniques, including DC offset removal, normalization, envelope calculation, and a moving average filter to accurately count beeps. These processed commands are then used to drive the car’s motors, enabling actions such as starting, stopping, and turning left and right. The VGA screen displays command prompts, while LED0 indicates active recording. This project showcases real-time signal processing and hardware interaction, utilizing embedded systems for hands-free vehicle control.

# Operating Instructions
Upon compiling and loading the code onto the DE1-SoC board, command prompts will appear on the VGA screen. After connecting the microphone to the board and the physical car to the GPIO pins, press any button to capture audio as you say a command. The system will record for 1.5 seconds, indicated by LED0 lighting up during recording. It recognizes three distinct commands: beep, beep beep, and beep beep beep, each activating the car's motors to perform a specific action—such as starting, stopping, or turning left or right. 

# Block Diagram

![Block Diagram](Block_Diagram.jpg)

# Real World Applications

This technology has broad real-world applications, ranging from robotic control in industrial and domestic settings to enabling self-driving cars that assist physically disabled individuals with greater independence and mobility. Its versatility makes it valuable across various sectors, including healthcare, transportation, and assistive technologies.
