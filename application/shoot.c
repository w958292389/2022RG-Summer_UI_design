/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       shoot.c/h
  * @brief      �������.
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

#include "shoot.h"
#include "main.h"

#include "cmsis_os.h"

#include "bsp_laser.h"
#include "bsp_fric.h"
#include "arm_math.h"
#include "user_lib.h"
#include "referee.h"

#include "CAN_receive.h"
#include "gimbal_behaviour.h"
#include "detect_task.h"
#include "pid.h"
#include "referee_usart_task.h"

#define shoot_fric1_on(pwm) fric1_on((pwm)) //Ħ����1pwm�궨��
#define shoot_fric2_on(pwm) fric2_on((pwm)) //Ħ����2pwm�궨��
#define shoot_fric_off()    fric_off()      //�ر�����Ħ����

#define shoot_laser_on()    laser_on()      //���⿪���궨��
#define shoot_laser_off()   laser_off()     //����رպ궨��
//΢������IO
#define BUTTEN_TRIG_PIN HAL_GPIO_ReadPin(BUTTON_TRIG_GPIO_Port, BUTTON_TRIG_Pin)




/**
  * @brief          ���״̬�����ã�ң�����ϲ�һ�ο��������ϲ��رգ��²�1�η���1�ţ�һֱ�����£���������䣬����3min׼��ʱ�������ӵ�
  * @param[in]      void
  * @retval         void
  */
static void shoot_set_mode(void);
/**
  * @brief          ������ݸ���
  * @param[in]      void
  * @retval         void
  */
static void shoot_feedback_update(void);

/**
  * @brief          ��ת��ת����
  * @param[in]      void
  * @retval         void
  */
static void trigger_motor_turn_back(void);

/**
  * @brief          ������ƣ����Ʋ�������Ƕȣ����һ�η���
  * @param[in]      void
  * @retval         void
  */
static void shoot_bullet_control(void);



shoot_control_t shoot_control;          //�������


int16_t temp_rpm_left;
int16_t temp_rpm_right;

fp32 temp_speed_setALL = 12.5;

/**
  * @brief          �����ʼ������ʼ��PID��ң����ָ�룬���ָ��
  * @param[in]      void
  * @retval         ���ؿ�
  */
void shoot_init(void)
{

    static const fp32 Trigger_speed_pid[3] = {TRIGGER_ANGLE_PID_KP, TRIGGER_ANGLE_PID_KI, TRIGGER_ANGLE_PID_KD};
    shoot_control.shoot_mode = SHOOT_STOP;
    //ң����ָ��
    shoot_control.shoot_rc = get_remote_control_point();
    //���ָ��
    shoot_control.shoot_motor_measure = get_trigger_motor_measure_point();
    //��ʼ��PID
    PID_init(&shoot_control.trigger_motor_pid, PID_POSITION, Trigger_speed_pid, TRIGGER_READY_PID_MAX_OUT, TRIGGER_READY_PID_MAX_IOUT);
    //��������
    shoot_feedback_update();
    ramp_init(&shoot_control.fric1_ramp, SHOOT_CONTROL_TIME * 0.001f, FRIC_DOWN, FRIC_OFF);
    ramp_init(&shoot_control.fric2_ramp, SHOOT_CONTROL_TIME * 0.001f, FRIC_DOWN, FRIC_OFF);
    shoot_control.fric_pwm1 = FRIC_OFF;
    shoot_control.fric_pwm2 = FRIC_OFF;
    shoot_control.ecd_count = 0;
    shoot_control.angle = shoot_control.shoot_motor_measure->ecd * MOTOR_ECD_TO_ANGLE;
    shoot_control.given_current = 0;
    shoot_control.move_flag = 0;
    shoot_control.set_angle = shoot_control.angle;
    shoot_control.speed = 0.0f;
    shoot_control.speed_set = 0.0f;
    shoot_control.key_time = 0;
		
		/*12-28-2021 SZL add for 
		infantry pid shooter friction wheel LEFT and RIGHT
		Everything above keep the same as the old PWM shooter
		*/
		//��ʼ�������������
		shoot_control.currentLeft_speed_set = 0;
		shoot_control.currentRight_speed_set = 0;
		shoot_control.currentLIM_shoot_speed = 0;
		
		
		//LEFT friction PID const init
		static const fp32 Left_friction_speed_pid[3] = {M3508_LEFT_FRICTION_PID_KP, M3508_LEFT_FRICTION_PID_KI, M3508_LEFT_FRICTION_PID_KD};
		//RIGHT friction PID const init
		static const fp32 Right_friction_speed_pid[3] = {M3508_RIGHT_FRICTION_PID_KP, M3508_RIGHT_FRICTION_PID_KI, M3508_RIGHT_FRICTION_PID_KD};

		//���ָ�� M3508ƨ�� ����Ħ����
		shoot_control.left_friction_motor_measure = get_left_friction_motor_measure_point();
		shoot_control.right_friction_motor_measure = get_right_friction_motor_measure_point();
		
		//��ʼ��PID
		PID_init(&shoot_control.left_fric_motor_pid, PID_POSITION, Left_friction_speed_pid, M3508_LEFT_FRICTION_PID_MAX_OUT, M3508_LEFT_FRICTION_PID_MAX_IOUT);
		PID_init(&shoot_control.right_fric_motor_pid, PID_POSITION, Right_friction_speed_pid, M3508_RIGHT_FRICTION_PID_MAX_OUT, M3508_RIGHT_FRICTION_PID_MAX_IOUT);
	
}

/**
  * @brief          ���ѭ��
  * @param[in]      void
  * @retval         ����can����ֵ
  */

//===============================================
uint8_t robot_Level = 0;

int16_t shoot_control_loop(void)
{

    shoot_set_mode();        //����״̬��
    shoot_feedback_update(); //��������
	  robot_Level = get_robot_level();

	 	 if(robot_Level == 0){
	  shoot_control.fric1_ramp.max_value = FRIC_LV1;
    shoot_control.fric2_ramp.max_value = FRIC_LV1;
	 }else if(robot_Level == 1){
	  shoot_control.fric1_ramp.max_value = FRIC_LV1;
    shoot_control.fric2_ramp.max_value = FRIC_LV1;
	 }else if(robot_Level == 2){
	 	shoot_control.fric1_ramp.max_value = FRIC_LV2;
    shoot_control.fric2_ramp.max_value = FRIC_LV2;
	 }else if(robot_Level == 3){
	 	shoot_control.fric1_ramp.max_value = FRIC_LV3;
    shoot_control.fric2_ramp.max_value = FRIC_LV3;
	 }else{
	 	shoot_control.fric1_ramp.max_value = FRIC_LV1;
    shoot_control.fric2_ramp.max_value = FRIC_LV1;
	 }
	 
	 //------------------�޸ĵȼ��ж�
	 shoot_control.currentLIM_shoot_speed = temp_speed_setALL;
	
    if (shoot_control.shoot_mode == SHOOT_STOP)
    {
        //���ò����ֵ��ٶ�
        shoot_control.speed_set = 0;
    }
    else if (shoot_control.shoot_mode == SHOOT_READY_FRIC)
    {
        //���ò����ֵ��ٶ�
        shoot_control.speed_set = 0;
    }
    else if(shoot_control.shoot_mode ==SHOOT_READY_BULLET)
    {
        if(shoot_control.key == SWITCH_TRIGGER_OFF)
        {
            //���ò����ֵĲ����ٶ�,��������ת��ת����
            shoot_control.trigger_speed_set = READY_TRIGGER_SPEED;
            trigger_motor_turn_back();
        }
        else
        {
            shoot_control.trigger_speed_set = 0.0f;
            shoot_control.speed_set = 0.0f;
        }
        shoot_control.trigger_motor_pid.max_out = TRIGGER_READY_PID_MAX_OUT;
        shoot_control.trigger_motor_pid.max_iout = TRIGGER_READY_PID_MAX_IOUT;
    }
    else if (shoot_control.shoot_mode == SHOOT_READY)
    {
        //���ò����ֵ��ٶ�
         shoot_control.speed_set = 0.0f;
    }
    else if (shoot_control.shoot_mode == SHOOT_BULLET)
    {
        shoot_control.trigger_motor_pid.max_out = TRIGGER_BULLET_PID_MAX_OUT;
        shoot_control.trigger_motor_pid.max_iout = TRIGGER_BULLET_PID_MAX_IOUT;
        shoot_bullet_control();
    }
    else if (shoot_control.shoot_mode == SHOOT_CONTINUE_BULLET)
    {
        //���ò����ֵĲ����ٶ�,��������ת��ת����
        shoot_control.trigger_speed_set = CONTINUE_TRIGGER_SPEED;
        trigger_motor_turn_back();
    }
    else if(shoot_control.shoot_mode == SHOOT_DONE)
    {
        shoot_control.speed_set = 0.0f;
    }

    if(shoot_control.shoot_mode == SHOOT_STOP)
    {
        shoot_laser_off();
        shoot_control.given_current = 0;
        //Ħ������Ҫһ����б������������ͬʱֱ�ӿ�����������ܵ����ת
        ramp_calc(&shoot_control.fric1_ramp, -SHOOT_FRIC_PWM_ADD_VALUE);
        ramp_calc(&shoot_control.fric2_ramp, -SHOOT_FRIC_PWM_ADD_VALUE);
			/*
			shoot_control.fric_pwm1 = FRIC_OFF;//no need for ramp_calc now
			shoot_control.fric_pwm2 = FRIC_OFF;
			Ϊʲôû�����
			*/
			
			//SZL����, Ҳ����ʹ��б������ ��ͨ�˲�
			shoot_control.currentLeft_speed_set = M3508_FRIC_STOP;
			shoot_control.currentRight_speed_set = M3508_FRIC_STOP;
    }
    else
    {
        shoot_laser_on(); //���⿪��
        //���㲦���ֵ��PID
        PID_calc(&shoot_control.trigger_motor_pid, shoot_control.speed, shoot_control.speed_set);
        shoot_control.given_current = (int16_t)(shoot_control.trigger_motor_pid.out);
        if(shoot_control.shoot_mode < SHOOT_READY_BULLET)
        {
            shoot_control.given_current = 0;
        }
        //Ħ������Ҫһ����б������������ͬʱֱ�ӿ�����������ܵ����ת
        ramp_calc(&shoot_control.fric1_ramp, SHOOT_FRIC_PWM_ADD_VALUE);
        ramp_calc(&shoot_control.fric2_ramp, SHOOT_FRIC_PWM_ADD_VALUE);
				
				//SZL����, Ҳ����ʹ��б������ ��ͨ�˲�
				shoot_control.currentLeft_speed_set = shoot_control.currentLIM_shoot_speed;
				shoot_control.currentRight_speed_set = shoot_control.currentLIM_shoot_speed;

    }

    shoot_control.fric_pwm1 = (uint16_t)(shoot_control.fric1_ramp.out + 19);
    shoot_control.fric_pwm2 = (uint16_t)(shoot_control.fric2_ramp.out);
		
		
		
    shoot_fric1_on(shoot_control.fric_pwm1);
    shoot_fric2_on(shoot_control.fric_pwm2);
		
		//vTaskDelay(5);
		
		//M3508_fric_wheel_spin_control(-tempLeft_speed_set, tempRight_speed_set);
		M3508_fric_wheel_spin_control(-shoot_control.currentLeft_speed_set, shoot_control.currentRight_speed_set);
		
    return shoot_control.given_current;
}




/**
  * @brief          ���״̬�����ã�ң�����ϲ�һ�ο��������ϲ��رգ��²�1�η���1�ţ�һֱ�����£���������䣬����3min׼��ʱ�������ӵ�
  * @param[in]      void
  * @retval         void
  */
static void shoot_set_mode(void)
{
    static int8_t last_s = RC_SW_UP;

    //�ϲ��жϣ� һ�ο������ٴιر�
    if ((switch_is_up(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !switch_is_up(last_s) && shoot_control.shoot_mode == SHOOT_STOP))
    {
        shoot_control.shoot_mode = SHOOT_READY_FRIC;
    }
    else if ((switch_is_up(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !switch_is_up(last_s) && shoot_control.shoot_mode != SHOOT_STOP))
    {
        shoot_control.shoot_mode = SHOOT_STOP;
    }

    //�����е��� ����ʹ�ü��̿���Ħ����
    if (switch_is_mid(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && (shoot_control.shoot_rc->key.v & SHOOT_ON_KEYBOARD) && shoot_control.shoot_mode == SHOOT_STOP)
    {
        shoot_control.shoot_mode = SHOOT_READY_FRIC; 
    }
    //�����е��� ����ʹ�ü��̹ر�Ħ����
    else if (switch_is_mid(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && (shoot_control.shoot_rc->key.v & SHOOT_OFF_KEYBOARD) && shoot_control.shoot_mode != SHOOT_STOP)
    {
        shoot_control.shoot_mode = SHOOT_STOP;
    }

    if(shoot_control.shoot_mode == SHOOT_READY_FRIC && shoot_control.fric1_ramp.out == shoot_control.fric1_ramp.max_value && shoot_control.fric2_ramp.out == shoot_control.fric2_ramp.max_value)
    {
        shoot_control.shoot_mode = SHOOT_READY_BULLET; //��Ħ�������Ԥ��
    }
    else if(shoot_control.shoot_mode == SHOOT_READY_BULLET && shoot_control.key == SWITCH_TRIGGER_ON)
    {
        shoot_control.shoot_mode = SHOOT_READY;  
    }
    else if(shoot_control.shoot_mode == SHOOT_READY && shoot_control.key == SWITCH_TRIGGER_OFF)
    {
        shoot_control.shoot_mode = SHOOT_READY_BULLET;
    }
    else if(shoot_control.shoot_mode == SHOOT_READY)
    {
        //�²�һ�λ�����갴��һ�Σ��������״̬
        if ((switch_is_down(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !switch_is_down(last_s)) || (shoot_control.press_l && shoot_control.last_press_l == 0))
        {
            shoot_control.shoot_mode = SHOOT_BULLET;
        }
    }
    else if(shoot_control.shoot_mode == SHOOT_DONE)
    {
			 if(1)  //pr �Ķ� ����΢������
       // if(shoot_control.key == SWITCH_TRIGGER_OFF)
        {
            shoot_control.key_time++;
            if(shoot_control.key_time > SHOOT_DONE_KEY_OFF_TIME)
            {
                shoot_control.key_time = 0;
                shoot_control.shoot_mode = SHOOT_READY_BULLET;
            }
        }
        else
        {
            shoot_control.key_time = 0;
            shoot_control.shoot_mode = SHOOT_BULLET;
        }
    }
/*
    if(shoot_control.shoot_mode > SHOOT_READY_FRIC){ //�Զ�����ָ���
		   if(shootCommand == 0xff){
			 shoot_control.shoot_mode = SHOOT_CONTINUE_BULLET;
			 }else if(shootCommand == 0x00){
			 shoot_control.shoot_mode = SHOOT_READY_BULLET;
			 }
		}
	*/
		extern uint8_t autoAimFlag; //�Զ���׼����״̬ 0�� 1�Զ���׼
		
		if(shoot_control.shoot_rc->key.v & KEY_PRESSED_OFFSET_X) //press X to turn on auto aim
		{
			autoAimFlag = 1;
		}else if(shoot_control.shoot_rc->key.v & KEY_PRESSED_OFFSET_C) // press C to turn off auto aim
		{
		  autoAimFlag = 0;
		}
		

    if(shoot_control.shoot_mode > SHOOT_READY_FRIC)
    {
        //��곤��һֱ�������״̬ ��������
        if (((shootCommand == 0xff) && (autoAimFlag == 1))|| (shoot_control.press_r ) || (shoot_control.rc_s_time == RC_S_LONG_TIME))
        {
            shoot_control.shoot_mode = SHOOT_CONTINUE_BULLET;
        }
        else if(shoot_control.shoot_mode == SHOOT_CONTINUE_BULLET)
        {
            shoot_control.shoot_mode =SHOOT_READY_BULLET;
        }
    }

    get_shoot_heat0_limit_and_heat0(&shoot_control.heat_limit, &shoot_control.heat);
    if(!toe_is_error(REFEREE_TOE) && (shoot_control.heat + SHOOT_HEAT_REMAIN_VALUE > shoot_control.heat_limit))
    {
        if(shoot_control.shoot_mode == SHOOT_BULLET || shoot_control.shoot_mode == SHOOT_CONTINUE_BULLET)
        {
            shoot_control.shoot_mode =SHOOT_READY_BULLET;
        }
    }
    //�����̨״̬�� ����״̬���͹ر����
    if (gimbal_cmd_to_shoot_stop())
    {
        shoot_control.shoot_mode = SHOOT_STOP;
    }

    last_s = shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL];
}
/**
  * @brief          ������ݸ���
	shoot motor �ǲ������
  * @param[in]      void
  * @retval         void
  */
static void shoot_feedback_update(void)
{

    static fp32 speed_fliter_1 = 0.0f;
    static fp32 speed_fliter_2 = 0.0f;
    static fp32 speed_fliter_3 = 0.0f;

    //�����ֵ���ٶ��˲�һ��
    static const fp32 fliter_num[3] = {1.725709860247969f, -0.75594777109163436f, 0.030237910843665373f};

    //���׵�ͨ�˲�
    speed_fliter_1 = speed_fliter_2;
    speed_fliter_2 = speed_fliter_3;
    speed_fliter_3 = speed_fliter_2 * fliter_num[0] + speed_fliter_1 * fliter_num[1] + (shoot_control.shoot_motor_measure->speed_rpm * MOTOR_RPM_TO_SPEED) * fliter_num[2];
    shoot_control.speed = speed_fliter_3;

    //���Ȧ�����ã� ��Ϊ�������תһȦ�� �������ת 36Ȧ������������ݴ�������������ݣ����ڿ��������Ƕ�
    if (shoot_control.shoot_motor_measure->ecd - shoot_control.shoot_motor_measure->last_ecd > HALF_ECD_RANGE)
    {
        shoot_control.ecd_count--;
    }
    else if (shoot_control.shoot_motor_measure->ecd - shoot_control.shoot_motor_measure->last_ecd < -HALF_ECD_RANGE)
    {
        shoot_control.ecd_count++;
    }

    if (shoot_control.ecd_count == FULL_COUNT)
    {
        shoot_control.ecd_count = -(FULL_COUNT - 1);
    }
    else if (shoot_control.ecd_count == -FULL_COUNT)
    {
        shoot_control.ecd_count = FULL_COUNT - 1;
    }

    //���������Ƕ�
    shoot_control.angle = (shoot_control.ecd_count * ECD_RANGE + shoot_control.shoot_motor_measure->ecd) * MOTOR_ECD_TO_ANGLE;
    //΢������ 
//    shoot_control.key = BUTTEN_TRIG_PIN;  //PR����ע�͵�΢������
    //��갴��
    shoot_control.last_press_l = shoot_control.press_l;
    shoot_control.last_press_r = shoot_control.press_r;
    shoot_control.press_l = shoot_control.shoot_rc->mouse.press_l;
    shoot_control.press_r = shoot_control.shoot_rc->mouse.press_r;
    //������ʱ
    if (shoot_control.press_l)
    {
        if (shoot_control.press_l_time < PRESS_LONG_TIME)
        {
            shoot_control.press_l_time++;
        }
    }
    else
    {
        shoot_control.press_l_time = 0;
    }

    if (shoot_control.press_r)
    {
        if (shoot_control.press_r_time < PRESS_LONG_TIME)
        {
            shoot_control.press_r_time++;
        }
    }
    else
    {
        shoot_control.press_r_time = 0;
    }

    //��������µ�ʱ���ʱ
    if (shoot_control.shoot_mode != SHOOT_STOP && switch_is_down(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]))
    {

        if (shoot_control.rc_s_time < RC_S_LONG_TIME)
        {
            shoot_control.rc_s_time++;
        }
    }
    else
    {
        shoot_control.rc_s_time = 0;
    }
		//12-30-2021 SZL ���� friction ��� ���� ����
		shoot_control.left_fricMotor.fricW_speed = M3508_FRIC_MOTOR_RPM_TO_LINEAR_VETOR_SEN * shoot_control.left_friction_motor_measure->speed_rpm;
		shoot_control.right_fricMotor.fricW_speed = M3508_FRIC_MOTOR_RPM_TO_LINEAR_VETOR_SEN * shoot_control.right_friction_motor_measure->speed_rpm;
		
		//Added for J-scope debug
		temp_rpm_right = shoot_control.right_friction_motor_measure->speed_rpm;
		temp_rpm_left = shoot_control.left_friction_motor_measure->speed_rpm;
		
}

static void trigger_motor_turn_back(void)
{
    if( shoot_control.block_time < BLOCK_TIME)
    {
        shoot_control.speed_set = shoot_control.trigger_speed_set;
    }
    else
    {
        shoot_control.speed_set = -shoot_control.trigger_speed_set;
    }

    if(fabs(shoot_control.speed) < BLOCK_TRIGGER_SPEED && shoot_control.block_time < BLOCK_TIME)
    {
        shoot_control.block_time++;
        shoot_control.reverse_time = 0;
    }
    else if (shoot_control.block_time == BLOCK_TIME && shoot_control.reverse_time < REVERSE_TIME)
    {
        shoot_control.reverse_time++;
    }
    else
    {
        shoot_control.block_time = 0;
    }
}

/**
  * @brief          ������ƣ����Ʋ�������Ƕȣ����һ�η���
  * @param[in]      void
  * @retval         void
  */
static void shoot_bullet_control(void)
{

    //ÿ�β��� 1/4PI�ĽǶ�
    if (shoot_control.move_flag == 0)
    {
        shoot_control.set_angle = rad_format(shoot_control.angle + PI_TEN);
        shoot_control.move_flag = 1;
    }
    if(shoot_control.key == SWITCH_TRIGGER_OFF)
    {
        shoot_control.shoot_mode = SHOOT_DONE;
    }
    //����Ƕ��ж�
    if (rad_format(shoot_control.set_angle - shoot_control.angle) > 0.15f) //pr�Ķ�ǰΪ0.05f
    {
        //û����һֱ������ת�ٶ�
        shoot_control.trigger_speed_set = TRIGGER_SPEED;
        trigger_motor_turn_back();
    }
    else
    {
        shoot_control.move_flag = 0;
			  shoot_control.shoot_mode = SHOOT_DONE; //pr test
    }
   
}
