`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 04/28/2024 03:43:26 PM
// Design Name: 
// Module Name: interconnect_top
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

//this is mark 1 of the interconnect
//12-bit colors (4bit r,g,b)
//12-bit dual spi-like communication with pixel receiver and pixel out
//slave to both receiver and out
//arbitrates between different clock speeds using a 2x640x12 bit cache (double buffering)
module interconnect_top #(
    parameter integer ITX_DATA_WIDTH = 12
    )(
    //io related to the device from which this module RECEIVES data from
    input logic sender_clk,
    input logic [ITX_DATA_WIDTH-1: 0] sender_from,     //the data received
    output logic sender_to,      //control output to sender
    
    //io related to the device to which this module SENDS data to
    input logic rec_clk,
    input logic rec_from,
    output logic [ITX_DATA_WIDTH-1: 0] rec_to
    );
    
    logic [ITX_DATA_WIDTH-1: 0] data_got;
    logic [ITX_DATA_WIDTH-1: 0] data_out;
    
    sender_handler #(.ITX_DATA_WIDTH(ITX_DATA_WIDTH)) sender_handler_inst (
        .*
    );
    
    rec_handler #(.ITX_DATA_WIDTH(ITX_DATA_WIDTH)) rec_handler_inst (
        .*
    );
    
endmodule
