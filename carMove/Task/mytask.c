#include "mytask.h"

float x;
uint8_t y='0';

void moveTaskFunction(void const * argument)
{	
  
	/* Infinite loop */	
		  for(;;)
		{
	 userCanInit(&hcan1);
   HAL_CAN_RxFifo0MsgPendingCallback(&hcan1);//�˴��д����������������
// ���ĸ����ӵ�PID ���е���
			for(int i=0; i<4; i++)
			{
				pid_calc(&pid_spd[i], moto_chassis[i].speed_rpm, set_spd[i]);
			}
			setMotoSpeed(&hcan1, pid_spd[0].pos_out, 
									pid_spd[1].pos_out,
									pid_spd[2].pos_out,
									pid_spd[3].pos_out,IDMARK_ONE_FOUR);			

			//�������õ���Ļ�������(�ٶ�)����
	switch(key_sta)
	{
		case 0:	//no key
			if( 0 == HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_10) )
			{
				key_sta = 1;
			}
			break;
		case 1: //key down wait release.
			if( 0 == HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_10) )
			{
				key_sta = 2;
				key_cnt++;
			}
			else
			{
				key_sta = 0;
			}
			break;
		case 2: 
			if( 0 != HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_10) )
			{
				key_sta = 0;
			}
			break;
	}
	if(key_cnt>10)
		key_cnt = 0;
  	    	vx = (float)rc.ch2 / 660 * 800;
        vy = -(float)rc.ch1 / 660 *800;
   if(mode=='4'||mode=='5')	
		 osDelay(10);
	 else
	 {
		set_spd[0] = -(-vy + vx+ rc.wheel*5.2)*(key_cnt+1);    
		set_spd[1] = -(-vy - vx+ rc.wheel*5.2)*(key_cnt+1);
	  set_spd[2] = -( vy + vx + rc.wheel*5.2)*(key_cnt+1);
	  set_spd[3] = -( vy- vx+ rc.wheel*5.2)*(key_cnt+1);	
	 }
	//  HAL_UART_Transmit(&huart6,&test,1,50);
//�����ٶ�			
		osDelay(10);
    }	
}

void msgSendTaskFunction(void const * argument) 
{
	for(;;)
	{
		//�жϴ�ʱ��ң������ģʽ
		if(rc_device_get_state(&rc,RC_S2_UP)&&rc_device_get_state(&rc,RC_S1_UP))//��ͨģʽ ��1 ��1
			mode=NORMAL_MODE;
		
		if(rc_device_get_state(&rc,RC_S2_UP)&&rc_device_get_state(&rc,RC_S1_MID))//��ͨģʽ�£����� ��1 ��2
	    mode=GIVEBULLET;
		
		if(rc_device_get_state(&rc,RC_S2_UP)&&rc_device_get_state(&rc,RC_S1_DOWN))//��ͨģʽ�£��ǵ� ��1 ��3
			mode=CLIMB_MODE;
		
		if(rc_device_get_state(&rc,RC_S1_UP)&&rc_device_get_state(&rc,RC_S2_MID))//ȡ��ģʽ�£�һ�� ��2  ��1
			mode=GETBULLET_1_MODE;
		
		if(rc_device_get_state(&rc,RC_S1_MID)&&rc_device_get_state(&rc,RC_S2_MID))//ȡ��ģʽ�£�ʮ�� ��2  ��2
		   mode=GETBULLET_10_MODE;
		
		if(rc_device_get_state(&rc,RC_S1_DOWN)&&rc_device_get_state(&rc,RC_S2_MID))//ȡ��ģʽ��,�Ľ� ��2  ��3
		   mode=GETBULLET_4_MODE;
		
		if(rc_device_get_state(&rc,RC_S1_UP)&&rc_device_get_state(&rc,RC_S2_DOWN))//�����Ԯģʽ ��3 ��1
			mode=RESCUE_MODE;
		
		if(rc_device_get_state(&rc,RC_S2_DOWN)&&rc_device_get_state(&rc,RC_S1_MID))//�˳���Ԯģʽ������ ��3 ��2
		   mode=OPEN_MODE;
    
		if(rc_device_get_state(&rc,RC_S2_DOWN)&&rc_device_get_state(&rc,RC_S1_DOWN))//��Ԯ��ģʽ������ ��3 ��3
		   mode=RESCUE_CAR_MODE;		
		
		//���ʹ�ʱ��ģʽ����̨����
		masge[0]='a';
		masge[6]='b';
		masge[5]=mode;
		HAL_UART_Transmit(&huart6,masge,sizeof(masge),50);
		osDelay(10);
	}
	
}

void getBulletTaskFunction(void const * argument)
{
	for(;;)
	{
		if(mode=='5'||mode=='6'||mode=='4')
		{
				set_spd[0]=set_spd[1]=set_spd[2]=set_spd[3]=0;
			//�źŵ������Ϊ̧��ȡ���Ļ������˴����ýǶ��ȫ
			 pid_calc(&pid_spd[8],moto_chassis[8].speed_rpm,1000);
			
			HAL_UART_Receive(&huart6,&actionSignal,1,50);
			while(actionSignal!='o'){//�յ�ȡ����ɱ�ʶ�����ȡ������
				HAL_UART_Receive(&huart6,&actionSignal,1,50);
				osDelay(500);
	    }
			pid_calc(&pid_spd[8],moto_chassis[8].speed_rpm,-1000);
			
			mode=NORMAL_MODE;
			actionSignal='f';//����������ź��ü�
	  }
}
}

/*
��Ԯ������Ҫ�Ķ�
*/
void rescueTaskeFunction(void const * argument)
{
		for(;;)
	{
		 if(mode=='7'||mode=='8'||mode=='9')
		  {
			   if(HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_1)==GPIO_PIN_RESET||HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_0)==GPIO_PIN_RESET)//��������
				 { 
			 HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2,800);//�Զ�����ֵΪ1000
				
		     if(mode=='9')
				 {
			      osDelay(1000);//�ȴ�1�룬ȷ��ץס����
						 HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);
			      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3,800);
					}
	    }		
			}
    else
		{
			 HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
			HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2,200);
			 __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3,200);	//�ջ�צ�Ӻ;�Ԯ��
		}
	}
	
}

//void climbUPIslandFunction(void const * argument){
//  for(;;){  
//	if(mode==CLIMB_MODE){
//    //�˴��Ѹ�Ϊ�Ƕ�Ϊ��		
//    pid_calc(&pid_spd[4],moto_chassis[4].angle,180*3591/187);
//		pid_calc(&pid_spd[5],moto_chassis[5].angle,180*3591/187);
//		setMotoSpeed(&hcan1,pid_spd[4].pos_out,
//		                    pid_spd[5].pos_out,
//		                    0,
//		                    0,IDMARK_FIVE_EIGHT);//̧����
//		//ǰ�ýż�⵽̨��
//		while(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_1)!=GPIO_PIN_SET|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_2)!=GPIO_PIN_SET){
//		pid_calc(&pid_spd[5],moto_chassis[5].speed_rpm,500);
//		pid_calc(&pid_spd[6],moto_chassis[6].speed_rpm,500);
//		setMotoSpeed(&hcan1,pid_spd[5].pos_out,
//		                    pid_spd[6].pos_out,
//		                    0,
//		                    0,IDMARK_FIVE_EIGHT);//ǰ�Ƴ���
//		}
//		
//	  setMotoSpeed(&hcan1,0,0,0,0,IDMARK_FIVE_EIGHT);//ǰ��ֹͣ��ʹ��ǰ�ִ�����
//		pid_calc(&pid_spd[4],moto_chassis[4].speed_rpm,-80*3591/187);
//		pid_calc(&pid_spd[4],moto_chassis[5].speed_rpm,80*3591/187);
//		setMotoSpeed(&hcan1,pid_spd[4].pos_out,
//		                    pid_spd[5].pos_out,
//		                    0,
//		                    0,IDMARK_FIVE_EIGHT);//��ǰ�ţ����ֺ��
//		
//		while(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_3)!=GPIO_PIN_SET|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_4)!=GPIO_PIN_SET){
//				for(int i=0; i<2; i++)
//			{
//				pid_calc(&pid_spd[i], moto_chassis[i].speed_rpm, 500);
//			}
//			setMotoSpeed(&hcan1, pid_spd[0].pos_out, 
//									pid_spd[1].pos_out,
//									0,
//									0,IDMARK_ONE_FOUR);//ǰ����ǰ��
//			osDelay(100);
//		}
//			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET);//���¸�����
//		
//		pid_calc(&pid_spd[4],moto_chassis[4].speed_rpm,-80*3591/187);
//		pid_calc(&pid_spd[4],moto_chassis[5].speed_rpm,-80*3591/187);
//		setMotoSpeed(&hcan1,pid_spd[4].pos_out,
//		                    pid_spd[5].pos_out,
//		                    0,
//		                    0,IDMARK_FIVE_EIGHT);//������
//      while(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0)!=GPIO_PIN_SET|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_5)!=GPIO_PIN_SET){
//			   osDelay(100);
//			}
//	 }
//	  HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET);//�ǵ���������������
//  	mode=NORMAL_MODE;//�˳��ǵ�ģʽ������һ��ģʽ
// }
//}

//void climbDownIslandFunction(void const * argument){
// for(;;){
//	 HAL_GPIO_WritePin(GPIOE,GPIO_PIN_12,GPIO_PIN_SET);//���¸�����
//	 while(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_0)!=GPIO_PIN_SET|HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_5)!=GPIO_PIN_SET){
//	  pid_calc(&pid_spd[5],moto_chassis[5].speed_rpm,-500);
//		pid_calc(&pid_spd[6],moto_chassis[6].speed_rpm,-500);
//		setMotoSpeed(&hcan1,pid_spd[5].pos_out,
//		                    pid_spd[6].pos_out,
//		                    0,
//		                    0,IDMARK_FIVE_EIGHT);//���Ƴ���
//	 }

//	 	pid_calc(&pid_spd[4],moto_chassis[4].speed_rpm,-80*3591/187);
//		pid_calc(&pid_spd[4],moto_chassis[5].speed_rpm,80*3591/187);
//		setMotoSpeed(&hcan1,pid_spd[4].pos_out,
//		                    pid_spd[5].pos_out,
//		                    0,
//		                    0,IDMARK_FIVE_EIGHT);//���º��
//	 
//	 
//	 mode=NORMAL_MODE;//�˳��µ�ģʽ������һ��ģʽ
// }
//}

void giveBulletTaskFunction(void const * argument){
  for(;;){
	if(mode==GIVEBULLET){
		while(actionSignal!='o'){
			osDelay(500);
			HAL_UART_Transmit(&huart6,&mode,1,50);//���������ź�
			HAL_UART_Receive(&huart6,&actionSignal,1,50);
     }
		actionSignal='f';//����������ź��ü�
	 }
  }
}



