#include "mytask.h"

/*
������ʵ��
*/

void takeBullteTaskFunction(void const * argument)
{
	for(;;){
	  userCanInit(&hcan1);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan1);//�˴��д����������������
		//��ɵ����ת������
					pid_calc(&pid_spd[0], moto_chassis[0].speed_rpm, set_speed);//һ��Ϊ��λ�������Ҫ����ת��
		      pid_calc(&pid_spd[1], moto_chassis[1].angle, set_angel);//����Ϊȡ�����,��Ҫ���ýǶ�
			setMotoSpeed(&hcan1, pid_spd[0].pos_out, 
						               pid_spd[1].pos_out,
									         0,
									         0);	
		
		HAL_UART_Receive(&huart6,masge,7,50);
		
		if(masge[5]==GETBULLET_1_MODE|masge[5]==GETBULLET_10_MODE|masge[5]==GETBULLET_4_MODE){
    direction=jugement();
     if(direction==2|direction==7){
			 	//�м��⵽������û�У�Ϊ��λ׼ȷ��ȡ��
		   set_speed=0;
/*ȡ���м䵯ҩ*/		 
       take();//צ��ȡ��
			 HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);//����ҩ���빩����
			 
	     //�ߵ�ƽ�����צ��
		   HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
			 
       take();
			 
/*ȡ�����൯ҩ*/				
       HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_SET);//�Ƶ����
			  take();
			 HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,GPIO_PIN_RESET);//�ص�ԭλ
			 HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);//����ҩ���빩����
			 
			 HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);//�Ƶ��ұ�
			  take();
			 HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET);//�ص�ԭλ
			 HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);//����ҩ���빩����
			 osDelay(2000);
/*��ȡ������������*/			 
      actionSignal='o';  //�������ȡ�����ź�
			HAL_UART_Transmit(&huart6,&actionSignal,1,50);
		 }

		 if(direction==0|direction==1|direction==3)	{
			  //���������Ϊ�����ƶ�
		   set_speed=2000;
			 set_angel=0;
		 }			 

		 if(direction==4|direction==5|direction==6){
			   //���������Ϊ�����ƶ�
		   set_speed=2000;
			 set_angel=0;
		 }
	  	direction=0;//��һ���жϽ����󣬸�λ���¿�ʼ
	}
	
}	
}


void giveBullteBullteTaskFunction(void const * argument){
  
	uint8_t signal;
	
	HAL_UART_Receive(&huart6,masge,7,50);//���������ź�
	if(masge[5]==GIVEBULLTE){
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_SET);
		osDelay(3000);
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_RESET);
		signal='o';
		HAL_UART_Transmit(&huart6,&signal,1,50);//������������ź�
	}
}

void holderTaskFunction(void const * argument){
	for(;;){
	//����PWMͨ��
	int x,y;//
		HAL_UART_Receive(&huart6,masge,7,50);
		   x=(masge[1]>> 1 | masge[2] << 7) & 0x07FF;
		   y=(masge[3]>> 1 | masge[4] << 7) & 0x07FF;
	    	x-=1024;//�о�һ�£�ò����ԭ������RC��ʱ�������
		    y-=1024;
		
    	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
	    HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2,x);//�Զ�����ֵΪ1000
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3,y);
	}
}

/*
ģ�鹦�ܺ���ʵ��
*/

int jugement(void){
	  int direction=0;
	  if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)==SET)
			direction=direction|4;  //4 = 100    ��λ�����Ƶĸ���λ��0��ʾδ��⵽  1��ʾ��⵽
		if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_5)==SET)		
			direction=direction|2;  //2 = 010
		if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_6)==SET)	
			direction=direction|1;  //1 = 001
		
		return direction;
}

void take(void ){
				set_angel=80*3591/187;//���������˼��ٱȣ����ԽǶ�����Ҫת�� ת80��
			 osDelay(500);
			 HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_SET); //�ߵ�ƽ��צ�ӹر�
			 set_angel=-80*3591/187; //������������
			 osDelay(500);
			 HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_RESET);//�͵�ƽ��צ�Ӵ򿪣�������ҩ��
}


