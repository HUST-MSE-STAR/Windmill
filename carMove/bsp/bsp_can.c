#include "bsp_can.h"


CAN_RxHeaderTypeDef hCAN_RxHeader; //CAN������Ϣ�Ľṹ��
CAN_FilterTypeDef hCAN1_Filter; //CAN1�˲���
CAN_FilterTypeDef hCAN2_Filter; //CAN2�˲���

moto_measure_t  moto_chassis[9];//���̰���ƵľŸ��������Ϣ����λ��

uint8_t RxData[8];

void userCanInit(CAN_HandleTypeDef *hcan)
{ 
   can1Init(&hcan1);	
	 can2Init(&hcan2);
}
/*
 CANl������
*/
void can1Init(CAN_HandleTypeDef *hcan){
		//��ʼ���˲���CAN1
	hCAN1_Filter.FilterBank = 0;  //��ͬ�ڹٷ���FilterNumber���������ù��������
	hCAN1_Filter.FilterMode = CAN_FILTERMODE_IDMASK; //���� ģʽ
	hCAN1_Filter.FilterScale = CAN_FILTERSCALE_32BIT;//ID 32λ��
	hCAN1_Filter.FilterIdHigh = 0x0000;
	hCAN1_Filter.FilterIdLow = 0x0000;
	hCAN1_Filter.FilterMaskIdHigh = 0x0000;
	hCAN1_Filter.FilterMaskIdLow = 0x0000;
	hCAN1_Filter .FilterFIFOAssignment= CAN_FilterFIFO0;
	hCAN1_Filter.SlaveStartFilterBank = 14;//���ڷֹ�����������CAN��can1(0-13)��can2(14-27)�ֱ�õ�һ���filter
	hCAN1_Filter.FilterActivation = ENABLE;
	
	//����hcan���˲���
	HAL_CAN_ConfigFilter(hcan, &hCAN1_Filter);
	//����hcan
	HAL_CAN_Start(hcan);
	//ʹ���ж�֪ͨ
	HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}

/*
 CAN2������
*/
void can2Init(CAN_HandleTypeDef *hcan){
		//��ʼ���˲���CAN1
	hCAN2_Filter.FilterBank = 14;  //��ͬ�ڹٷ���FilterNumber���������ù��������
	hCAN2_Filter.FilterMode = CAN_FILTERMODE_IDMASK; //���� ģʽ
	hCAN2_Filter.FilterScale = CAN_FILTERSCALE_32BIT;//ID 32λ��
	hCAN2_Filter.FilterIdHigh = 0x0000;
	hCAN2_Filter.FilterIdLow = 0x0000;
	hCAN2_Filter.FilterMaskIdHigh = 0x0000;
	hCAN2_Filter.FilterMaskIdLow = 0x0000;
	hCAN2_Filter .FilterFIFOAssignment= CAN_FilterFIFO0;
	hCAN2_Filter.SlaveStartFilterBank = 14;//���ڷֹ�����������CAN��can1(0-13)��can2(14-27)�ֱ�õ�һ���filter
	hCAN2_Filter.FilterActivation = ENABLE;
	
	//����hcan���˲���
	HAL_CAN_ConfigFilter(hcan, &hCAN2_Filter);
	//����hcan
	HAL_CAN_Start(hcan);
	//ʹ���ж�֪ͨ
	HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}

/*
������Ϣ�����õ���
*/
void setMotoSpeed(CAN_HandleTypeDef *hcan,uint16_t iq1, uint16_t iq2, uint16_t iq3, uint16_t iq4,uint16_t IDRange)
{
	uint8_t Data[8]={0};
	CAN_TxHeaderTypeDef hCAN_TxHeader; //������Ϣ�ľ��
		//��ʼ�����͵ľ��
	hCAN_TxHeader.StdId = IDRange;//���ݲ�ͬ�ı�ʶ�������������ID������1-4����5-8��
	hCAN_TxHeader.IDE = CAN_ID_STD;
	hCAN_TxHeader.RTR = CAN_RTR_DATA;
	hCAN_TxHeader.DLC = 0x08;
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

	HAL_CAN_AddTxMessage(hcan,&hCAN_TxHeader,Data,(uint32_t*)CAN_TX_MAILBOX0);

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

	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &hCAN_RxHeader, RxData)==HAL_ERROR)// CAN_RX_FIFO0 �ڶ��������ǽ��жϵ�һ���ж�
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
  * @Func			void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* _hcan)
  * @Brief    ����һ���ص�����,������������������ѡ�����ʱ��FIFO/FIFIO1��Ӧ����������ͬ�Ļص�����
  * @Param		
  * @Retval		None 
  * @Date     2015/11/24
 *******************************************************************************************/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* _hcan)
{   
	//�Բ�ͬ��CAN���߸���Э�齫can���ص����ݽ�������
	if(_hcan==&hcan1){
	switch(hCAN_RxHeader.StdId){
		case IDMARK_ONE_FOUR:
			get_moto_measure(&moto_chassis[hCAN_RxHeader.StdId-IDMARK_ONE_FOUR-1],_hcan);//����ID��ȡ����±겢������Ϣ
			break;
		
		case IDMARK_FIVE_EIGHT:
			get_moto_measure(&moto_chassis[hCAN_RxHeader.StdId-IDMARK_ONE_FOUR+4-1],_hcan);
			break;
	}			
	}
	
	if(_hcan==&hcan2){
	 get_moto_measure(&moto_chassis[8],_hcan);//��Ϊ9�ŵ������can2���ϵ�һ�ŵ������ֱ�Ӵ���Ϣ
	}
 
    __HAL_CAN_ENABLE_IT(&hcan1, CAN_IER_FMPIE0);//����ж�ֻ�ܽ�һ�ε�ë��
    
}

