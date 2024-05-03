`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 05/03/2024 10:37:59 AM
// Design Name: 
// Module Name: line_colorer
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


module line_colorer(
    input logic [31:0] colDataOut,
    input logic [9:0] vga_x_in,
    
    output logic [3:0] red, green, blue
    );
    
    logic [9:0] pix_beg, pix_end;
    
    always_comb     //calculating the beginning and end points 
    begin: ENDPOINT_CALC
        pix_beg = 320 - (colDataOut[11:0]>>1);
        pix_end = 319 + (colDataOut[11:0]>>1);
    end
    
    
    
    always_comb
    begin: COLOR_CALC
        if(vga_x_in < pix_beg || vga_x_in > pix_end)        //black outside of fill area
        begin
            red = 0;
            green = 0;
            blue = 0;
        end
        else
        begin
            case(colDataOut[27:24])
                4'b0001:
                begin
                    red = 4'hA;
                    green = 4'hA;
                    blue = 4'hA;
                end
                4'b0010:
                begin
                    red = 4'hF;
                    green = 0;
                    blue = 0;
                end
                4'b0011:
                begin
                    red = 0;
                    green = 4'hF;
                    blue = 0;
                end
                default:
                begin
                    red = 0;
                    green = 0;
                    blue = 4'hF;
                end
            endcase
            //shading if side = 0
            
            if(colDataOut[31:28] == 0)
            begin
                red = red>>1;
                green = green>>1;
                blue = blue>>1;
            end
            
        end
    end
    
    
endmodule
