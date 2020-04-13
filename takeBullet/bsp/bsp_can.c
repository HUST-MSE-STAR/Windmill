#include "bsp_can.h"


CAN_RxHeaderTypeDef hCAN1_RxHeader; //CAN1������Ϣ
CAN_FilterTypeDef hCAN1_Filter; //CAN1�˲���
moto_measure_t  moto_chassis[4];//�����ĸ��������Ϣ����λ��
moto_measure_t  moto_info;//�����ĸ��������Ϣ����λ��
uint8_t RxData[8];

void userCanInit(CAN_HandleTypeDef *hcan)
{ 

	
	//��ʼ���˲���
	hCAN1_Filter.FilterBank = 0;  //��ͬ�ڹٷ���FilterNumber���������ù��������
	hCAN1_Filter.FilterMode = CAN_FILTERMODE_IDMASK; //���� ģʽ
	hCAN1_Filter.FilterScale = CAN_FILTERSCALE_32BIT;//ID 32λ��
	hCAN1_Filter.FilterIdHigh = 0x0000;
	hCAN1_Filter.FilterIdLow = 0x0000;
	hCAN1_Filter.FilterMaskIdHigh = 0x0000;
	hCAN1_Filter.FilterMaskIdLow = 0x0000;
	hCAN1_Filter .FilterFIFOAssignment= CAN_FilterFIFO0;
//	hCAN1_Filter.SlaveStartFilterBank = 14;//��ͬ�ڹٷ�BankNumber,���ڷֹ�����������CAN��can1(0-13)��can2(14-27)�ֱ�õ�һ���filter
	hCAN1_Filter.FilterActivation = ENABLE;
	
	//����hcan���˲���
	HAL_CAN_ConfigFilter(hcan, &hCAN1_Filter);
	
	//����hcan
	HAL_CAN_Start(hcan);
			
	//ʹ���ж�֪ͨ
	HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}


void setMotoSpeed(CAN_HandleTypeDef *hcan,uint16_t iq1, uint16_t iq2, uint16_t iq3, uint16_t iq4)
{
	uint8_t Data[8]={0};
	CAN_TxHeaderTypeDef hCAN1_TxHeader; //CAN1������Ϣ
		//��ʼ�����͵ľ��
	hCAN1_TxHeader.StdId = 0x200;
	hCAN1_TxHeader.IDE = CAN_ID_STD;
	hCAN1_TxHeader.RTR = CAN_RTR_DATA;
	hCAN1_TxHeader.DLC = 0x08;
	//hCAN1_TxHeader.TransmitGlobalTime    =    ENABLE;
	
	Data[0] = (iq1 >> 8)&0xff;
	Data[1] = iq1&0xff;
	Data[2] = (iq2 >> 8)&0xff;
	Data[3] = iq2&0xff;
	Data[4] = (iq3 >> 8)&0xff;
	Data[5] = iq3&0xff;
  Data[6] = (iq4 >> 8)&0xff;
	Data[7] = iq4&0xff;
	//HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t aData[], uint32_t *pTxMailbox)
	//can�������ݵĺ���

	HAL_CAN_AddTxMessage(hcan,&hCAN1_TxHeader,Data,(uint32_t*)CAN_TX_MAILBOX0);

}


/*******************************************************************************************
  * @Func			void get_moto_measure(moto_measure_t *moto_chassis, CAN_HandleTypeDef* hcan)
  * @Brief    ������̨���,3510���ͨ��CAN����������Ϣ
  * @Param		
  * @Retval		None
  * @Date     2015/11/24
 *******************************************************************************************/
void get_moto_measure(moto_measure_t *moto_chassis, CAN_HandleTypeDef* hcan)
{

	if(HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &hCAN1_RxHeader, RxData)==HAL_ERROR)// CAN_RX_FIFO0 �ڶ�������ò���ǽ��жϵ�һ���ж�
	{					
	moto_chassis->last_angle = moto_chassis->angle;

	// hcan canͨ�Ź�������Ϣ������Э��ȡ����Ӧ����
	
	moto_chassis->angle = (uint16_t)(RxData[0]<<8 | RxData[1]) ;
	moto_chassis->real_current  = (int16_t)(RxData[2]<<8 | RxData[3]);
	moto_chassis->speed_rpm = moto_chassis->real_current;	
	moto_chassis->given_current = (int16_t)(RxData[4]<<8 | RxData[5])/-5;
	moto_chassis->hall = RxData[6];
		
	if(moto_chassis->angle - moto_chassis->last_angle > 4096)
		moto_chassis->round_cnt --;
	else if (moto_chassis->angle - moto_chassis->last_angle < -4096)
		moto_chassis->round_cnt ++;
	moto_chassis->total_angle = moto_chassis->round_cnt * 8192 + moto_chassis->angle - moto_chassis->offset_angle;
}
	
}
/*this function should be called after system+can init */
void get_moto_offset(moto_measure_t *ptr, CAN_HandleTypeDef* hcan)
{
	ptr->angle = (uint16_t)(RxData[0]<<8 | RxData[1]) ;
	ptr->offset_angle = ptr->angle;
}



#define ABS(x)	( (x>0) ? (x) : (-x) )
/**
*@bref ����ϵ�Ƕ�=0�� ֮���������������3510�������Կ�����Ϊ0������ԽǶȡ�
	*/
void get_total_angle(moto_measure_t *p){
	
	int res1, res2, delta;
	if(p->angle < p->last_angle){			//���ܵ����
		res1 = p->angle + 8192 - p->last_angle;	//��ת��delta=+
		res2 = p->angle - p->last_angle;				//��ת	delta=-
	}else{	//angle > last
		res1 = p->angle - 8192 - p->last_angle ;//��ת	delta -
		res2 = p->angle - p->last_angle;				//��ת	delta +
	}
	//��������ת���϶���ת�ĽǶ�С���Ǹ������``
	if(ABS(res1)<ABS(res2))
		delta = res1;//���������ö������仯��
	else
		delta = res2;

	p->total_angle += delta;
	p->last_angle = p->angle;
}

/*******************************************************************************************
  * @Func			void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* _hcan)
  * @Brief    ����һ���ص�����,����������
  * @Param		
  * @Retval		None 
  * @Date     2015/11/24
 *******************************************************************************************/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* _hcan)
{   
	int i=0;
	//ignore can1 or can2.����Э�齫can���ص����ݽ�������
//	switch(hCAN1_RxHeader.StdId){//
//		case CAN_3510Moto1_ID:
//		case CAN_3510Moto2_ID:
//		case CAN_3510Moto3_ID:
//		case CAN_3510Moto4_ID:
//			{
//				static uint8_t i;
//				i = hCAN1_RxHeader.StdId - CAN_3510Moto1_ID;//����õ������1/2/3/4
//				//����msg_cnt�������ж�ִ��
//				moto_chassis[i].msg_cnt++ <= 50	?	get_moto_offset(&moto_chassis[i], _hcan) : get_moto_measure(&moto_chassis[i], _hcan);
//				get_moto_measure(&moto_info, _hcan);

//				//get_moto_measure(&moto_chassis[i], _hcan);
//			}
//			break;		
//	}			 
	  for(i=0;i<4;i++)//�õ����
    get_moto_measure(&moto_chassis[i], _hcan);
    __HAL_CAN_ENABLE_IT(&hcan1, CAN_IER_FMPIE0);//����ж�ֻ�ܽ�һ�ε�ë��
    
}

