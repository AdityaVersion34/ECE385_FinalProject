`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 04/28/2024 11:11:54 PM
// Design Name: 
// Module Name: rec_handler
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


module rec_handler#(
    parameter integer ITX_DATA_WIDTH = 12
    )(
    input logic rec_clk,
    input logic [ITX_DATA_WIDTH-1: 0] data_out,     //the data received
    input logic rec_from,
    output logic [ITX_DATA_WIDTH-1: 0] rec_to
    );
    
    always_ff @ (posedge rec_clk)
    begin
        rec_to = data_out;
    end
    
endmodule
