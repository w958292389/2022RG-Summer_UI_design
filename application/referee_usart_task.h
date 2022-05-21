
/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       referee_usart_task.c/h
  * @brief      RM referee system data solve. RM����ϵͳ���ݴ���
  * @note       
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Nov-11-2019     RM              1. done
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */
#ifndef REFEREE_USART_TASK_H
#define REFEREE_USART_TASK_H
#include "main.h"
#include "referee.h"
#define USART_RX_BUF_LENGHT     512
#define REFEREE_FIFO_BUF_LENGTH 1024
#define PC_FIFO_BUF_LENGTH 1024

/**
  * @brief          referee task
  * @param[in]      pvParameters: NULL
  * @retval         none
  */
/**
  * @brief          ����ϵͳ����
  * @param[in]      pvParameters: NULL
  * @retval         none
  */
extern void referee_usart_task(void const * argument);

/*
@brief         ������ͻ���������    ��UART1)
                Rui Peng 2021/2/25
								���԰汾
*/
void sendPack(uint8_t cmd_ID,uint8_t level, uint8_t robot_ID);
	
/*
@brief  ����CV����EE/ͨѶ����
*/


#define HEADER 0xaf //��ͷ
#define AUTOFIRE 0xff //�Զ�����
#define AUTOAIM 0x00 //�Զ���׼
#define MANUEL  0xfa //���ֶ�ģʽ
#define BIGBUFF 0xbb //����
#define SINBIGBUFF 0xaa //���ٴ��

#define LEVEL_I  0x01 //1��
#define LEVEL_II 0x02 //2��
#define LEVEL_III 0x03 //3��

#define ROBOTID_RED 0xff  //������������ɫ
#define ROBOTID_BLUE 0x00  //������������ɫ


/*DIY framheader*/
typedef __packed struct
{
	uint8_t  SOF;
	uint16_t DataLength;
	uint8_t  Seq;
	uint8_t  CRC8;
	
} xFrameHeader;

/* ID: 0x0100  Byte: 13 */
typedef __packed struct
{
	uint16_t CmdID;// 0x0100
	float data1;//floating value display
	float data2;
	float data3;
	uint8_t mask;//6 least significant match with 6 indicator 
	
} xShowData;
typedef __packed struct
{
	xFrameHeader   							txFrameHeader;//frame header
	uint16_t		 						CmdID;// commend id
	ext_student_interactive_data_t   dataFrameHeader;// data frameheader
	custom_data_t  					clientData;//
	uint16_t		 						FrameTail;//
}ext_SendClientData_t;
/*
@brief         Send data to referee for drawing the UI ��UART6)
                Ruizhe 2022/5/19
								test version
*/
void JUDGE_Show_Data(void);
/*
@brief         ����λ��ͨѶ  ����   �����ͣ� ��UART1)
                Rui Peng 2021/2/25
								���԰汾
*/
void sendData_Task_EE_To_PC(void);




/*
@brief         ��λ��ͨѶ ����  ���� ��ʼ����UART1)
                Rui Peng 2021/2/25
								���԰汾
*/
void pc_control_init(void);


/*
@brief         ��λ��ͨѶ ����  ��UART1)
                Rui Peng 2021/2/25
								���԰汾
*/

#define PACK_LENG 8
#define GIMBAL_MOVEMENT 1


struct gimbal_cmd
{
	uint8_t head;
	uint8_t id;
	int16_t pitch;
	int16_t yaw;
};

#endif
