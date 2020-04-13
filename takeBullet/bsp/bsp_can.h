#ifndef __BSP_CAN
#define __BSP_CAN

#include "can.h"
#include "mytype.h"
#include "pid.h"

/*CAN���ͻ��ǽ��յ�ID*/
typedef enum
{

	CAN_TxPY12V_ID 	= 0x200,		//��̨12V����ID
	CAN_TxPY24V_ID	= 0x1FF,		//��̨24V����ID
//	CAN_Pitch_ID 	= 0x201,			//��̨Pitch
//	CAN_Yaw_ID   	= 0x203,			//��̨Yaw
	CAN_YAW_FEEDBACK_ID   = 0x205,		//��̨Yaw24v
	CAN_PIT_FEEDBACK_ID  = 0x206,			//��̨Yaw24v
	CAN_POKE_FEEDBACK_ID  = 0x207,
	CAN_ZGYRO_RST_ID 			= 0x404,
	CAN_ZGYRO_FEEDBACK_MSG_ID = 0x401,
	CAN_MotorLF_ID 	= 0x041,    //��ǰ
	CAN_MotorRF_ID 	= 0x042,		//��ǰ
	CAN_MotorLB_ID 	= 0x043,    //���
	CAN_MotorRB_ID 	= 0x044,		//�Һ�

	CAN_EC60_four_ID	= 0x200,	//EC60���ճ���
	CAN_backLeft_EC60_ID = 0x203, //ec60
	CAN_frontLeft_EC60_ID = 0x201, //ec60
	CAN_backRight_EC60_ID = 0x202, //ec60
	CAN_frontRight_EC60_ID = 0x204, //ec60
	
	//add by langgo
	CAN_3510Moto_ALL_ID = 0x200,
	CAN_3510Moto1_ID = 0x201,
	CAN_3510Moto2_ID = 0x202,
	CAN_3510Moto3_ID = 0x203,
	CAN_3510Moto4_ID = 0x204,
	CAN_DriverPower_ID = 0x80,
	
	
	
	CAN_HeartBeat_ID = 0x156,
	
}CAN_Message_ID;

#define FILTER_BUF_LEN		5
/*���յ�����̨����Ĳ����ṹ��*/
typedef struct{
	int16_t	 	speed_rpm;
    int16_t  	real_current;
    int16_t  	given_current;
    uint8_t  	hall;
	uint16_t 	angle;				//abs angle range:[0,8191]
	uint16_t 	last_angle;	//abs angle range:[0,8191]
	uint16_t	offset_angle;
	int32_t		round_cnt;
	int32_t		total_angle;
	u8			buf_idx;
	u16			angle_buf[FILTER_BUF_LEN];
	u16			fited_angle;
	u32			msg_cnt;
}moto_measure_t;

extern moto_measure_t  moto_chassis[4];//�����ĸ��������Ϣ����λ��
extern moto_measure_t  moto_info;//�����ĸ��������Ϣ����λ��
extern CAN_TxHeaderTypeDef hCAN1_TxHeader; //CAN1������Ϣ
extern CAN_RxHeaderTypeDef hCAN1_RxHeader; //CAN1������Ϣ
extern CAN_FilterTypeDef hCAN1_Filter; //CAN1�˲���
extern uint8_t RxData[8];


void userCanInit(CAN_HandleTypeDef *hcan);
void setMotoSpeed(CAN_HandleTypeDef *hcan,uint16_t iq1, uint16_t iq2, uint16_t iq3, uint16_t iq4);
void get_moto_measure(moto_measure_t *moto_chassis, CAN_HandleTypeDef* hcan);
	
#endif

