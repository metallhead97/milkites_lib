
#define USE_JTAG_A // ������������ JTAG_A �� ����� �

 void STEPPER_SPEED( uint32_t PWM_speed ); // ������� ������������� �������� �������� ���� ��
 void STEP_ENABLE(void); // ������� ��������� �������� ��
 void STEP_DISABLE(void); // ������� ���������� �������� ��
 void DIR_LEFT(void); // ������� ������ ����������� �������� ���� ����� (� ����� ������)
 void DIR_RIGHT(void); // ������� ������ ����������� �������� ������ (� ����� �����) (������������ ���� ���� ���������)
 
 void PORTB_Init();
