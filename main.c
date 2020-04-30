/*******Project Name: Magnetic field measurement (Lab #7)*******/
#include "MDR32Fx.h"                    // Device header
#include "MDR32F9Qx_port.h"             // Keil::Drivers:PORT
#include "MDR32F9Qx_rst_clk.h"          // Keil::Drivers:RST_CLK
#include "MDR32F9Qx_uart.h"             // Keil::Drivers:UART
#include "milkites_delay.h"
#include "milkites_display.h"
#include "milkites_i2c.h"
#include "tlv493d.h"
#include "pwm_step_motor.h"

/*===============================================================*/

int x = 0, y = 0, z = 0; // ���������� ��� �������� �������� �������� ������� ���������� ���� �� ��, �� � �z

/*===============================================================*/

/***********��������� �������� �������************/
void OSC_init ( void ){
 
    MDR_RST_CLK->HS_CONTROL = 0x01;
	while ((MDR_RST_CLK->CLOCK_STATUS & 0x04) != 0x04);     
	MDR_RST_CLK->PLL_CONTROL = ((1 << 2) | (7 << 8)); 	

	while((MDR_RST_CLK->CLOCK_STATUS & 0x02) != 0x02);   

	MDR_RST_CLK->CPU_CLOCK  = (2                           
							 |(1 << 2)                   
							 |(1 << 4)                      
							 |(1 << 8));                   

	 
	MDR_RST_CLK->PER_CLOCK = 0xffffffff; // ���. ������������ ���� ��������� ��
	 
}//void CPU_init
/*============================================================*/

/**********������������� ����������� UART***********/
void UART_ini(void){
	
	PORT_InitTypeDef GPIO_ini;
	UART_InitTypeDef uart_user_ini;
	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTD, ENABLE);
	
	GPIO_ini.PORT_Pin = PORT_Pin_1;
	GPIO_ini.PORT_OE = PORT_OE_OUT;
	GPIO_ini.PORT_PULL_UP = PORT_PULL_UP_OFF;
	GPIO_ini.PORT_PULL_DOWN = PORT_PULL_DOWN_OFF;
	GPIO_ini.PORT_PD_SHM = PORT_PD_SHM_OFF;
	GPIO_ini.PORT_PD = PORT_PD_DRIVER;
	GPIO_ini.PORT_GFEN = PORT_GFEN_OFF;
	GPIO_ini.PORT_FUNC = PORT_FUNC_ALTER;
	GPIO_ini.PORT_SPEED = PORT_SPEED_MAXFAST;
	GPIO_ini.PORT_MODE = PORT_MODE_DIGITAL;
	
	PORT_Init(MDR_PORTD, &GPIO_ini);
	
	RST_CLK_PCLKcmd(RST_CLK_PCLK_UART2, ENABLE);
	
	UART_BRGInit(MDR_UART2, UART_HCLKdiv1);
	
	uart_user_ini.UART_BaudRate = 115200;
	uart_user_ini.UART_FIFOMode = UART_FIFO_OFF;
	uart_user_ini.UART_HardwareFlowControl = UART_HardwareFlowControl_TXE;
	uart_user_ini.UART_Parity = UART_Parity_No;
	uart_user_ini.UART_StopBits = UART_StopBits1;
	uart_user_ini.UART_WordLength = UART_WordLength8b;
	
	UART_Init(MDR_UART2, &uart_user_ini);
	
	UART_Cmd(MDR_UART2, ENABLE);
}
/*=======================================================================*/

/*********�������� �� �� ����� ������***********/
void uart2_send_byte(uint8_t byte)
{
	// �������� ����� ������
	// �������� ���������� UART2 ��� �������� ����� ������
	while(MDR_UART2->FR & (1 << 5)) {}
	MDR_UART2->DR = byte;					// �������� ����� ������
}
 /*==========================================================*/

/*********�������� �� �� ����� � ������ �����*********/
void uart2_send_num(int32_t data){

	unsigned char buf[10], count=0;

	if (data < 0) // ���� ������������� �����
	{
		data = -data; // ����������� �����
		uart2_send_byte('-'); // ������� ����
	}

	if (data) // ���� ������������� �����
	{
		while (data)
	{ // ���������� ����� �� ��������� �����
	buf[count++] = data % 10 + '0';
	data /= 10;
}

	while (count) // ����� ������ ��������� ����� �� �������
		uart2_send_byte(buf[--count]);
		uart2_send_byte(0x0A);
	}
	else uart2_send_byte('0'); // NaN
}
/*==============================================================*/
int main(){
	
	MDR_RST_CLK->PER_CLOCK = 0xffffffff; 		  // ���. ������������ ���� ��������� ��
	//OSC_init();                                   
	delay_init(); 	                              // ������������� ��������
	
	I2C_Init();                                   // ������������� ��������� I2C
	TLV493D_init();                               // ������������� ������� ���������� ����
	PORTB_Init();                                 // ������������� ����� �
	LCD_init(); 								  // ������������� �������
	LCD_clear(); 								  // ������� �������
	LCD_led_en; 	                              // ��������� ��������� �������
	
	STEP_DISABLE();                               // ������� �������� ��������� ��������
	//STEP_ENABLE();                              // ������� �������� ��������� �������
	DIR_RIGHT();                                  // �������� ���� ������
    //DIR_LEFT();                                 // �������� ���� �����
	STEPPER_SPEED(1);                             // �������� �������� ���� �� (1 - 100)
    TLV493D_reset();                              // ������������ �������� ������� ���������� ���� (������������� � ���.������������)
	
	while(1){
		
	x = Get_Induc_X();                            // ������ ���������� �� ���������� ����
	y = Get_Induc_Y();                            // ������ ���������� �� ���������� ����
	z = Get_Induc_Z();	                          // ������ ���������� �z ���������� ����
	  	
	LCD_set_cursor(0);                            // ��������� ������� �� ������ ������ �������
	LCD_column_set(0);                            // ��������� ������� �� ������ ������� �������
	LCD_print_text("Bx: ");                       // ����� ������, ������������ �� ��, ��� ��������� �������� ���������� ��
	LCD_print_num(x);                             // ����� �������� ���������� �� ���������� ����
	LCD_column_set(25);                           // ��������� ������� �� 15-�� ������� �������
  LCD_print_text("Gs");                           // ����� ������ ��������� ���������� ����
		
    LCD_set_cursor(1);                            // ��������� ������� �� ������ ������ �������
	LCD_column_set(0);                            // ��������� ������� �� ������ ������� �������
	LCD_print_text("By: ");                       // ����� ������, ������������ �� ��, ��� ��������� �������� ���������� �y
	LCD_print_num(y);                             // ����� �������� ���������� �y ���������� ����
	LCD_column_set(25);                           // ��������� ������� �� 15-�� ������� �������
  LCD_print_text("Gs");                           // ����� ������ ��������� ���������� ����
		
    LCD_set_cursor(2);                            // ��������� ������� �� ������ ������ �������
	LCD_column_set(0);                            // ��������� ������� �� ������ ������� �������
	LCD_print_text("Bz: ");                       // ����� ������, ������������ �� ��, ��� ��������� �������� ���������� �z
	LCD_print_num(y);                             // ����� �������� ���������� �z ���������� ����
	LCD_column_set(25);                           // ��������� ������� �� 15-�� ������� �������
  LCD_print_text("Gs");                           // ����� ������ ��������� ���������� ����
		
	uart2_send_num(x);                            // �������� �� �� �������� ���������� �� ���������� ����
	uart2_send_byte(0x09);                        // �������� ������� Horizontal Tab \t
	
	uart2_send_num(y);                            // �������� �� �� �������� ���������� �y ���������� ����
	uart2_send_byte(0x09);                        // �������� ������� Horizontal Tab \t
	
	uart2_send_num(z);                            // �������� �� �� �������� ���������� �z ���������� ����

		
	delay_ms(100);                                // �������� 100 ��
	//LCD_clear();	                              // ���������� ������� ������ 100 ��
	
	} //while(1)
	
	
} //main()