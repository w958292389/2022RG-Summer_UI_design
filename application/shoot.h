/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       shoot.c/h
  * @brief      ������ܡ�
  * @note       
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. ���
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */

#ifndef SHOOT_H
#define SHOOT_H
#include "struct_typedef.h"

#include "CAN_receive.h"
#include "gimbal_task.h"
#include "remote_control.h"
#include "user_lib.h"



//������俪��ͨ������
#define SHOOT_RC_MODE_CHANNEL       1
//��̨ģʽʹ�õĿ���ͨ��

#define SHOOT_CONTROL_TIME          GIMBAL_CONTROL_TIME

#define SHOOT_FRIC_PWM_ADD_VALUE    200.0f

//���Ħ���ּ���� �ر�
#define SHOOT_ON_KEYBOARD           KEY_PRESSED_OFFSET_Q
#define SHOOT_OFF_KEYBOARD          KEY_PRESSED_OFFSET_E

//�����ɺ� �ӵ�����ȥ���ж�ʱ�䣬�Է��󴥷�
#define SHOOT_DONE_KEY_OFF_TIME     15
//��곤���ж�
#define PRESS_LONG_TIME             80
//ң����������ش��µ�һ��ʱ��� ���������ӵ� �����嵥
#define RC_S_LONG_TIME              2000
//Ħ���ָ��� ���� ʱ��
#define UP_ADD_TIME                 80
//�����������ֵ��Χ
#define HALF_ECD_RANGE              4096
#define ECD_RANGE                   8191
//���rmp �仯�� ��ת�ٶȵı���
//rpm to rad/s
#define MOTOR_RPM_TO_SPEED          0.00290888208665721596153948461415f
#define MOTOR_ECD_TO_ANGLE          0.000021305288720633905968306772076277f
#define FULL_COUNT                  18
//�����ٶ�
#define TRIGGER_SPEED               10.0f
#define CONTINUE_TRIGGER_SPEED      9.0f
#define READY_TRIGGER_SPEED         5.0f

#define KEY_OFF_JUGUE_TIME          500
#define SWITCH_TRIGGER_ON           0
#define SWITCH_TRIGGER_OFF          1

//����ʱ�� �Լ���תʱ��
#define BLOCK_TRIGGER_SPEED         1.0f
#define BLOCK_TIME                  700
#define REVERSE_TIME                500
#define REVERSE_SPEED_LIMIT         13.0f

#define PI_FOUR                     0.78539816339744830961566084581988f
#define PI_TEN                      0.69f//initial 0.314 radian,0.69 is approximately 40 degree

//�����ֵ��PID
#define TRIGGER_ANGLE_PID_KP        800.0f
#define TRIGGER_ANGLE_PID_KI        0.5f
#define TRIGGER_ANGLE_PID_KD        0.0f

#define TRIGGER_BULLET_PID_MAX_OUT  10000.0f
#define TRIGGER_BULLET_PID_MAX_IOUT 9000.0f

#define TRIGGER_READY_PID_MAX_OUT   10000.0f
#define TRIGGER_READY_PID_MAX_IOUT  7000.0f


#define SHOOT_HEAT_REMAIN_VALUE     30

/*
12-28-2021 SZL��� PID M3508 ƨ�� shooter ��� 2��
���䷽����Left ��Right������������ױ���+�궨�壬һ����ֵ����һ��
��ΪCan ID 1 ��ΪCan ID2
M3508_RIGHT_FRICTION_PID_MAX_OUT = M3508_LEFT_FRICTION_PID_MAX_OUT = TRIGGER_READY_PID_MAX_OUT Լ���� MAX_MOTOR_CAN_CURRENT 16000.0f
*/
//����3508���can���͵���ֵ 16384-->20A
//#define MAX_MOTOR_CAN_CURRENT 16000.0f

//LEFT
#define M3508_LEFT_FRICTION_PID_KP 800.0f
#define M3508_LEFT_FRICTION_PID_KI 10.0f
#define M3508_LEFT_FRICTION_PID_KD 600.0f 

#define M3508_LEFT_FRICTION_PID_MAX_OUT 10000.0f//10000
#define M3508_LEFT_FRICTION_PID_MAX_IOUT 2000.0f

//RIGHT
#define M3508_RIGHT_FRICTION_PID_KP 800.0f //800 //900
#define M3508_RIGHT_FRICTION_PID_KI 10.0f //10 //20
#define M3508_RIGHT_FRICTION_PID_KD 600.0f //600 //600

#define M3508_RIGHT_FRICTION_PID_MAX_OUT 10000.0f
#define M3508_RIGHT_FRICTION_PID_MAX_IOUT 2000.0f

#define M3508_FRIC_MOTOR_RPM_TO_LINEAR_VETOR_SEN 3.141592654e-3f
//

typedef enum
{
    SHOOT_STOP = 0,
    SHOOT_READY_FRIC,    //1
    SHOOT_READY_BULLET,  //2
    SHOOT_READY,         //3
    SHOOT_BULLET,        //4
    SHOOT_CONTINUE_BULLET,  //5
    SHOOT_DONE,          //6
} shoot_mode_e;

//SZL 12-30-2021 ��� fric ��� M3508 ���ݽ�� ������������� �ṹ��
//fric Wheel
typedef struct
{
		fp32 fricW_speed_set;
	  fp32 fricW_speed;
	
    //fp32 fricW_angle;
    //fp32 fricW_set_angle;
    //int8_t fricW_ecd_count;
		
		int16_t fricW_given_current;
} M3508_fric_motor_t;

typedef struct
{
    shoot_mode_e shoot_mode;
    const RC_ctrl_t *shoot_rc;
    const motor_measure_t *shoot_motor_measure;
    ramp_function_source_t fric1_ramp;
    uint16_t fric_pwm1;
    ramp_function_source_t fric2_ramp;
    uint16_t fric_pwm2;
    pid_type_def trigger_motor_pid;
    fp32 trigger_speed_set;
    fp32 speed;
    fp32 speed_set;
    fp32 angle;
    fp32 set_angle;
    int16_t given_current;
    int8_t ecd_count;

    bool_t press_l;
    bool_t press_r;
    bool_t last_press_l;
    bool_t last_press_r;
    uint16_t press_l_time;
    uint16_t press_r_time;
    uint16_t rc_s_time;

    uint16_t block_time;
    uint16_t reverse_time;
    bool_t move_flag;

    bool_t key;
    uint8_t key_time;

    uint16_t heat_limit;
    uint16_t heat;
		
		/*12-28-2021 SZL add for 
		infantry pid shooter friction wheel LEFT and RIGHT
		Everything above keep the same as the old PWM shooter
		*/
		const motor_measure_t *left_friction_motor_measure;
		const motor_measure_t *right_friction_motor_measure;
		pid_type_def left_fric_motor_pid;
		pid_type_def right_fric_motor_pid;
		
		//LEFT and RIGHT
		M3508_fric_motor_t left_fricMotor;
		M3508_fric_motor_t right_fricMotor;
		
		fp32 currentLeft_speed_set;
		fp32 currentRight_speed_set;
		
		fp32 currentLIM_shoot_speed;
		
} shoot_control_t;

//shoot motor �� ������ M2006 motor

extern uint8_t shootCommand; //�Զ�����ָ��  0x00 = ͣ��  0xff = ����
extern uint8_t autoAimFlag; //�Զ���׼����״̬ 0�� 1�Զ���׼

//�����������̨ʹ��ͬһ��can��id��Ҳ�����������̨������ִ��
extern void shoot_init(void);
extern int16_t shoot_control_loop(void);

#endif
