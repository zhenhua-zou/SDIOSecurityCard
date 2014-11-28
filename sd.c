/*********************************************
This program was produced by the
CodeWizardAVR V1.23.8d Standard
Automatic Program Generator
� Copyright 1998-2003 HP InfoTech s.r.l.
http://www.hpinfotech.ro
e-mail:office@hpinfotech.ro

Project : SDIO KEY
Version : V2.0
Date    : 2007-7-9
Author  : Zhenhua Zou
Company : seu
Comments: 


Chip type           : ATmega8L
Program type        : Application
Clock frequency     : 8.000000 MHz
Memory model        : Small
External SRAM size  : 0
Data Stack size     : 256
*********************************************/

#include <mega8.h>
#include <delay.h>

#define	RX_BUFFER_SIZE 	255   	// USART Receiver buffer,����Զ����ɵĴ��ڴ���
#define	RESPONSE_SIZE	255     //���ܿ���Ӧ���ֽڳ���
#define rst		PORTB.0
#define io		PORTB.2
#define iomode		DDRB.2      

/*******************for USART****************************/
#define RXB8	1
#define TXB8 	0
#define UPE 	2
#define OVR 	3
#define FE 	4
#define UDRE 	5
#define RXC 	7

#define FRAMING_ERROR 		(1<<FE)
#define PARITY_ERROR 		(1<<UPE)
#define DATA_OVERRUN 		(1<<OVR)
#define DATA_REGISTER_EMPTY 	(1<<UDRE)
#define RX_COMPLETE 		(1<<RXC)

unsigned char c = 0;
unsigned char 	rx_wr_index;
unsigned char	rx_counter;                 	
bit 	rx_buffer_overflow;		// This flag is set on USART Receiver buffer overflow 
/**********************************************************/


/*********************���ܿ����������**********************/
unsigned char	commanddata[RX_BUFFER_SIZE];  //commanddata[] �������ڴ�PDA���յ����ܿ�����*****
unsigned char	responsedata[RESPONSE_SIZE]; 	//responsedata[]������Ӧ����
unsigned char	commandlength=5;              //�Ӵ��ڽ��յ������ 
unsigned char	commandtype=0xFD;            	//��������
unsigned char  	responselength=22;        	//���ڸ�λ��Ϣ�ĳ���
unsigned char	i=14;                         //���������λָ��
unsigned char  	j=0;                          //����������ֽ�ָ��   
/*********************λ����������*************************/
bit checkbit=0;                   
bit resetbit=0;
bit transimitbit=0;  
bit recievebit=0;        
unsigned char temp=0;
 
void test(unsigned char i)
{
        if(UCSRA.6) // TXC==1 ������һ���ֽڷ��ͽ���
 	{
	     UCSRA.6=1;//������ͽ�����־λ    
	}  
 	if(UCSRA.5) //UDRE==1 �������ͼĴ�����,���Է������ֽ�                           
 	{
 	     if(UCSRA.7==0)//RXC==0, ������������ǰû�н��յ����ݣ����Է���		
 	     UDR=i;     
 	} 
 	while(UCSRA.5==0)//�ȴ���һ�ֽڷ��꣡
	{     
	     #asm("wdr");
	} 
}

/***********************�Զ��庯����************************/
void delay100clk(void)//������ʱ100clock=100x0.2793us=27.93us�����ú���ʵ����ʱ��27.63uS
{
	unsigned char i=0;
	for(i=0;i<35;i++)
	{
		#asm("nop"); 
	}
}

void delay40000clk(void)
{
	unsigned int i=0;
	for(i=0;i<7447;i++)
	{
		#asm("nop"); 
		#asm("nop");
		#asm("nop");
		#asm("nop");
	}
}

void delayhalfetu (void)   //ʵ��delay 0.5 etu 52.38uS ,�����ܿ�if baud=9600,0.5etu=0.5x(1/9600)=52.083uS
{                    
	unsigned char i=0;
	
// 	for(i=0;i<67;i++)				//ԭ����67��  
 	for(i=0;i<73;i++)                               //���ڲ������£���ʱ��У���£������Ƚ��ȶ�
            #asm("nop"); 		
} 

void delayoneetu (void)   //ʵ��delay 1 etu 104.13uS ,�����ܿ�if baud=9600,1etu=1/9600=104.167uS
{                    
	unsigned char i=0;
	
//	for(i=0;i<135;i++)                
        for(i=0;i<135;i++)                              ////���ڲ������£���ʱ��У���£������Ƚ��ȶ�
            #asm("nop"); 
}

/*#define delay100clk() delay_us(30)
#define delay40000clk() delay_us(11175) 
#define delayhalfetu() delay_us(52)
#define delayoneetu() delay_us(104)*/

void Reset(void)
{
  /*****************��λʱ��Ҫ��*****************************   
  Vcc_|------------------------------------------------|__ 
       |                                               |
  Vpp_|------------------------------------------------|__    
       |                                               |
  clk__|-|_|-|_...................................|-|_|-|_   
       |                                 
  rst__________________________|----------------------|__
       |											 |
  i/o__xxx|------------------------------|___answer____xxx
       |  | 									 |				 |						 |
     ->|--|<-t2-               |<--t1--->|             |
       |<------- T1(t3)------->|
       T0 t2                      				             T2
       
  400   clock =< t1 <= 40000 clock
                 t2 <= 200   clock
  40000 clock =< t3        
  ���ܿ�ʱ�Ӳ��ö���mega16L����Դ���壬 Ƶ��Ϊ3.579545MHz���ϵ缴����.     
  1��clock����Ϊ 1/3.579545MHz=0.2794uS
  1��etu=372/3.579545MHz=103.923uS   
  BUAD=3.579545MHz/372=9622.4      
  
  ��ʵ���ϣ����I���ܿ��涨�BBUAD=9600,��fi=9600 x 327 = 3.5712MHz
  1��clock����Ϊ 1/3.5712MHz=0.280uS
  1��etu=372/3.579545MHz=104.167uS   
  
  �����и�ʱ��ƫ�
  /*****************��λʱ��Ҫ��*****************************/     
  
  unsigned char i=0;  
  unsigned char j=0;
  responselength = 0;  
  checkbit=0;   //��λ��ϢУ��  
  resetbit=0;	//��λ����ָʾλ   
  
  /*****************��λ���ܿ�******************************/
  #asm("nop");
  rst=0;   // rst����T0Ϊʱ����㣬��ʼ�õͣ�����t3ʱ�䡣
           //����ʱ�� Ƶ��Ϊ3.579545MHz.                 
  delay100clk(); //io����T0Ϊʱ����㣬t2ʱ�����ó�Z״̬����ѡ����ʱ100clock=100x0.2793us=27.93us
  iomode=0;      //io���� ����Ϊ����
  #asm("nop");
  delay40000clk();//t3ʱ�����rst�øߣ���ѡ��t3 40000clk=40000 x 0.2793us = 11172us
  rst=1;
  delay100clk();     	//������rst=1��t1ʱ�䣬���ܿ����ܷ���λ��Ϣ
  delay100clk();
  delay100clk();
  delay100clk();     	//���ٵĵȴ����� 
   
  for(i=0;i<22;i++)  	//��Ϊ��λ��Ϣ��22���ֽڣ���176λ
  {
   	while(PINB.2)	//�ȴ���λ��Ϣ�����ֽڵ���ʼλ

  	delayhalfetu();
  	for(j=0;j<8;j++)  //��8������λ
  	{ 
  		delayoneetu();
  		responsedata[i]=responsedata[i]>>1; //�ճ���λ�Ĵ洢λ��  	
  		if(PINB.2)    	//��ȡPINA.2��״̬����io����
  		{
  			responsedata[i]=responsedata[i]|0x80;	//���PINB.2==1����Ѹ�λ��Ϣ����ĵ�ǰ���λ��1������������  
  			checkbit++;//������ֽ���1����ż��
  		}
  	}
    //��ǰʱ��Ϊ8.5etu	
    delayoneetu();//����ʱ����Զ���żλ��9.5etu 
     
    /**������żλ**/
    if(PINB.2)	//��ǰ�ֽڶ�����,���ڶ���żλ  
    	checkbit++;            
        
    if(checkbit) {
    	resetbit=1; 
        responsedata[i] = 0xEE; //added by zzh
//        return 0; //added by zzh 
        responselength = i + 1;
    }
    checkbit = 0; //added by zzh              
        
    responselength++;
        
    /**������żλ**/
    
    delayoneetu();		//�ӿ���ӿ��豸�����ַ����Ϊ12��etu,�������ı���ʱ�䡣���ӿ��豸��������Ҫ12+n��etu
    delayoneetu();		//11.5etu
  }  
  delayoneetu();			//12.5etu    
}

void Close(void)
{
  /*****************�ر����ܿ�******************************/
	rst=0; 
        #asm("nop");  
	iomode=1;						//io���� ����Ϊ���     
	io=0;                
	#asm("nop");                 
}

void recieveresponse (void)
{
	/****************�������ܿ����������Ӧ*********************/   	
 	unsigned char i=0;    
 	unsigned char j=0;   
 	unsigned int k=0;	//�����ж���Ӧ�Ƿ����   
 	
	#asm("nop"); //1 us
	iomode=0;  //io���� ����Ϊ����    0.25us    
	io=1;	   //ʹ����������  
	responselength=0;//��Ӧ���ȸ�λ
	for(i=0;i<RESPONSE_SIZE;i++) //0.5us
  	{
   	
   	/*************�ò��ִ������������Զ���¼��Ӧ�ĳ���****************/
   	
   	if(responselength)
   	{
   		while(PINB.2)	  	//�ȴ���Ӧ��Ϣ��2���ֽڵ���ʼλ------���һ���ֽڵ���ʼλ
			{
				#asm("nop");
				#asm("nop");
				#asm("nop");
				#asm("nop");
				#asm("nop");
				#asm("nop");
				#asm("nop");
				#asm("nop");
				#asm("nop");
				#asm("nop");
				#asm("nop");
				#asm("nop");
				#asm("nop");
				#asm("nop");
				#asm("nop");
				#asm("nop");  //16��nop��ʱ2us
				if(++k>500)   //����2.1�汾COS�Ĺ����ֽں���Ӧ֮����������3.2ms,k>200Ϊ�о��������������ˡ�
			                      //������2.0�汾COS�Ĺ����ֽں���Ӧ֮����������4ms,k>250Ϊ�о������������С�
				   return;    //������400us����PINB.2��ȻΪ�ߣ�˵����Ӧȫ��������
			} 
	} 
	else
	{
		while(PINB.2)	  	//�ȴ���Ӧ��Ϣ��һ���ֽڵ���ʼλ  
		{       
			#asm("wdr");
		}	
	}    
	k=0;	 
			 
		/*************�ò��ִ������������Զ���¼��Ӧ�ĳ���****************/ 
	
  	delayhalfetu(); 	//44.88us
  	for(j=0;j<8;j++)  	//��8������λ                          0.38us
  	{ 
  		delayoneetu();	                                    //104.13us          
  		responsedata[i]=responsedata[i]>>1; //�ճ���λ�Ĵ洢λ�� 2.63us
  		if(PINB.2)    		//��ȡPINB.2��״̬����io����
  		{
  			responsedata[i]=responsedata[i]|0x80;	//���PINB.2==1����Ѹ�λ��Ϣ����ĵ�ǰ���λ��1��2.63us
  								//����������0.38us
  			checkbit++;			//ͳ�Ƹ��ֽ���1����ż��	
  		}    
  	}                 	//1.13us
  	//��ǰʱ��Ϊ8.5etu	
    delayoneetu();			//����ʱ����Զ���żλ��9.5etu     
    
    /**������żλ**/
    if(PINB.2)				//��ǰ�ֽڶ�����,���ڶ���żλ  
    	checkbit++;
    if(checkbit)
    {
    	recievebit=1;   	
    	responsedata[i] = 0xee; // added by zouzhenhua
    }
    checkbit = 0;
    /**������żλ**/	          
  
    delayoneetu();			//�ӿ���ӿ��豸�����ַ����Ϊ12��etu,�������ı���ʱ�䡣���ӿ��豸��������Ҫ12+n��etu
//    delayoneetu();			//11.5etu
    responselength++;   //�յ�һ���ֽڵ���Ӧ����Ӧ���ȼ� 1    
  }                      
}   
 
/************�򴮿ڷ������ܿ�����Ӧ***************/
void transmitresponse(void)
{               
	unsigned char i=0;        
	for(i=0;i<responselength;i++)   
	{                               
		if(UCSRA.6) // TXC==1 ������һ���ֽڷ��ͽ���
 		{
		     UCSRA.6=1;//������ͽ�����־λ    
		}  
 		if(UCSRA.5) //UDRE==1 �������ͼĴ�����,���Է������ֽ�                           
 		{
 		     if(UCSRA.7==0)//RXC==0, ������������ǰû�н��յ����ݣ����Է���		
 		     UDR=responsedata[i];     
 		} 
 		while(UCSRA.5==0)//�ȴ���һ�ֽڷ��꣡
		{     
		     #asm("wdr");
		} 
 	} 	
} 

//Timer 1 overflow interrupt service routine
interrupt [TIM1_OVF] void timer1_ovf_isr(void)
{
//	Place your code here
 
// 	TCNT2=0x97;     //��ʱ1/9600=104.17uS                       

        TCNT1L=0xBE;                                                     
//	TCNT1L=0xEA;  // ?????�����ʱ����μ��������??? zouzhenhua ������������ܿ��Ķ�ʱ����ʱ������

	TCNT1H=0xFC;                                              
	/*************�����ܿ���������***************************
 	      				���ֽڷ���ʱ��
 	      |												|  |  		 	   |
 				|<-----8 bits data----->|  |<---2+n--->|<--next char 
 				|												|  |        	 |
 	---|__|--|xx|xx|xx|xx|xx|xx|xx|xx|-----------|__|xx|xx|xxx
 	   |	|												|	 | 
 	   |	|												|	 |
 	-->|--|<-start bit---			 -->|--|<-even check bit---		
 	
 	*****************************************************/   
 	
 	switch(i)
 	{  
 		case	14:        //1 etu start 	
 		{
 			/*******����������ԭ����ǰ�����������ŵ�forǰ����***/
 			#asm("nop");
			iomode=1;  //io���� ����Ϊ���	
			/*******����������ԭ����ǰ�����������ŵ�forǰ����***/
		
 			checkbit=0;//��λżУ��λ
 			/*������ʼλ*/ 
 			io=0;								
 			i--;	// i == 13
 			#asm("nop");          
 			break; 
 		}
 		/*��������λ*/  
 		case	13: 	//2 etu start 
 		{  
 			if(commanddata[j]&0x01)  //��Ϊ��λ��Ϣ�涨����Լ���������ȴ���λ��������λ  
 			{
 				io=1;   			         	
 				checkbit++; //��������λ�е���ż��
 			}  
 			else
 			io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;   //i == 12     
 			break;
 		}
 		case	12:     //3 etu start 
 		{  
 			if(commanddata[j]&0x01)  //��Ϊ��λ��Ϣ�涨����Լ���������ȴ���λ��������λ  
 			{
 				io=1;   			         	
 				checkbit++;    	//��������λ�е���ż��
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;     //i == 11     
 			break;
 		}
 		case	11:      //4 etu start 
 		{  
 			if(commanddata[j]&0x01)  //��Ϊ��λ��Ϣ�涨����Լ���������ȴ���λ��������λ  
 			{
 				io=1;   			         	
 				checkbit++;     //��������λ�е���ż��
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;     //i == 10     
 			break;
 		} 
 		case	10:      //5 etu start 
 		{  
 			if(commanddata[j]&0x01)  //��Ϊ��λ��Ϣ�涨����Լ���������ȴ���λ��������λ  
 			{
 				io=1;   			         	
 				checkbit++;      //��������λ�е���ż��
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;      //i == 9     
 			break;
 		}
 		case	9:        //6 etu start 
 		{  
 			if(commanddata[j]&0x01)  //��Ϊ��λ��Ϣ�涨����Լ���������ȴ���λ��������λ  
 			{
 				io=1;   			         	
 				checkbit++;     //��������λ�е���ż��
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;      //i == 8     
 			break;
 		}
 		case	8:        //7 etu start 
 		{  
 			if(commanddata[j]&0x01)  //��Ϊ��λ��Ϣ�涨����Լ���������ȴ���λ��������λ  
 			{
 				io=1;   			         	
 				checkbit++;      //��������λ�е���ż��
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;      //i == 7     
 			break;
 		}                 //8 etu 
 		case	7: 
 		{  
 			if(commanddata[j]&0x01)  //��Ϊ��λ��Ϣ�涨����Լ���������ȴ���λ��������λ  
 			{
 				io=1;   			         	
 				checkbit++;      //��������λ�е���ż��
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;      //i == 6     
 			break;
 		}
 		case	6:        //9 etu 
 		{  
 			if(commanddata[j]&0x01)  //��Ϊ��λ��Ϣ�涨����Լ���������ȴ���λ��������λ  
 			{
 				io=1;   			         	
 				checkbit++;      //��������λ�е���ż��
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;                     //i == 5     
 			break;
 		}
 		case	5:  	//10 etu start
 		{
 			/*������żλ*/    
 		
 			if(checkbit)       	
 				io=1;
 			else
 				io=0;  
			checkbit=0;	//��λ��żλ���Ա�ͳ����һ���ֽڵ���żλ����  
			i--;            // i == 4  
			break;
 		}         
 		case	4:  	//11 etu start
 		{
 			/*���鿨�Ƿ���ȷ��������*/   
 			#asm("nop");
			iomode=0;	//io���� ����Ϊ���� 
			io=1;		//ʹ����������   
			delayhalfetu(); 				
 			if(PINB.2==0)  
 			{
 				transimitbit=1;	  //�������մ�����һ���ֽ���Ҫ�ط�  
 				temp=j;
 			}
 			i--;    // i == 3 
 			break;
 		}
 		case	3:       //12 etu start
 		{
 			i--;	// i == 2
 			break;
 		}   
 		case 2:         //13 etu start
 		{
 			i--;	// i == 1
 			break;	
 		} 
 		case	1:      //14 etu start
 		{
 			i--;	// i == 0
 			break; 
 		}
 		case	0:      //15 etu start
 		{
 			i=14;
 			j++;
 			if(j==commandlength)   
 			{   
 				j=0;
 				TCCR1B=0x00;	//�رշ��Ͷ�ʱ��������ͽ���		 
 				recieveresponse();
			 	transmitresponse();
			 	commandtype=0xFF;	
 			}
 			break;
 		}	          		
 	}
  	if (transimitbit == 1)    // added by zouzhenhua
        {                       
//  	        test(temp);
  	        transimitbit = 0;
  	}        
}        


// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
	// Place your code here 
	/******************************************************* 
	
	9600ʱ,����һ���ֽ�Ҫ1040us,����TCNT0=0xDE�Ǽ��ޣ�(0xFF-0xDE) x 32us=1056us,
	��ʱ��0xFF-0xCD) x 32us = 1600us�ǱȽ����׵ġ����������ʱ���ڸ���9600�Ĳ����ʶ������õġ�
	********************************************************/ 
	
 	TCNT0=0xCD;    // һ�������жϣ�����һ���������������
 	TCCR0=0x00;	//���Թرմ��ڽ��ճ�ʱ��ʱ��
  	commandlength=rx_wr_index;//��¼��������ܳ���   
 	rx_wr_index=0;   //׼����һ���������

        // ���ڰ��յ������ݽ��л��� 	
//         for (c = 0; c < commandlength; c++)
//                 test(commanddata[c]);
 	  	 
	if(commanddata[0]==0x20)  //ר������   commanddata
 	{
 		if(commanddata[4]==0x01)//��λ���ܿ�  2000000001   
 		{
 			commandtype=0xFD;
 			responsedata[0]=0x90;  //����9000
 			responsedata[1]=0x00;
 			responselength=0x02;
 			transmitresponse();  
 		}
 		else if(commanddata[4]==0x02)
 		{
 			commandtype=0xFE;    	//�ر����ܿ�	2000000002 
 			responsedata[0]=0x90; //����9000
 			responsedata[1]=0x00;
 			responselength=0x02;
 			transmitresponse(); 
 		}
 		else
 		{
 			responsedata[0]=0x6F; 
 			responsedata[1]=0x00;
 			responselength=0x02;
 			transmitresponse();
 		}
 	}
 	else											//���ܿ�����(��һ,��,��,����������������ϲ��������,��Ƭ������ֻ��ת��)
 	{ 
// 		**ͳһ���������࣬���๤�����ϲ��������**     
//                test(commanddata[0]);
//                test(commanddata[1]);
//                test(commanddata[2]);
//                test(commanddata[3]);
//                test(commanddata[4]);                                                                
 		commandtype=0xFC;   
 	}
  
}

// USART Receiver interrupt service routine
#pragma savereg-
interrupt [USART_RXC] void uart_rx_isr(void)
{

		/********************************************************   

		ͨ�����ڽ������ܿ���������Զ��ж�һ�����������Ƿ������  

		������
		�뿪���ڽ����ж���������ʱ���������ж���رն�ʱ��������   
		һ����ʱ�жϷ������������յ�һ���������         

		********************************************************/  
	char status,data;
	#asm
    		push r26
    		push r27
			push r30
    		push r31
    		in   r26,sreg
    		push r26
	#endasm                
		
	TCCR0=0x00;//�رմ��ڽ��ճ�ʱ��ʱ��
	status=UCSRA;
	data=UDR;
 	if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
   	{
   		commanddata[rx_wr_index]=data;
   		
   		if (++rx_wr_index == RX_BUFFER_SIZE) 
   		{
   		 	rx_wr_index=0;
   		}               
		   		
   		if (++rx_counter == RX_BUFFER_SIZE) //�жϽ��ջ����Ƿ����
      		{
      			rx_counter=0;
      			rx_buffer_overflow=1;
      		};
   	};    
   	                                   
	TCNT0=0xCD;
	TCCR0=0x04;//�������ڽ��ճ�ʱ��ʱ*/		
	#asm
    		pop  r26
    		out  sreg,r26
    		pop  r31
			pop  r30
    		pop  r27
    		pop  r26
	#endasm
}
#pragma savereg+

void main(void)
{        
// Input/Output Ports initialization
// Port B initialization
// Func0=In Func1=In Func2=In Func3=In Func4=In Func5=In Func6=In Func7=In 
// State0=T State1=T State2=T State3=T State4=T State5=T State6=T State7=T 
PORTB=0x00;
DDRB=0x05;

// Port C initialization
// Func0=In Func1=In Func2=In Func3=In Func4=In Func5=In Func6=In 
// State0=T State1=T State2=T State3=T State4=T State5=T State6=T 
PORTC=0x00;
DDRC=0x00;

// Port D initialization
// Func0=In Func1=In Func2=In Func3=In Func4=In Func5=In Func6=In Func7=In 
// State0=T State1=T State2=T State3=T State4=T State5=T State6=T State7=T 
PORTD=0x00;
DDRD=0x00;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 8000.000 kHz
// Mode: Normal top=FFh
// OC0 output: Disconnected  
//������ʱ����256��Ƶ
TCCR0=0x00;   			
TCNT0=0xCD;//205   //256-205==51//�����жϽ���CG-100ת��������Ľ�����

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 8000.000 kHz
// Mode: Normal top=FFFFh
// OC1A output: Discon.                                               
// OC1B output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
TCCR1A=0x00;
TCCR1B=0x00;
TCNT1H=0xFC;   // 64702  //65536-64702=834
TCNT1L=0xBE; 		//��ʱ104.125uS,����9600�Ĳ�����ʱ��λ���ʡ�
OCR1AH=0x00;    //���������ܿ��������ݵķ��Ͷ�ʱ����
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer 2 Stopped
// Mode: Normal top=FFh
// OC2 output: Disconnected
ASSR=0x00;
TCCR2=0x00;
TCNT2=0x00;
OCR2=0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
MCUCR=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x05;

/*��Ƭ��������AC21C00֮��Ĳ�����Ϊ9600ʱ������*/
// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART Mode: Asynchronous
// USART Baud rate: 9600
UCSRA=0x00;
UCSRB=0x98;
UCSRC=0x86;
UBRRH=0x00;
UBRRL=0x33;   
PORTD|=0x01;//ʹ��rxd�˵���������

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
// Analog Comparator Output: Off
ACSR=0x80;
SFIOR=0x00;

//set stack  at the end internal SRAM
SPL=0x5F;
SPH=0x04;    

// Global enable interrupts
#asm("sei")

// Watchdog Timer initialization
// Watchdog Timer Prescaler: OSC/2048k
WDTCR=0x18;		//���ÿ��Ź���ʱ������ʱ2.2s��   
WDTCR=0x0F;                               

OSCCAL = 0xB0;          // ����У�����Ĵ�����8MHz�����1.8%

//c = 0;
// while(1) {test(c++); #asm("wdr");delay_ms(10);}
//test(0x05);                                    
//iomode = 1;
//io = 1;
//while (1) {delayhalfetu();io = ~io;#asm("wdr");}

// while(1) {Reset();#asm("wdr");}   
// flag = 0;    
// 
// iomode = 1;
// io = 1;
// TCCR1B=0x01;
// while(1) 
// {
//         #asm("wdr");
// }                 
               

//while(1) {test(0x4e); #asm("wdr"); delay_ms(100);}

while (1)
      {
      		switch(commandtype)
  		{  
       			 case  0xFC:        //ת���Ӵ��ڽ��յ��Ŀ�����
  			{
 //					test(commandlength);											
					j=0;  //������ֽ�ָ�븴λ��ָ�������commanddata[0]��һ���ֽ�									
					TCCR1B=0x01;      
					commandtype=0xFF;//�ڷ����������ҽ��ղ�������Ӧ���Ѿ���commandtype=0xFF,����ظ���Ϊ�˳����Ƚ�		
					break;
  			}     
  			case  0xFD:          //����λ������
  			{                     
 					responselength=22;	//��λ��Ϣ�ĳ��� 
 					Reset();                //��λ����
 					transmitresponse(); 	//���ظ�λ��Ϣ22�ֽ�
					commandtype=0xFF;					
					break ;	
  			}
  			case  0xFE:         //����رտ�����
  			{       
  				Close(); //�ؿ�����
  				commandtype=0xFF;
  				break;
  			}
  			default :  
  			{  
  				break;
  			}
  		}   
  		#asm("nop");  		
 	   	#asm("wdr");
       };
}