#include <stdio.h>
#include "platform.h"
#include "lw_usb/GenericMacros.h"
#include "lw_usb/GenericTypeDefs.h"
#include "lw_usb/MAX3421E.h"
#include "lw_usb/USB.h"
#include "lw_usb/usb_ch9.h"
#include "lw_usb/transfer.h"
#include "lw_usb/HID.h"

#include "xparameters.h"
#include <xgpio.h>

#include <math.h>

extern HID_DEVICE hid_device;

static XGpio Gpio_hex;

static BYTE addr = 1; 				//hard-wired USB address
const char* const devclasses[] = { " Uninitialized", " HID Keyboard", " HID Mouse", " Mass storage" };

//--------------------
//new defs:

#define PI 3.14159
#define monitorXdim 480		//since hdmi prints row major order, currently using a rotated monitor
#define monitorYdim 640

//--------------------

BYTE GetDriverandReport() {
	BYTE i;
	BYTE rcode;
	BYTE device = 0xFF;
	BYTE tmpbyte;

	DEV_RECORD* tpl_ptr;
	xil_printf("Reached USB_STATE_RUNNING (0x40)\n");
	for (i = 1; i < USB_NUMDEVICES; i++) {
		tpl_ptr = GetDevtable(i);
		if (tpl_ptr->epinfo != NULL) {
			xil_printf("Device: %d", i);
			xil_printf("%s \n", devclasses[tpl_ptr->devclass]);
			device = tpl_ptr->devclass;
		}
	}
	//Query rate and protocol
	rcode = XferGetIdle(addr, 0, hid_device.interface, 0, &tmpbyte);
	if (rcode) {   //error handling
		xil_printf("GetIdle Error. Error code: ");
		xil_printf("%x \n", rcode);
	} else {
		xil_printf("Update rate: ");
		xil_printf("%x \n", tmpbyte);
	}
	xil_printf("Protocol: ");
	rcode = XferGetProto(addr, 0, hid_device.interface, &tmpbyte);
	if (rcode) {   //error handling
		xil_printf("GetProto Error. Error code ");
		xil_printf("%x \n", rcode);
	} else {
		xil_printf("%d \n", tmpbyte);
	}
	return device;
}

void printHex (u32 data, unsigned channel)
{
	XGpio_DiscreteWrite (&Gpio_hex, channel, data);
}

int main() {
    
	//making the game map
	u8 gameMap[15][15] = 
	{
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,2,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,2,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,2,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,2,2,2,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,2,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
	};

	//defining and initializing position and direction variables. camera plane vector is tied to the dir vect
	float xpos = 7;		//starts in the middle
	float ypos = 7;
	float xdir = 0;		//starts facing to the top. unit vector.
	float ydir = 1;
	float xcam = 1;
	float ycam = 0;

	//declaring more variables
	float scaledScreenPos;		//this variable stores the current screen position scaled on a -1 to 1 scale
	float castRayx;			//stores x and y pos of the current cast vector (not including player pos)
	float castRayy;

	float distFromx;		//distance from current cast ray position to the next x or y edge it will collide with
	float distFromy;

	float distToNextx;		//how far the cast ray has to be projected to produce a 1x or 1y step
	float distToNexty;

	u8 isHit;		//stores the kind of wall that has been hit
	u8 whichSide;	//was the wall hit in x or y dir
	u8 stepDirx;	//which direction to step in for the DDA alg in both directions
	u8 stepDiry;

	u8 truncXpos;		//truncated x and y player positions - which map cell are we in
	u8 truncYpos;
	
	init_platform();
    XGpio_Initialize(&Gpio_hex, XPAR_GPIO_USB_KEYCODE_DEVICE_ID);
   	XGpio_SetDataDirection(&Gpio_hex, 1, 0x00000000); //configure hex display GPIO
   	XGpio_SetDataDirection(&Gpio_hex, 2, 0x00000000); //configure hex display GPIO


   	BYTE rcode;
	BOOT_MOUSE_REPORT buf;		//USB mouse report
	BOOT_KBD_REPORT kbdbuf;

	BYTE runningdebugflag = 0;//flag to dump out a bunch of information when we first get to USB_STATE_RUNNING
	BYTE errorflag = 0; //flag once we get an error device so we don't keep dumping out state info
	BYTE device;

	xil_printf("initializing MAX3421E...\n");
	MAX3421E_init();
	xil_printf("initializing USB...\n");
	USB_init();
	while (1) {
		xil_printf("."); //A tick here means one loop through the USB main handler
		MAX3421E_Task();
		USB_Task();
		if (GetUsbTaskState() == USB_STATE_RUNNING) {	//operations within this condition - usb is polling
			if (!runningdebugflag) {
				runningdebugflag = 1;
				device = GetDriverandReport();
			} else if (device == 1) {		//within this - using keyboard not mouse
				//run keyboard debug polling
				rcode = kbdPoll(&kbdbuf);
				if (rcode == hrNAK) {
					continue; //NAK means no new data
				} else if (rcode) {
					xil_printf("Rcode: ");
					xil_printf("%x \n", rcode);
					continue;
				}
				//COMMENTING OUT PRINT SECTION TO PREVENT CLUTTER
				/*
				xil_printf("keycodes: ");
				for (int i = 0; i < 6; i++) {
					xil_printf("%x ", kbdbuf.keycode[i]);
				}
				*/
				//Outputs the first 4 keycodes using the USB GPIO channel 1
				//printHex (kbdbuf.keycode[0] + (kbdbuf.keycode[1]<<8) + (kbdbuf.keycode[2]<<16) + + (kbdbuf.keycode[3]<<24), 1);
				//Modify to output the last 2 keycodes on channel 2.
				//xil_printf("\n");
				//---------------------------------------------

				//game loop execution

				//digital differential analysis algorithm - for each ray, based on the direction,
				//jump between the nearest edge and see if it touches a wall. return the nearest one with the dist

				//leaving algorithm in main for now, may refactor later

				//loop through every vertical strip on screen:
				for(u16 i=0; i<monitorXdim; i++){
					scaledScreenPos = 2*(i/(double)monitorXdim) - 1;		//setting ssp based on curr screen pos
					
					castRayx = xdir + xcam*(scaledScreenPos);
					castRayy = ydir + ycam*(scaledScreenPos);

					truncXpos = (u8)xpos;		//updating current cell we're in
					truncYpos = (u8)ypos;

					//these represent the distance the ray has to cover to travel 1 unit in x and y dist
					//condition to prevent division by zero, setting it to very large value
					distToNextx = (castRayx == 0) ? 1e30 : abs(1/castRayx);
					distToNexty = (castRayy == 0) ? 1e30 : abs(1/castRayy);

					//initializing which direction we're going in and initial relative distFrom values
					if(castRayx < 0){		//for x axis
						stepDirx = -1;
						distFromx = (xpos - truncXpos)*distToNextx;
					}else{
						stepDirx = 1;
						distFromx = (1.0 + (truncXpos - xpos))*distToNextx;
					}

					if(castRayy < 0){
						stepDiry = -1;
						distFromy = (ypos - truncYpos)*distToNexty;
					}else{
						stepDirx = 1;
						distFromy = (1.0 + (truncYpos - ypos))*distToNexty;
					}

					//loop through ray, until we hit a wall
					while(isHit == 0){
						//jump based on which axis grid side is closer
						if(distFromx < distFromy){
							distFromx += distToNextx;
							truncXpos += stepDirx;
							whichSide = 0;
						}else{

						}
					}
				}

			}

			else if (device == 2) {
				rcode = mousePoll(&buf);
				if (rcode == hrNAK) {
					//NAK means no new data
					continue;
				} else if (rcode) {
					xil_printf("Rcode: ");
					xil_printf("%x \n", rcode);
					continue;
				}
				xil_printf("X displacement: ");
				xil_printf("%d ", (signed char) buf.Xdispl);
				xil_printf("Y displacement: ");
				xil_printf("%d ", (signed char) buf.Ydispl);
				xil_printf("Buttons: ");
				xil_printf("%x\n", buf.button);
			}
		} else if (GetUsbTaskState() == USB_STATE_ERROR) {
			if (!errorflag) {
				errorflag = 1;
				xil_printf("USB Error State\n");
				//print out string descriptor here
			}
		} else //not in USB running state
		{

			xil_printf("USB task state: ");
			xil_printf("%x\n", GetUsbTaskState());
			if (runningdebugflag) {	//previously running, reset USB hardware just to clear out any funky state, HS/FS etc
				runningdebugflag = 0;
				MAX3421E_init();
				USB_init();
			}
			errorflag = 0;
		}

	}
    cleanup_platform();
	return 0;
}
