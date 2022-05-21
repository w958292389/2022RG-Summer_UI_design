#ifndef __SUPERCAP_COMM_H___
#define __SUPERCAP_COMM_H___

#include "struct_typedef.h"
#include "CAN_receive.h"

#define SUPERCAP_CAN hcan1

extern uint8_t debug_max_pwr;
extern uint8_t debug_fail_safe_pwr;
extern void CAN_command_superCap(uint8_t max_pwr, uint8_t fail_safe_pwr);
extern void superCap_offline_proc(void);
extern bool_t superCap_is_data_error_proc(void);
extern void superCap_solve_data_error_proc(void);
extern void superCap_comm_bothway_init(void);

typedef enum
{
	//TypeC -> SuperCap ʱ CAN���� ��ָ���ID
	 RMTypeC_Master_Command_ID = 0x4FF,
	//SuperCap -> TypeCʱ CAN���� ���������ĵ�ID:
	 SuperCap_ID = 0x500,
}supercap_can_msg_id_e;

typedef enum
{
	superCap_offline,
	superCap_online
}superCap_connection_status_e;

typedef enum
{
	SuperCap_dataIsError,
	SuperCap_dataIsCorrect
}superCap_dataError_status_e;

typedef struct
{
	//����Cap������
	uint8_t max_charge_pwr_command;
	uint8_t fail_safe_charge_pwr_command;
	
	//���յ��� ����ת���������
	float EBPct_fromCap; //����������0J 0%-100%
	float VBKelvin_fromCap;//
	superCap_connection_status_e status;
	
	superCap_dataError_status_e data_EBPct_status;
	union superCap_msg_u
	{
		uint16_t msg_u;
		/* С��ģʽ��: msg_u_EBPct���ֽ�λ��Ӧ����a[1]; ���ֽ�λ��Ӧ����a[0] */
		uint8_t array[2];
	}msg_u_EBPct, msg_u_VBKelvin;
	
	uint8_t a;
	uint8_t b;
	uint8_t c;
	
}superCap_info_t;

extern superCap_info_t superCap_info;
extern uint8_t debug_a;
extern uint8_t debug_b;
extern uint8_t debug_c;

#endif /*__SUPERCAP_COMM_H___*/
