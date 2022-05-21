//3-10-2022����Cap��C���ͨ�� Can 1 ��������

#include "SuperCap_comm.h"
#include "main.h"
#include "CAN_receive.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

void superCap_offline_proc(void);

//static CAN_TxHeaderTypeDef  superCap_tx_message;
static uint8_t              superCap_can_send_data[8];
superCap_info_t superCap_info1;
superCap_info_t superCap_info2;
superCap_info_t superCap_info3;//<----Խ��ָ��߽� ����λ��
superCap_info_t superCap_info4;
superCap_info_t superCap_info;//�õ���һ�� 3 4�������������
CAN_TxHeaderTypeDef  superCap_tx_message;

uint8_t debug_max_pwr;
uint8_t debug_fail_safe_pwr;
uint8_t debug_a=0;
uint8_t debug_b;
uint8_t debug_c;

void superCap_comm_bothway_init()
{
	/*
	��ʼ������:
	1 CAN��������
	2 CAN��������
	*/
	//1��ʼ������
	superCap_info.max_charge_pwr_command = 0;
	superCap_info.fail_safe_charge_pwr_command = 0;
	
	//2��ʼ������
	superCap_info.EBPct_fromCap = 0;
	superCap_info.VBKelvin_fromCap = 0;
	superCap_info.status = superCap_offline;
	superCap_info.data_EBPct_status = SuperCap_dataIsError;
	superCap_info.msg_u_EBPct.array[0] = 0;
	superCap_info.msg_u_EBPct.array[1] = 0;
	superCap_info.a = 0;
	superCap_info.b = 0;
	superCap_info.c = 0;
}

/*
SZL 3-10-2022 �·���SuperCap������
*/
void CAN_command_superCap(uint8_t max_pwr, uint8_t fail_safe_pwr)
{
		uint32_t send_mail_box;
    superCap_tx_message.StdId = RMTypeC_Master_Command_ID;
    superCap_tx_message.IDE = CAN_ID_STD;
    superCap_tx_message.RTR = CAN_RTR_DATA;
    superCap_tx_message.DLC = 0x08;
    superCap_can_send_data[0] = max_pwr;
    superCap_can_send_data[1] = fail_safe_pwr;
    superCap_can_send_data[2] = 0;
    superCap_can_send_data[3] = 0;
    superCap_can_send_data[4] = 0; 
    superCap_can_send_data[5] = 0; 
    superCap_can_send_data[6] = 0; 
    superCap_can_send_data[7] = 0; 
    HAL_CAN_AddTxMessage(&SUPERCAP_CAN, &superCap_tx_message, superCap_can_send_data, &send_mail_box);
}

//�����������
void superCap_offline_proc()
{
		superCap_info.status = superCap_offline;
		
	
}

bool_t superCap_is_data_error_proc()
{
		superCap_info.status = superCap_online;
	
		//ֻ�ж�EBPct_fromCap����Ϊֻ�����, �����0% ����0.0����
		if(superCap_info.EBPct_fromCap < 0.0f || superCap_info.EBPct_fromCap > 100.0f)
		{
			superCap_info.data_EBPct_status = SuperCap_dataIsError;
			return 1;
		}
		else
		{
			superCap_info.data_EBPct_status = SuperCap_dataIsCorrect;
			return 0;
		}
}

void superCap_solve_data_error_proc()
{
		if(superCap_info.data_EBPct_status == SuperCap_dataIsError)
		{
			if(superCap_info.EBPct_fromCap < 0.0f)
				superCap_info.EBPct_fromCap = 0.0f;
			if(superCap_info.EBPct_fromCap > 100.0f)
				superCap_info.EBPct_fromCap = 100.0f;
		}
}
