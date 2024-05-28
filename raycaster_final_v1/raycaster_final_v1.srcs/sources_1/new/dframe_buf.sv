`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 05/03/2024 08:39:14 AM
// Design Name: 
// Module Name: dframe_buf
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


module dframe_buf(
    input logic Clk,
    input logic [31:0] colDataIn,
    input logic [9:0] vga_y_in,
    
    output logic [31:0] colDataOut
    );
    //assign colDataOut = colDataIn;
    //assign colDataIn = 8'h120FF0FF;
    
    //the front and back buffers
    logic [31:0] buf_set_0 [479];
    logic [31:0] buf_set_1 [479];
    
    //signal that toggles between the front and back buffers
    logic buf_toggle = 0;
    //signal that indicates if we hold the output frame, waiting for the back frame to finish
    logic disp_frame_hold = 0;
    
    always_ff @(posedge Clk)
    begin
        if(buf_toggle)
        begin
            //when back buffer is 1
            colDataOut = buf_set_0[vga_y_in];   //assigning the data out
            buf_set_1[colDataIn[23:12]] = colDataIn;
        end
        else
        begin
            //when back buffer is 0
            colDataOut = buf_set_1[vga_y_in];
            buf_set_0[colDataIn[23:12]] = colDataIn;
        end
        
        //afterwards, if vga_y_in is 479 (the final row), we toggle
        if(vga_y_in == 479)
        begin
            disp_frame_hold = 1;    //when front buffer reaches end, we hold it until back buffer is finished
        end
        
        
        if(disp_frame_hold == 1 && colDataIn[23:12] == 479)
        begin
            disp_frame_hold = 0;
            buf_toggle = ~buf_toggle;
        end
    end
    
    
endmodule
