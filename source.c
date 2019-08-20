
#include <TM4C123GH6PM.h>
#include <TM4C123GH6PMX.h>	 /*TM4C123GH6PMX.h refers to header file with ports defined as 
			         volatile unsigned ints (e.g GPIO_PORTF_DATA_R) and not the pointer syntax (e.g GPIOF->DATA)  */

void SysTick_Init(void);       // initialize SysTick Timer                  
void SysTick_Wait(unsigned long delay1);
void SysTick_Wait10ms(unsigned long delay1);//
void manualGame(void);
void initializePorts(void);

//LCD Function prototypes
void LCD_init(void);
void LCD_Cmd(unsigned char command);
void LCD_Data(unsigned char data);
void LCD_writeString(char*);
void delay_micro(int);
void delay_milli(int);


typedef struct {
	unsigned long TrafficLights_portD;
	unsigned long TrafficLights_portF;
}Manual_States;


#define Go_West          0   //name of the states
#define Wait_West        1
#define Go_South         2
#define Wait_South       3

Manual_States Manual[] = {
	{0x0C,0x10},
	{0x42,0x10},
	{0x81,0x01},
	{0x42,0x10}
};

char* stateNames[]={
								" GO WEST",
								"	WAIT WEST",
								"	 GO SOUTH",
								"	WAIT SOUTH"
								};

unsigned long delay;
unsigned long S;  //index to the current state
unsigned long choice=0;


void initializePorts()
{
	SYSCTL_RCGC2_R=0x3B;        			//activate clock for port A,B,D,E,F	
	delay=SYSCTL_RCGC2_R;       		
	
	GPIO_PORTA_AMSEL_R=0x00;          //disable analog on port A
	GPIO_PORTA_PCTL_R=0x00000000;     //enable regular GPIO
	GPIO_PORTA_AFSEL_R=0x00;          //disable alternate function
	GPIO_PORTA_DEN_R=0xE0; 	
	GPIO_PORTA_DIR_R=0xE0;		  //enable output on PA2,3,5,6,7
	GPIO_PORTA_DATA_R=0x00;
	
	GPIO_PORTB_AMSEL_R=0x00;          //disable analog on port B
	GPIO_PORTB_PCTL_R=0x00000000;     //enable regular GPIO
	GPIO_PORTB_DIR_R=0xFF;            //outputs on PB0-7
	GPIO_PORTB_AFSEL_R=0x00;          //disable alternate function
	GPIO_PORTB_DEN_R=0xFF;            //enable data on PB0-5
	GPIO_PORTB_DATA_R=0x00;
	
	/*
	GPIO_PORTC_AMSEL_R=0x00;          //disable analog on port B
	GPIO_PORTC_PCTL_R=0x00000000;     //enable regular GPIO
	GPIO_PORTC_DIR_R |=0xC0;          //outputs on PC6-7
	GPIO_PORTC_AFSEL_R=0x00;          //disable alternate function
	GPIO_PORTC_DEN_R |=0xC0;           //enable data on PC6-7
	GPIO_PORTC_DATA_R=0x00; 
	*/
	
	GPIO_PORTE_AMSEL_R=0x00;          //disable analog on port E
	GPIO_PORTE_PCTL_R=0x00000000;     //enable regular GPIO	
	GPIO_PORTE_DIR_R=0x00;            //inputs on PE0-3
	GPIO_PORTE_AFSEL_R=0x00;          //disable alternate function
	GPIO_PORTE_DATA_R=0x00;
	GPIO_PORTE_DEN_R=0xF;            //enable digital I/O on PE0-2
	
	GPIO_PORTF_LOCK_R = 0x4C4F434B;
	GPIO_PORTF_CR_R = 0xFF;
	GPIO_PORTF_AMSEL_R=0x00;          //disable analog on port E
	GPIO_PORTF_PCTL_R=0x00000000;     //enable regular GPIO	
	GPIO_PORTF_DIR_R=0x1F;            //output on PF0,1,2,3,4
	GPIO_PORTF_AFSEL_R=0x00;          //disable alternate function
	GPIO_PORTF_DEN_R=0x1F;            //enable digital I/O on PF0-4
	GPIO_PORTF_DATA_R=0x00;
	//GPIO_PORTF_PUR_R = 0x10;         /* enable pull up for pin 4 */
	
	
	GPIO_PORTD_LOCK_R=0x4C4F434B;
	GPIO_PORTD_CR_R = 0xFF;
	GPIO_PORTD_AMSEL_R=0x00;          //disable analog on port E
	GPIO_PORTD_PCTL_R=0x00000000;     //enable regular GPIO	
	GPIO_PORTD_DIR_R=0xFF;            //outputs on PD0-7
	GPIO_PORTD_AFSEL_R=0x00;          //disable alternate function
	GPIO_PORTD_DEN_R=0xFF;            //enable digital I/O on PD1-7
	
	
 delay_milli(500);
}

int main(void) {
	volatile unsigned long delay;
  	SysTick_Init();
  	initializePorts();
	  LCD_init();
  	

	
	int returningFromManual=0;
	S=Go_West;
	
		while(S<4)
		{
			choice=(GPIO_PORTE_DATA_R&0x08);  //read sensors(edit: We've transferred the switches to Port E)
			
			if (choice == 0x08)
			{
				manualGame();
				returningFromManual = 1;
			}

			if (returningFromManual) {
				 S=0;
				 returningFromManual = 0;
				 GPIO_PORTF_DATA_R |= 0x02;
				 SysTick_Wait10ms(50);
				continue;
			}
	
		    LCD_Cmd(0x01);
	        LCD_writeString(stateNames[S]);
      		GPIO_PORTF_DATA_R |= 0x02;
			GPIO_PORTD_DATA_R=Manual[S].TrafficLights_portD;    //set car signal lights
			GPIO_PORTF_DATA_R&=0x0E;
			GPIO_PORTF_DATA_R|=Manual[S].TrafficLights_portF;
			SysTick_Wait10ms(200);
				
			S++;
			
			/*if(S == 1) //Wait West is our Challaning Signal
			{
				LCD_writeString("Challan Issued");
				delay_milli(3000);
				LCD_cmd(0x01);
				GPIO_PORTF_DATA_R=1;
				SysTick_Wait10ms(50);
				GPIO_PORTF_DATA_R=0x0;
			}*/
			if (S==4)
				S=0;
		
			SysTick_Wait(500);
		}

}

void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
  NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock
}
// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(unsigned long delay1){
  NVIC_ST_RELOAD_R = delay1-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}
// 800000*12.5ns equals 10ms
void SysTick_Wait10ms(unsigned long delay1){
  unsigned long i;
  for(i=0; i<delay1; i++){
    SysTick_Wait(800000);  // wait 10ms
  }
}

void manualGame() {
	GPIO_PORTF_DATA_R |= 0x04;
	while((GPIO_PORTE_DATA_R&0x08) == 0x08 ) //Since Auto to Manual Switch is on PE3, we need the ftn to work as long it is on. I think this logic is wrong wese
	{
		  
			S = (GPIO_PORTE_DATA_R&0x07);
			if (S<4)
			{
			    LCD_Cmd(0x01);
	      	    LCD_writeString(stateNames[S]);
		        GPIO_PORTD_DATA_R=Manual[S].TrafficLights_portD;    //set car signal lights
				GPIO_PORTF_DATA_R&=0x0E;
				GPIO_PORTF_DATA_R|=Manual[S].TrafficLights_portF;
				SysTick_Wait10ms(100);
		    }
	}
	
	GPIO_PORTF_DATA_R &= ~0x04;
	for(int i=0;i<3;++i) {
		SysTick_Wait10ms(10);
		GPIO_PORTF_DATA_R |= 0x02;
		SysTick_Wait10ms(10);
		GPIO_PORTF_DATA_R &= 0x00;
	}
	  SysTick_Wait10ms(10);
	
}

void delay_micro(int n)
{
	for(int i=0;i<n;++i)
	   for(int j=0;j<3;++j)
	      {}
}

void delay_milli(int n)
{
	for(int i=0;i<n;++i)
	   for(int j=0;j<3180;++j)
	      {}
}

void LCD_init(void)
{
	LCD_Cmd(0x38); //8-bits,2 display lines, 5x7 font
	LCD_Cmd(0x06); //increments automatically
	LCD_Cmd(0x0C); //Turn on display
	LCD_Cmd(0x01); //clear display
}
 
void LCD_Cmd(unsigned char command)
{
	GPIOA->DATA = 0x00; //RS =0, E=0, RW=0
	GPIOB->DATA =command;
	GPIOA->DATA =0x80; //E=1 to secure command
	delay_micro(0);
	GPIOA ->DATA =0x00;
	if(command <4) 
		delay_milli(2); 
	else 
		delay_micro(37); 
} 

void LCD_Data(unsigned char data) 
{
	GPIOA->DATA =0x20; //RS=1, E=0,RW=0
	GPIOB->DATA =data;
	GPIOA->DATA |= 0x80;
	GPIOA->DATA =0x00;
	delay_micro(0);
 
}

void LCD_writeString(char* str)
{
	while(*str) {
			LCD_Data(*str);
	    	delay_milli(5);
		  	str++;
	}
}


