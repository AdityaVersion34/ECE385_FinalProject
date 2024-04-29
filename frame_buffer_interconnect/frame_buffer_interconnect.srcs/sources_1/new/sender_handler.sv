`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 04/28/2024 11:11:37 PM
// Design Name: 
// Module Name: sender_handler
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

//handles the data received from the sender
module sender_handler#(
    parameter integer ITX_DATA_WIDTH = 12
    )(
    input logic sender_clk,
    input logic [ITX_DATA_WIDTH-1: 0] sender_from,     //the data received
    output logic sender_to,
    output logic [ITX_DATA_WIDTH-1: 0] data_got
    );
    
    //on clock, buffer data received
    always_ff @(posedge sender_clk)
    begin
        data_got = sender_from;
    end
    
endmodule
