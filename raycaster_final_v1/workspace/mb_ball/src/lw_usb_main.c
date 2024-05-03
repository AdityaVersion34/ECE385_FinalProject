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

//shift data for fixed-point arithmetic
#define SHIFT_BY 8
//will use u16 with 8 decimal points

#define INT_MAX 2147483647
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
    
	xil_printf("Main function entered \n");

	//making the game map
	u8 gameMap[15][15] = 
	{
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,2,0,2,0,2,2,0,2,0,1},
		{1,0,2,0,2,0,0,0,0,0,0,0,2,0,1},
		{1,0,2,0,0,0,0,0,0,0,2,0,2,0,1},
		{1,0,2,0,0,0,0,0,0,0,0,0,2,0,1},
		{1,0,2,0,0,0,0,0,0,0,0,0,2,0,1},
		{1,0,2,0,0,0,0,0,0,0,0,0,2,0,1},
		{1,0,2,0,0,0,0,0,0,2,0,0,2,0,1},
		{1,0,2,0,0,0,0,0,0,0,0,0,2,0,1},
		{1,0,2,2,2,2,0,0,2,2,2,2,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,2,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
	};

	//defining and initializing position and direction variables. camera plane vector is tied to the dir vect
	int xpos = 7 << SHIFT_BY;		//starts in the middle
	int ypos = 7 << SHIFT_BY;
	int xdir = 0 << SHIFT_BY;		//starts facing to the top. unit vector.
	int ydir = 1 << SHIFT_BY;
	int xcam = 1 << SHIFT_BY;
	int ycam = 0 << SHIFT_BY;

	//declaring more variables
	int scaledScreenPos;		//this variable stores the current screen position scaled on a -1 to 1 scale
	int castRayx;			//stores x and y pos of the current cast vector (not including player pos)
	int castRayy;

	int distFromx;		//distance from current cast ray position to the next x or y edge it will collide with
	int distFromy;

	int distToNextx;		//how far the cast ray has to be projected to produce a 1x or 1y step
	int distToNexty;

	int perpDistFromCam;	//used to calculate the perpendicular distance of the hit wall from the camera

	u8 isHit;		//stores the kind of wall that has been hit
	u8 whichSide;	//was the wall hit in x or y dir
	int stepDirx;	//which direction to step in for the DDA alg in both directions
	int stepDiry;

	int truncXpos;		//truncated x and y player positions - which map cell are we in
	int truncYpos;

	u16 currentColHeight;	//height of the current pixel column

	int framecounter = 0;	//test frame counter
	
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
		//xil_printf("-"); //A tick here means one loop through the USB main handler
		MAX3421E_Task();
		USB_Task();
		if (GetUsbTaskState() == USB_STATE_RUNNING) {	//operations within this condition - usb is polling
			if (!runningdebugflag) {
				xil_printf("runningdebugflag = 0 \n");
				runningdebugflag = 1;
				device = GetDriverandReport();
			} else if (device == 1) {		//within this - using keyboard not mouse
				//run keyboard debug polling
				rcode = kbdPoll(&kbdbuf);
				if (rcode == hrNAK) {
					//continue; //NAK means no new data
				} else if (rcode) {
					xil_printf("Rcode: ");
					xil_printf("%x \n", rcode);
					//continue;
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

				

				for(int i=0; i<monitorXdim; i++){
					
					//xil_printf("iter \n");
					isHit = 0;

					scaledScreenPos = 2 * ((i << SHIFT_BY) / (monitorXdim)) - (1 << SHIFT_BY);		//setting ssp based on curr screen pos
					
					//xil_printf("ssp: %d \n", (scaledScreenPos));

					castRayx = xdir + ((xcam*scaledScreenPos)>>SHIFT_BY);
					castRayy = ydir + ((ycam*scaledScreenPos)>>SHIFT_BY);

					//xil_printf("castray x: %d y: %d \n", castRayx, castRayy);
					
					truncXpos = xpos>>SHIFT_BY;		//updating current cell we're in
					truncYpos = ypos>>SHIFT_BY;

					//these represent the distance the ray has to cover to travel 1 unit in x and y dist
					//condition to prevent division by zero, setting it to very large value
					//shifted from this
					distToNextx = (castRayx == 0) ? INT_MAX : abs((1<<(SHIFT_BY*2))/castRayx);
					distToNexty = (castRayy == 0) ? INT_MAX : abs((1<<(SHIFT_BY*2))/castRayy);

					//xil_printf("before if conds \n");

					//initializing which direction we're going in and initial relative distFrom values
					if(castRayx < 0){		//for x axis
						stepDirx = -1;
						distFromx = ((xpos - truncXpos<<SHIFT_BY)*distToNextx)>>SHIFT_BY;
					}else{
						stepDirx = 1;
						distFromx = ((1<<SHIFT_BY + (truncXpos<<SHIFT_BY - xpos))*distToNextx)>>SHIFT_BY;
					}

					if(castRayy < 0){
						stepDiry = -1;
						distFromy = ((ypos - truncYpos<<SHIFT_BY)*distToNexty)>>SHIFT_BY;
					}else{
						stepDiry = 1;
						distFromy = ((1<<SHIFT_BY + (truncYpos<<SHIFT_BY - ypos))*distToNexty)>>SHIFT_BY;
					}

					//truncXpos<<SHIFT_BY;
					//truncYpos<<SHIFT_BY;
					//xil_printf("before ray loop \n");
					//xil_printf("distFromx: %d\ndistFromy: %d\ndistToNextx: %d\ndistToNexty: %d\n", distFromx, distFromy, distToNextx, distToNexty);
					//loop through ray, until we hit a wall
					while(isHit == 0){
						//jump based on which axis grid side is closer
						if(distFromx < distFromy){
							distFromx += distToNextx;
							truncXpos += stepDirx;
							whichSide = 0;
						}else{
							distFromy += distToNexty;
							truncYpos += stepDiry;
							whichSide = 1;
						}
						//xil_printf("trunc pos (x,y): %d, %d\n", truncXpos, truncYpos);
						//check if the truncated ray position is in a solid map area
						if(gameMap[truncYpos][truncXpos] > 0){
							isHit = gameMap[truncYpos][truncXpos];
						}
					}

					//xil_printf("post loop stuff \n");

					//calculating perpendicular distance of wall to camera to obtain correct dist
					//refer lodev page for derivation
					if(whichSide == 0){
						perpDistFromCam = distFromx - distToNextx;
					}else{
						perpDistFromCam = distFromy - distToNexty;
					}

					//if collision is more common than expected, will add condition for div by 0
					currentColHeight = (u16)((monitorYdim<<SHIFT_BY)/perpDistFromCam);

					//xil_printf("ypos: %d, xpos: %d, hitwall: %d \n", (truncYpos), (truncXpos), isHit);
					xil_printf("colheight: %d\nhitwall: %d\n", currentColHeight, isHit);
					
				}

				

				//frame is complete, increment frame counter
				framecounter++;
				if(framecounter%60 == 0){
					xil_printf("<60 frames> \n");
				}
			}

			else if (device == 2) {
				xil_printf("THIS SHOULD NOT BE HAPPENING \n");
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
			xil_printf("error state \n");
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
