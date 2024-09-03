/*  
Name: Tharshigan Vithiyananthan
Date: Apr.15, 2024
Description: Final 2DX3 Project - File adapted from COE2DX3 Studio 8C
*/
#include <stdint.h>
#include "PLL.h"
#include "SysTick.h"
#include "uart.h"
#include "onboardLEDs.h"
#include "tm4c1294ncpdt.h"
#include "VL53L1X_api.h"
#include "math.h"


#define I2C_MCS_ACK             0x00000008  // Data Acknowledge Enable
#define I2C_MCS_DATACK          0x00000008  // Acknowledge Data
#define I2C_MCS_ADRACK          0x00000004  // Acknowledge Address
#define I2C_MCS_STOP            0x00000004  // Generate STOP
#define I2C_MCS_START           0x00000002  // Generate START
#define I2C_MCS_ERROR           0x00000002  // Error
#define I2C_MCS_RUN             0x00000001  // I2C Master Enable
#define I2C_MCS_BUSY            0x00000001  // I2C Busy
#define I2C_MCR_MFE             0x00000010  // I2C Master Function Enable

#define MAXRETRIES              5           // number of receive attempts before giving up
void I2C_Init(void){
  SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;           													// activate I2C0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;          												// activate port B
  while((SYSCTL_PRGPIO_R&0x0002) == 0){};																		// ready?

    GPIO_PORTB_AFSEL_R |= 0x0C;           																	// 3) enable alt funct on PB2,3       0b00001100
    GPIO_PORTB_ODR_R |= 0x08;             																	// 4) enable open drain on PB3 only

    GPIO_PORTB_DEN_R |= 0x0C;             																	// 5) enable digital I/O on PB2,3
//    GPIO_PORTB_AMSEL_R &= ~0x0C;          																// 7) disable analog functionality on PB2,3

                                                                            // 6) configure PB2,3 as I2C
//  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00003300;
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00002200;    //TED
    I2C0_MCR_R = I2C_MCR_MFE;                      													// 9) master function enable
    I2C0_MTPR_R = 0b0000000000000101000000000111011;                       	// 8) configure for 100 kbps clock (added 8 clocks of glitch suppression ~50ns)
//    I2C0_MTPR_R = 0x3B;                                        						// 8) configure for 100 kbps clock
        
}

//The VL53L1X needs to be reset using XSHUT. Use PG0
void PortG_Init(void){
    //Use PortG0
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R6;                // activate clock for Port N
    while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R6) == 0){};    // allow time for clock to stabilize
    GPIO_PORTG_DIR_R &= 0x00;                                        // make PG0 in (HiZ)
  GPIO_PORTG_AFSEL_R &= ~0x01;                                     // disable alt funct on PG0
  GPIO_PORTG_DEN_R |= 0x01;                                        // enable digital I/O on PG0
                                                                                                    // configure PG0 as GPIO
  //GPIO_PORTN_PCTL_R = (GPIO_PORTN_PCTL_R&0xFFFFFF00)+0x00000000;
  GPIO_PORTG_AMSEL_R &= ~0x01;                                     // disable analog functionality on PN0

    return;
}

void PortH_Init(void){
	//Use PortH pins (PH0-PH3) for output
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R7;				// activate clock for Port H
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R7) == 0){};	// allow time for clock to stabilize
	GPIO_PORTH_DIR_R |= 0x0F;        								// configure Port H pins (PH0-PH3) as output
  GPIO_PORTH_AFSEL_R &= ~0x0F;     								// disable alt funct on Port H pins (PH0-PH3)
  GPIO_PORTH_DEN_R |= 0x0F;        								// enable digital I/O on Port H pins (PH0-PH3)
																									// configure Port H as GPIO
  GPIO_PORTH_AMSEL_R &= ~0x0F;     								// disable analog functionality on Port H	pins (PH0-PH3)	
	return;
}

void PortM_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R11;				// activate clock for Port M
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R11) == 0){};	// allow time for clock to stabilize
	GPIO_PORTM_DIR_R &= ~0x03;        								// configure Port M pins (PM0 and PM1) as input
  GPIO_PORTM_AFSEL_R &= ~0x03;     								// disable alt funct on Port M pins (PM0, PM1)
	//GPIO_PORTM_PCTL_R &= ~0x00000000;
	GPIO_PORTM_DEN_R |= 0x03;        								// enable digital I/O on Port M pins (PM0, PM1)
	
	GPIO_PORTM_AMSEL_R &= ~0x03;     								// disable analog functionality on Port M	pins (PM0, PM1)	
	return;
}

void PortE_Init(void){
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;             //Activate clock for Port E
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R4) == 0){};    //Allow clock to stabilize
	GPIO_PORTE_DIR_R |= 0x01;                            //Configure PE0 as output
	GPIO_PORTE_AFSEL_R |= ~0x01;
	GPIO_PORTE_DEN_R |= 0x01;                            //Enable PE0

	GPIO_PORTE_AMSEL_R |= ~0x01;	
	return;		
}

//XSHUT     This pin is an active-low shutdown input; 
//					the board pulls it up to VDD to enable the sensor by default. 
//					Driving this pin low puts the sensor into hardware standby. This input is not level-shifted.
void VL53L1X_XSHUT(void){
    GPIO_PORTG_DIR_R |= 0x01;                                        // make PG0 out
    GPIO_PORTG_DATA_R &= 0b11111110;                                 //PG0 = 0
    FlashAllLEDs();
    SysTick_Wait10ms(10);
    GPIO_PORTG_DIR_R &= ~0x01;                                            // make PG0 input (HiZ)
    
}

void spin(int dir, int steps){																			
	uint32_t delay = 2;															
	
	//CW motor rotation
	if(dir == 0){
		for(int i=0; i<steps/4; i++){									
			GPIO_PORTH_DATA_R = 0b00000011;
			GPIO_PORTE_DATA_R ^= 0b00000001; //Toggling Port E pin to see system clock 
			SysTick_Wait10ms(delay);											
			GPIO_PORTH_DATA_R = 0b00000110;
			GPIO_PORTE_DATA_R ^= 0b00000001;
			SysTick_Wait10ms(delay);
			GPIO_PORTH_DATA_R = 0b00001100;
			GPIO_PORTE_DATA_R ^= 0b00000001;
			SysTick_Wait10ms(delay);
			GPIO_PORTH_DATA_R = 0b00001001;
			GPIO_PORTE_DATA_R ^= 0b00000001;
			SysTick_Wait10ms(delay);
		}
	}
	
	//CCW motor rotation
	else{
		for(int i=0; i<steps/4; i++){
			GPIO_PORTH_DATA_R = 0b00001001;
			GPIO_PORTE_DATA_R ^= 0b00000001;
			SysTick_Wait10ms(delay);											
			GPIO_PORTH_DATA_R = 0b00001100;
			GPIO_PORTE_DATA_R ^= 0b00000001;
			SysTick_Wait10ms(delay);
			GPIO_PORTH_DATA_R = 0b00000110;
			GPIO_PORTE_DATA_R ^= 0b00000001;
			SysTick_Wait10ms(delay);
			GPIO_PORTH_DATA_R = 0b00000011;
			GPIO_PORTE_DATA_R ^= 0b00000001;
			SysTick_Wait10ms(delay);
		}
	}
	
}



//*********************************************************************************************************
//*********************************************************************************************************
//***********					MAIN Function				*****************************************************************
//*********************************************************************************************************
//*********************************************************************************************************
uint16_t	dev = 0x29;			//address of the ToF sensor as an I2C slave peripheral
int status=0;

int main(void) {
  uint8_t byteData, byteTwoData, sensorState=0, myByteArray[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , i=0;
  uint16_t wordData;
  uint16_t Distance;
  uint16_t SignalRate;
  uint16_t AmbientRate;
  uint16_t SpadNum; 
  uint8_t RangeStatus;
  uint8_t dataReady;
	int refSteps, input = 0, motorStatus, deviceStatus = 0, x_displacement = 0, missedSteps=1, missedDegrees, approxMeasure=0, numRescans = 0;
	double radians, z, y, currentDegrees, degrees = 11.25;

	//initialize
	PLL_Init();	
	SysTick_Init();
	onboardLEDs_Init();
	I2C_Init();
	UART_Init();
	PortH_Init();
	PortM_Init();
	PortE_Init();
	
	
	// hello world!
	UART_printf("Program Begins\r\n");


/* Those basic I2C read functions can be used to check your own I2C functions */
	status = VL53L1X_GetSensorId(dev, &wordData);

	sprintf(printf_buffer,"(Model_ID, Module_Type)=0x%x\r\n",wordData);
	UART_printf(printf_buffer);

	// 1 Wait for device booted
	while(sensorState==0){
		status = VL53L1X_BootState(dev, &sensorState);
		SysTick_Wait10ms(10);
  }
	FlashAllLEDs();
	UART_printf("ToF Chip Booted!\r\n Please Wait...\r\n");
	
	status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/
	
  /* 2 Initialize the sensor with the default setting  */
  status = VL53L1X_SensorInit(dev);
	Status_Check("SensorInit", status);

	
  /* 3 Optional functions to be used to change the main ranging parameters according the application requirements to get the best ranging performances */
  status = VL53L1X_SetDistanceMode(dev, 2); /* 1=short, 2=long */
  status = VL53L1X_SetTimingBudgetInMs(dev, 100); /* in ms possible values [20, 50, 100, 200, 500] */
  status = VL53L1X_SetInterMeasurementInMs(dev, 200); /* in ms, IM must be > = TB */

  // 4 Start Ranging
  status = VL53L1X_StartRanging(dev);
	
	//Do while loop to prevent program advancing until PM1 button pressed (Device is turned ON)
	do{
		if ((GPIO_PORTM_DATA_R & 0b00000010) == 0b10){
			deviceStatus ^= 1;
			
			UART_printf("DAQ ON!\r\n");
			//Button debouning			
			while((GPIO_PORTM_DATA_R & 0b00000010) == 0b10){};
		}
	}while(!deviceStatus);
	
	
	//While loop to continuously run program while device is "ON"
	while(deviceStatus){
		motorStatus = 0;
		
		//For loop for 1 full rotation
		for(int i=1; i<=512; i++){
			
			//Do while loop to check for button press and to stay in loop while motor off (scan not started)
			do{
				
				//If statement to check if PM1 button pressed (to turn "OFF" device)
				if ((GPIO_PORTM_DATA_R & 0b00000010) == 0b10){
					deviceStatus ^= 1;
					//Button debouning			
					while((GPIO_PORTM_DATA_R & 0b00000010) == 0b10){};
				}
				
				//If button press detected toggle flag to turn on/off the motor (to start/continue a scan)
				if ((GPIO_PORTM_DATA_R & 0b00000001) == 1){
					//Toggle flag to indicate motor as on/off
					motorStatus ^= 1;
					
					//Button debouncing					
					while((GPIO_PORTM_DATA_R & 0b00000001) == 1){};
				}
			}while((motorStatus == 0) & deviceStatus);
			
			//If device is "OFF" exit main while loop to end program
			if(!deviceStatus){
				break;
			}
			
			spin(0, 4); //Spin one full step in CCW
			
			//Calculating specific degrees for distance measurements in steps
			refSteps = 512/(360/degrees);
			
			//If statement to record distance at specified angle
			if (i % refSteps == 0){
				
				//Do while loop to rerun measurment sequence when invalid range status received
				do{
					numRescans = 0;
					//Do while to rescan at specific position a maximum of 3 times if invalid range status received
					do{
						//Wait until the ToF sensor's data is ready
						while (dataReady == 0){
							status = VL53L1X_CheckForDataReady(dev, &dataReady);
									//FlashLED3(1);
									VL53L1_WaitMs(dev, 5);
						}
						//Reset TOF ready flag to 0
						dataReady = 0;
					
					
						//Obtaining range status and distance measurement from TOF sensor
						status = VL53L1X_GetRangeStatus(dev, &RangeStatus);
						status = VL53L1X_GetDistance(dev, &Distance) ;	
							
						//Clear interrupt to enable next interrupt
						status = VL53L1X_ClearInterrupt(dev);
						
						//Sending error message to UART if invalid range status
						if(RangeStatus != 0){
							UART_printf("Error with TOF measurement, rescanning\r\n");
							sprintf(printf_buffer,"RangeStatus %i\r\n", RangeStatus);
							UART_printf(printf_buffer);
						}
						
						numRescans++;
						
						//Break out of loop once rescanned 3 times
						if (numRescans >= 3){
							break;
						}
						
					}while(RangeStatus != 0);
					
					//If invalid range status even after 3 scans
					if (RangeStatus == 7 || RangeStatus == 4 || RangeStatus == 2){
						
						UART_printf("Error with TOF measurement, rotating motor by 1 step and retrying TOF measurement\r\n");
						missedSteps++; // Keeping track of missed steps
						
						//Spin motor one full step
						spin(0, 4);
						//Increment i to ensure position relative to starting position is tracked
						i++;
						
						//If missing measurement occurs from faulty measurement sequence of rotating one step and remeasuring for 11.25 degrees
						if (missedSteps == refSteps-1){
							UART_printf("Regaining lost data point!\r\n");
							//Calculating specific angle where the measurement occured
							missedDegrees = i-refSteps-1;
							//Approximating distance to be used for coordinate calculations
							Distance = 4000;
							approxMeasure = 1;
							
							break;
						}
						
						FlashLED4(3); //Blink LED4 for faulty measurement status
					}
					
				}while(RangeStatus == 7 || RangeStatus == 4 || RangeStatus == 2);
				
				
				//If distance approximation occured, calculate XYZ using the angle at which the missing measurement occured
				if(approxMeasure){
					currentDegrees = missedDegrees;
					//radians = missedDegrees*3.14159265359/180;
				}
				
				else{
					currentDegrees = (i/512.0)*360;
				}
				
				approxMeasure = 0;
				
				//Flash LED3 to indicate measurement successfully taken
				FlashLED3(1);
				
				//Convert degrees to radians and calculate z-y components of distance measurement based on angle of motor
				radians = currentDegrees*3.14159265359/180;
				z = Distance*cos(radians);
				y = Distance*sin(radians);
				
				//Print the resulted readings to UART
				sprintf(printf_buffer,"%i %f %f\r\n", x_displacement, y, z); //CHANGE
				UART_printf(printf_buffer);
				SysTick_Wait10ms(50);
				
			}
			
			//Reset missed steps to 1
			missedSteps = 1;
		
		}
		//Increment x-coordinate by 50 cm (500mm)
		x_displacement += 500;
		
		//Reverse motor 360 degrees only when scan is finished
		if(deviceStatus){
			UART_printf("Resetting motor\r\n");
			spin(1, 2048);
		}
	}
	
	//Stop ranging
	VL53L1X_StopRanging(dev);
  
	//Send message to UART indicating that system is "OFF"
	UART_printf("DAQ OFF!\r\n");

}

