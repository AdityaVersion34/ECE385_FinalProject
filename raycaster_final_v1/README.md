ECE 385 - Final Project
By Aditya Venkatesh

This is a 3D-like raycasting engine.
To run the program, use the Vitis application project in the "re_workspace" folder. Use the provided .xsa file as the base hardware.

The engine map can be edited by modifying the map array in the lw_usb_main.c file.
This program is intended to be run on the Xilinx Spartan-7 chip. Refer to the .xpr file for details.

The raycasting algorithm was sourced from https://lodev.org/cgtutor/raycasting.html

The same concepts were used, translated to C and using fixed-point representations.
The MicroBlaze outputs are fed into a double-frame buffer and rendered using some hardware modules.

The program accepts keyboard inputs (WASD) from a USB keyboard, and produces visual output through the Urbana board HDMI port.