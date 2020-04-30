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

int x = 0, y = 0, z = 0; // переменные для хранения значений проекций вектора магнитного поля на Ох, Оу и Оz

/*===============================================================*/

/***********Настройка тактовой частоты************/
void OSC_init ( void ){
 
    MDR_RST_CLK->HS_CONTROL = 0x01;
	while ((MDR_RST_CLK->CLOCK_STATUS & 0x04) != 0x04);     
	MDR_RST_CLK->PLL_CONTROL = ((1 << 2) | (7 << 8)); 	

	while((MDR_RST_CLK->CLOCK_STATUS & 0x02) != 0x02);   

	MDR_RST_CLK->CPU_CLOCK  = (2                           
							 |(1 << 2)                   
							 |(1 << 4)                      
							 |(1 << 8));                   

	 
	MDR_RST_CLK->PER_CLOCK = 0xffffffff; // вкл. тактирования всей периферии МК
	 
}//void CPU_init
/*============================================================*/

/**********Инициализация передатчика UART***********/
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

/*********Отправка на ПК байта данных***********/
void uart2_send_byte(uint8_t byte)
{
	// Отправка байта данных
	// Ожидание готовности UART2 для передачи байта данных
	while(MDR_UART2->FR & (1 << 5)) {}
	MDR_UART2->DR = byte;					// отправка байта данных
}
 /*==========================================================*/

/*********Отправка на ПК числа с учетом знака*********/
void uart2_send_num(int32_t data){

	unsigned char buf[10], count=0;

	if (data < 0) // если отрицательное число
	{
		data = -data; // инвертируем число
		uart2_send_byte('-'); // выводим знак
	}

	if (data) // если положительное число
	{
		while (data)
	{ // разложение числа на отдельные цифры
	buf[count++] = data % 10 + '0';
	data /= 10;
}

	while (count) // вывод буфера отдельных чисел на дисплей
		uart2_send_byte(buf[--count]);
		uart2_send_byte(0x0A);
	}
	else uart2_send_byte('0'); // NaN
}
/*==============================================================*/
int main(){
	
	MDR_RST_CLK->PER_CLOCK = 0xffffffff; 		  // вкл. тактирования всей периферии МК
	//OSC_init();                                   
	delay_init(); 	                              // инициализация задержек
	
	I2C_Init();                                   // инициализация протокола I2C
	TLV493D_init();                               // инициализация датчика магнитного поля
	PORTB_Init();                                 // инициализация порта Б
	LCD_init(); 								  // инициализация дисплея
	LCD_clear(); 								  // очистка дисплея
	LCD_led_en; 	                              // включение подсветки дисплея
	
	STEP_DISABLE();                               // драйвер шагового двигателя отключен
	//STEP_ENABLE();                              // драйвер шагового двигателя включен
	DIR_RIGHT();                                  // вращение вала вправо
    //DIR_LEFT();                                 // вращение вала влево
	STEPPER_SPEED(1);                             // скорость вращения вала ШД (1 - 100)
    TLV493D_reset();                              // перезагрузка настроек датчика магнитного поля (рекомендовано в тех.документации)
	
	while(1){
		
	x = Get_Induc_X();                            // чтение компоненты Вх магнитного поля
	y = Get_Induc_Y();                            // чтение компоненты Ву магнитного поля
	z = Get_Induc_Z();	                          // чтение компоненты Вz магнитного поля
	  	
	LCD_set_cursor(0);                            // установка курсора на первую строку дисплея
	LCD_column_set(0);                            // установка курсора на первый столбец дисплея
	LCD_print_text("Bx: ");                       // вывод текста, указывающего на то, что выводятся значения компоненты Вх
	LCD_print_num(x);                             // вывод значения компоненты Вх магнитного поля
	LCD_column_set(25);                           // установка курсора на 15-ый столбец дисплея
  LCD_print_text("Gs");                           // вывод единиц измерения магнитного поля
		
    LCD_set_cursor(1);                            // установка курсора на вторую строку дисплея
	LCD_column_set(0);                            // установка курсора на первый столбец дисплея
	LCD_print_text("By: ");                       // вывод текста, указывающего на то, что выводятся значения компоненты Вy
	LCD_print_num(y);                             // вывод значения компоненты Вy магнитного поля
	LCD_column_set(25);                           // установка курсора на 15-ый столбец дисплея
  LCD_print_text("Gs");                           // вывод единиц измерения магнитного поля
		
    LCD_set_cursor(2);                            // установка курсора на третью строку дисплея
	LCD_column_set(0);                            // установка курсора на первый столбец дисплея
	LCD_print_text("Bz: ");                       // вывод текста, указывающего на то, что выводятся значения компоненты Вz
	LCD_print_num(y);                             // вывод значения компоненты Вz магнитного поля
	LCD_column_set(25);                           // установка курсора на 15-ый столбец дисплея
  LCD_print_text("Gs");                           // вывод единиц измерения магнитного поля
		
	uart2_send_num(x);                            // отправка на ПК значения компоненты Вх магнитного поля
	uart2_send_byte(0x09);                        // отправка символа Horizontal Tab \t
	
	uart2_send_num(y);                            // отправка на ПК значения компоненты Вy магнитного поля
	uart2_send_byte(0x09);                        // отправка символа Horizontal Tab \t
	
	uart2_send_num(z);                            // отправка на ПК значения компоненты Вz магнитного поля

		
	delay_ms(100);                                // задержка 100 мс
	//LCD_clear();	                              // обновление дисплея каждые 100 мс
	
	} //while(1)
	
	
} //main()