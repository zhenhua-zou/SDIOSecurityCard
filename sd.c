/*********************************************
This program was produced by the
CodeWizardAVR V1.23.8d Standard
Automatic Program Generator
© Copyright 1998-2003 HP InfoTech s.r.l.
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

#define	RX_BUFFER_SIZE 	255   	// USART Receiver buffer,½è¼ø×Ô¶¯Éú³ÉµÄ´®¿Ú´úÂë
#define	RESPONSE_SIZE	255     //ÖÇÄÜ¿¨ÏìÓ¦µÄ×Ö½Ú³¤¶È
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


/*********************ÖÇÄÜ¿¨ÃüÁî±äÁ¿Çø**********************/
unsigned char	commanddata[RX_BUFFER_SIZE];  //commanddata[] Êı×éÓÃÓÚ´ÓPDA½ÓÊÕµÄÖÇÄÜ¿¨ÃüÁî*****
unsigned char	responsedata[RESPONSE_SIZE]; 	//responsedata[]ÃüÁîÏìÓ¦Êı×é
unsigned char	commandlength=5;              //´Ó´®¿Ú½ÓÊÕµ½ÃüÁî³¤¶È 
unsigned char	commandtype=0xFD;            	//ÃüÁîÀàĞÍ
unsigned char  	responselength=22;        	//µÈÓÚ¸´Î»ĞÅÏ¢µÄ³¤¶È
unsigned char	i=14;                         //·¢ËÍÃüÁîµÄÎ»Ö¸Õë
unsigned char  	j=0;                          //·¢ËÍÃüÁîµÄ×Ö½ÚÖ¸Õë   
/*********************Î»±äÁ¿¶¨ÒåÇø*************************/
bit checkbit=0;                   
bit resetbit=0;
bit transimitbit=0;  
bit recievebit=0;        
unsigned char temp=0;
 
void test(unsigned char i)
{
        if(UCSRA.6) // TXC==1 ±íÃ÷ÉÏÒ»¸ö×Ö½Ú·¢ËÍ½áÊø
 	{
	     UCSRA.6=1;//Çå³ı·¢ËÍ½áÊø±êÖ¾Î»    
	}  
 	if(UCSRA.5) //UDRE==1 ±íÃ÷·¢ËÍ¼Ä´æÆ÷¿Õ,¿ÉÒÔ·¢ËÍĞÂ×Ö½Ú                           
 	{
 	     if(UCSRA.7==0)//RXC==0, ±íÃ÷½ÓÊÕÆ÷µ±Ç°Ã»ÓĞ½ÓÊÕµ½Êı¾İ£¬¿ÉÒÔ·¢ËÍ		
 	     UDR=i;     
 	} 
 	while(UCSRA.5==0)//µÈ´ıÉÏÒ»×Ö½Ú·¢Íê£¡
	{     
	     #asm("wdr");
	} 
}

/***********************×Ô¶¨Òåº¯ÊıÇø************************/
void delay100clk(void)//ÀíÂÛÑÓÊ±100clock=100x0.2793us=27.93us£¬¶ø¸Ãº¯ÊıÊµ¼ÊÑÓÊ±ÁË27.63uS
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

void delayhalfetu (void)   //Êµ¼Êdelay 0.5 etu 52.38uS ,¶øÖÇÄÜ¿¨if baud=9600,0.5etu=0.5x(1/9600)=52.083uS
{                    
	unsigned char i=0;
	
// 	for(i=0;i<67;i++)				//Ô­ÏÈÊÇ67¡£  
 	for(i=0;i<73;i++)                               //ÔÚÄÚ²¿¾§ÕñÏÂ£¬¼ÓÊ±ÖÓĞ£ÕıÏÂ£¬¹¤×÷±È½ÏÎÈ¶¨
            #asm("nop"); 		
} 

void delayoneetu (void)   //Êµ¼Êdelay 1 etu 104.13uS ,¶øÖÇÄÜ¿¨if baud=9600,1etu=1/9600=104.167uS
{                    
	unsigned char i=0;
	
//	for(i=0;i<135;i++)                
        for(i=0;i<135;i++)                              ////ÔÚÄÚ²¿¾§ÕñÏÂ£¬¼ÓÊ±ÖÓĞ£ÕıÏÂ£¬¹¤×÷±È½ÏÎÈ¶¨
            #asm("nop"); 
}

/*#define delay100clk() delay_us(30)
#define delay40000clk() delay_us(11175) 
#define delayhalfetu() delay_us(52)
#define delayoneetu() delay_us(104)*/

void Reset(void)
{
  /*****************¸´Î»Ê±¼äÒªÇó*****************************   
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
  ÖÇÄÜ¿¨Ê±ÖÓ²ÉÓÃ¶ÀÁ¢mega16LµÄÓĞÔ´¾§Ìå£¬ ÆµÂÊÎª3.579545MHz£¬ÉÏµç¼´Æô¶¯.     
  1¸öclockÖÜÆÚÎª 1/3.579545MHz=0.2794uS
  1¸öetu=372/3.579545MHz=103.923uS   
  BUAD=3.579545MHz/372=9622.4      
  
  ¶øÊµ¼ÊÉÏ£º°´ÕIÖÇÄÜ¿¨¹æ¶¨£BBUAD=9600,Ôòfi=9600 x 327 = 3.5712MHz
  1¸öclockÖÜÆÚÎª 1/3.5712MHz=0.280uS
  1¸öetu=372/3.579545MHz=104.167uS   
  
  ËùÒÔÓĞ¸öÊ±ÖÓÆ«²î¡£
  /*****************¸´Î»Ê±¼äÒªÇó*****************************/     
  
  unsigned char i=0;  
  unsigned char j=0;
  responselength = 0;  
  checkbit=0;   //¸´Î»ĞÅÏ¢Ğ£Ñé  
  resetbit=0;	//¸´Î»´íÎóÖ¸Ê¾Î»   
  
  /*****************¸´Î»ÖÇÄÜ¿¨******************************/
  #asm("nop");
  rst=0;   // rstÔÚÒÔT0ÎªÊ±¼äÆğµã£¬¿ªÊ¼ÖÃµÍ£¬±£³Öt3Ê±¼ä¡£
           //Æô¶¯Ê±ÖÓ ÆµÂÊÎª3.579545MHz.                 
  delay100clk(); //ioÔÚÒÔT0ÎªÊ±¼äÆğµã£¬t2Ê±¼äÄÚÖÃ³ÉZ×´Ì¬£¬ÎÒÑ¡ÔñÑÓÊ±100clock=100x0.2793us=27.93us
  iomode=0;      //ioÒı½Å ÉèÖÃÎªÊäÈë
  #asm("nop");
  delay40000clk();//t3Ê±¼ä¹ıºórstÖÃ¸ß£¬ÎÒÑ¡Ôñt3 40000clk=40000 x 0.2793us = 11172us
  rst=1;
  delay100clk();     	//ÖÁÉÙÔÚrst=1ºót1Ê±¼ä£¬ÖÇÄÜ¿¨²ÅÄÜ·¢¸´Î»ĞÅÏ¢
  delay100clk();
  delay100clk();
  delay100clk();     	//×îÉÙµÄµÈ´ı½áÊø 
   
  for(i=0;i<22;i++)  	//ÒòÎª¸´Î»ĞÅÏ¢¹²22¸ö×Ö½Ú£¬¹²176Î»
  {
   	while(PINB.2)	//µÈ´ı¸´Î»ĞÅÏ¢Õû¸ö×Ö½ÚµÄÆğÊ¼Î»

  	delayhalfetu();
  	for(j=0;j<8;j++)  //¶Á8¸öÊı¾İÎ»
  	{ 
  		delayoneetu();
  		responsedata[i]=responsedata[i]>>1; //¿Õ³öĞÂÎ»µÄ´æ´¢Î»ÖÃ  	
  		if(PINB.2)    	//¶ÁÈ¡PINA.2µÄ×´Ì¬£¬¼´ioÊı¾İ
  		{
  			responsedata[i]=responsedata[i]|0x80;	//Èç¹ûPINB.2==1£¬Ôò°Ñ¸´Î»ĞÅÏ¢Êı×éµÄµ±Ç°×î¸ßÎ»ÖÃ1£¬·ñÔò²»×÷´¦Àí¡£  
  			checkbit++;//¼ÆËã¸Ã×Ö½ÚÖĞ1µÄÆæÅ¼ĞÔ
  		}
  	}
    //µ±Ç°Ê±¿ÌÎª8.5etu	
    delayoneetu();//¸ÃÑÓÊ±ºó¿ÉÒÔ¶ÁÆæÅ¼Î»£¬9.5etu 
     
    /**´¦ÀíÆæÅ¼Î»**/
    if(PINB.2)	//µ±Ç°×Ö½Ú¶ÁÍêÁË,ÏÖÔÚ¶ÁÆæÅ¼Î»  
    	checkbit++;            
        
    if(checkbit) {
    	resetbit=1; 
        responsedata[i] = 0xEE; //added by zzh
//        return 0; //added by zzh 
        responselength = i + 1;
    }
    checkbit = 0; //added by zzh              
        
    responselength++;
        
    /**´¦ÀíÆæÅ¼Î»**/
    
    delayoneetu();		//´Ó¿¨Ïò½Ó¿ÚÉè±¸Á½¸ö×Ö·û¼ä¸ôÎª12¸öetu,²»Ğè¶îÍâµÄ±£»¤Ê±¼ä¡£¶ø½Ó¿ÚÉè±¸Ïò¿¨ÖÁÉÙĞèÒª12+n¸öetu
    delayoneetu();		//11.5etu
  }  
  delayoneetu();			//12.5etu    
}

void Close(void)
{
  /*****************¹Ø±ÕÖÇÄÜ¿¨******************************/
	rst=0; 
        #asm("nop");  
	iomode=1;						//ioÒı½Å ÉèÖÃÎªÊä³ö     
	io=0;                
	#asm("nop");                 
}

void recieveresponse (void)
{
	/****************½ÓÊÕÖÇÄÜ¿¨¶ÔÃüÁîµÄÏìÓ¦*********************/   	
 	unsigned char i=0;    
 	unsigned char j=0;   
 	unsigned int k=0;	//ÓÃÓÚÅĞ¶ÏÏìÓ¦ÊÇ·ñ½áÊø   
 	
	#asm("nop"); //1 us
	iomode=0;  //ioÒı½Å ÉèÖÃÎªÊäÈë    0.25us    
	io=1;	   //Ê¹ÄÜÉÏÀ­µç×è  
	responselength=0;//ÏìÓ¦³¤¶È¸´Î»
	for(i=0;i<RESPONSE_SIZE;i++) //0.5us
  	{
   	
   	/*************¸Ã²¿·Ö´úÂë×÷ÓÃÊÇÄÜ×Ô¶¯¼ÍÂ¼ÏìÓ¦µÄ³¤¶È****************/
   	
   	if(responselength)
   	{
   		while(PINB.2)	  	//µÈ´ıÏìÓ¦ĞÅÏ¢µÚ2¸ö×Ö½ÚµÄÆğÊ¼Î»------×îºóÒ»¸ö×Ö½ÚµÄÆğÊ¼Î»
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
				#asm("nop");  //16¸önopÑÓÊ±2us
				if(++k>500)   //¶ÔÓÚ2.1°æ±¾COSµÄ¹ı³Ì×Ö½ÚºÍÏìÓ¦Ö®¼ä¼ä¸ô²»³¬¹ı3.2ms,k>200ÎªÅĞ¾ö½áÊøÌõ¼ş¾ÍĞĞÁË¡£
			                      //µ«¶ÔÓÚ2.0°æ±¾COSµÄ¹ı³Ì×Ö½ÚºÍÏìÓ¦Ö®¼ä¼ä¸ô²»³¬¹ı4ms,k>250ÎªÅĞ¾ö½áÊøÌõ¼ş²ÅĞĞ¡£
				   return;    //Èç¹û´ó¸Å400us¹ıºó£¬PINB.2ÈÔÈ»Îª¸ß£¬ËµÃ÷ÏìÓ¦È«²¿½áÊø¡£
			} 
	} 
	else
	{
		while(PINB.2)	  	//µÈ´ıÏìÓ¦ĞÅÏ¢µÚÒ»¸ö×Ö½ÚµÄÆğÊ¼Î»  
		{       
			#asm("wdr");
		}	
	}    
	k=0;	 
			 
		/*************¸Ã²¿·Ö´úÂë×÷ÓÃÊÇÄÜ×Ô¶¯¼ÍÂ¼ÏìÓ¦µÄ³¤¶È****************/ 
	
  	delayhalfetu(); 	//44.88us
  	for(j=0;j<8;j++)  	//¶Á8¸öÊı¾İÎ»                          0.38us
  	{ 
  		delayoneetu();	                                    //104.13us          
  		responsedata[i]=responsedata[i]>>1; //¿Õ³öĞÂÎ»µÄ´æ´¢Î»ÖÃ 2.63us
  		if(PINB.2)    		//¶ÁÈ¡PINB.2µÄ×´Ì¬£¬¼´ioÊı¾İ
  		{
  			responsedata[i]=responsedata[i]|0x80;	//Èç¹ûPINB.2==1£¬Ôò°Ñ¸´Î»ĞÅÏ¢Êı×éµÄµ±Ç°×î¸ßÎ»ÖÃ1£¬2.63us
  								//·ñÔò²»×÷´¦Àí¡£0.38us
  			checkbit++;			//Í³¼Æ¸Ã×Ö½ÚÖĞ1µÄÆæÅ¼ĞÔ	
  		}    
  	}                 	//1.13us
  	//µ±Ç°Ê±¿ÌÎª8.5etu	
    delayoneetu();			//¸ÃÑÓÊ±ºó¿ÉÒÔ¶ÁÆæÅ¼Î»£¬9.5etu     
    
    /**´¦ÀíÆæÅ¼Î»**/
    if(PINB.2)				//µ±Ç°×Ö½Ú¶ÁÍêÁË,ÏÖÔÚ¶ÁÆæÅ¼Î»  
    	checkbit++;
    if(checkbit)
    {
    	recievebit=1;   	
    	responsedata[i] = 0xee; // added by zouzhenhua
    }
    checkbit = 0;
    /**´¦ÀíÆæÅ¼Î»**/	          
  
    delayoneetu();			//´Ó¿¨Ïò½Ó¿ÚÉè±¸Á½¸ö×Ö·û¼ä¸ôÎª12¸öetu,²»Ğè¶îÍâµÄ±£»¤Ê±¼ä¡£¶ø½Ó¿ÚÉè±¸Ïò¿¨ÖÁÉÙĞèÒª12+n¸öetu
//    delayoneetu();			//11.5etu
    responselength++;   //ÊÕµ½Ò»¸ö×Ö½ÚµÄÏìÓ¦£¬ÏìÓ¦³¤¶È¼Ó 1    
  }                      
}   
 
/************Ïò´®¿Ú·µ»ØÖÇÄÜ¿¨µÄÏìÓ¦***************/
void transmitresponse(void)
{               
	unsigned char i=0;        
	for(i=0;i<responselength;i++)   
	{                               
		if(UCSRA.6) // TXC==1 ±íÃ÷ÉÏÒ»¸ö×Ö½Ú·¢ËÍ½áÊø
 		{
		     UCSRA.6=1;//Çå³ı·¢ËÍ½áÊø±êÖ¾Î»    
		}  
 		if(UCSRA.5) //UDRE==1 ±íÃ÷·¢ËÍ¼Ä´æÆ÷¿Õ,¿ÉÒÔ·¢ËÍĞÂ×Ö½Ú                           
 		{
 		     if(UCSRA.7==0)//RXC==0, ±íÃ÷½ÓÊÕÆ÷µ±Ç°Ã»ÓĞ½ÓÊÕµ½Êı¾İ£¬¿ÉÒÔ·¢ËÍ		
 		     UDR=responsedata[i];     
 		} 
 		while(UCSRA.5==0)//µÈ´ıÉÏÒ»×Ö½Ú·¢Íê£¡
		{     
		     #asm("wdr");
		} 
 	} 	
} 

//Timer 1 overflow interrupt service routine
interrupt [TIM1_OVF] void timer1_ovf_isr(void)
{
//	Place your code here
 
// 	TCNT2=0x97;     //¶¨Ê±1/9600=104.17uS                       

        TCNT1L=0xBE;                                                     
//	TCNT1L=0xEA;  // ?????Õâ¸ö¶¨Ê±ÊÇÈçºÎ¼ÆËã³öÀ´µÄ??? zouzhenhua ·¢ËÍÃüÁî¸øÖÇÄÜ¿¨µÄ¶¨Ê±Æ÷µÄÊ±¼äÉèÖÃ

	TCNT1H=0xFC;                                              
	/*************ÏòÖÇÄÜ¿¨·¢ËÍÃüÁî***************************
 	      				µ¥×Ö½Ú·¢ËÍÊ±Ğò
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
 			/*******ÕâÁ½Ìì´íÎóµÄÔ­Òò¾ÍÊÇ°ÑÏÂÃæÕâÁ½¾ä·Åµ½forÇ°ÃæÁË***/
 			#asm("nop");
			iomode=1;  //ioÒı½Å ÉèÖÃÎªÊä³ö	
			/*******ÕâÁ½Ìì´íÎóµÄÔ­Òò¾ÍÊÇ°ÑÉÏÃæÕâÁ½¾ä·Åµ½forÇ°ÃæÁË***/
		
 			checkbit=0;//¸´Î»Å¼Ğ£ÑéÎ»
 			/*·¢ËÍÆğÊ¼Î»*/ 
 			io=0;								
 			i--;	// i == 13
 			#asm("nop");          
 			break; 
 		}
 		/*·¢ËÍÊı¾İÎ»*/  
 		case	13: 	//2 etu start 
 		{  
 			if(commanddata[j]&0x01)  //ÒòÎª¸´Î»ĞÅÏ¢¹æ¶¨ÕıÏòÔ¼¶¨£¬ËùÒÔÏÈ´«µÍÎ»£¬×îºó´«×î¸ßÎ»  
 			{
 				io=1;   			         	
 				checkbit++; //¼ÆËãÊı¾İÎ»ÖĞµÄÆæÅ¼ĞÔ
 			}  
 			else
 			io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;   //i == 12     
 			break;
 		}
 		case	12:     //3 etu start 
 		{  
 			if(commanddata[j]&0x01)  //ÒòÎª¸´Î»ĞÅÏ¢¹æ¶¨ÕıÏòÔ¼¶¨£¬ËùÒÔÏÈ´«µÍÎ»£¬×îºó´«×î¸ßÎ»  
 			{
 				io=1;   			         	
 				checkbit++;    	//¼ÆËãÊı¾İÎ»ÖĞµÄÆæÅ¼ĞÔ
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;     //i == 11     
 			break;
 		}
 		case	11:      //4 etu start 
 		{  
 			if(commanddata[j]&0x01)  //ÒòÎª¸´Î»ĞÅÏ¢¹æ¶¨ÕıÏòÔ¼¶¨£¬ËùÒÔÏÈ´«µÍÎ»£¬×îºó´«×î¸ßÎ»  
 			{
 				io=1;   			         	
 				checkbit++;     //¼ÆËãÊı¾İÎ»ÖĞµÄÆæÅ¼ĞÔ
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;     //i == 10     
 			break;
 		} 
 		case	10:      //5 etu start 
 		{  
 			if(commanddata[j]&0x01)  //ÒòÎª¸´Î»ĞÅÏ¢¹æ¶¨ÕıÏòÔ¼¶¨£¬ËùÒÔÏÈ´«µÍÎ»£¬×îºó´«×î¸ßÎ»  
 			{
 				io=1;   			         	
 				checkbit++;      //¼ÆËãÊı¾İÎ»ÖĞµÄÆæÅ¼ĞÔ
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;      //i == 9     
 			break;
 		}
 		case	9:        //6 etu start 
 		{  
 			if(commanddata[j]&0x01)  //ÒòÎª¸´Î»ĞÅÏ¢¹æ¶¨ÕıÏòÔ¼¶¨£¬ËùÒÔÏÈ´«µÍÎ»£¬×îºó´«×î¸ßÎ»  
 			{
 				io=1;   			         	
 				checkbit++;     //¼ÆËãÊı¾İÎ»ÖĞµÄÆæÅ¼ĞÔ
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;      //i == 8     
 			break;
 		}
 		case	8:        //7 etu start 
 		{  
 			if(commanddata[j]&0x01)  //ÒòÎª¸´Î»ĞÅÏ¢¹æ¶¨ÕıÏòÔ¼¶¨£¬ËùÒÔÏÈ´«µÍÎ»£¬×îºó´«×î¸ßÎ»  
 			{
 				io=1;   			         	
 				checkbit++;      //¼ÆËãÊı¾İÎ»ÖĞµÄÆæÅ¼ĞÔ
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;      //i == 7     
 			break;
 		}                 //8 etu 
 		case	7: 
 		{  
 			if(commanddata[j]&0x01)  //ÒòÎª¸´Î»ĞÅÏ¢¹æ¶¨ÕıÏòÔ¼¶¨£¬ËùÒÔÏÈ´«µÍÎ»£¬×îºó´«×î¸ßÎ»  
 			{
 				io=1;   			         	
 				checkbit++;      //¼ÆËãÊı¾İÎ»ÖĞµÄÆæÅ¼ĞÔ
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;      //i == 6     
 			break;
 		}
 		case	6:        //9 etu 
 		{  
 			if(commanddata[j]&0x01)  //ÒòÎª¸´Î»ĞÅÏ¢¹æ¶¨ÕıÏòÔ¼¶¨£¬ËùÒÔÏÈ´«µÍÎ»£¬×îºó´«×î¸ßÎ»  
 			{
 				io=1;   			         	
 				checkbit++;      //¼ÆËãÊı¾İÎ»ÖĞµÄÆæÅ¼ĞÔ
 			}  
 			else
 				io=0;
 			commanddata[j]=commanddata[j]>>1;
 			i--;                     //i == 5     
 			break;
 		}
 		case	5:  	//10 etu start
 		{
 			/*·¢ËÍÆæÅ¼Î»*/    
 		
 			if(checkbit)       	
 				io=1;
 			else
 				io=0;  
			checkbit=0;	//¸´Î»ÆæÅ¼Î»£¬ÒÔ±ãÍ³¼ÆÏÂÒ»¸ö×Ö½ÚµÄÆæÅ¼Î»¸öÊı  
			i--;            // i == 4  
			break;
 		}         
 		case	4:  	//11 etu start
 		{
 			/*¼ìÑé¿¨ÊÇ·ñÕıÈ·½ÓÊÕÃüÁî*/   
 			#asm("nop");
			iomode=0;	//ioÒı½Å ÉèÖÃÎªÊäÈë 
			io=1;		//Ê¹ÄÜÉÏÀ­µç×è   
			delayhalfetu(); 				
 			if(PINB.2==0)  
 			{
 				transimitbit=1;	  //±íÃ÷½ÓÊÕ´íÎó£¬ÉÏÒ»¸ö×Ö½ÚĞèÒªÖØ·¢  
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
 				TCCR1B=0x00;	//¹Ø±Õ·¢ËÍ¶¨Ê±Æ÷£¬ÃüÁî·¢ËÍ½áÊø		 
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
	
	9600Ê±,·¢ËÍÒ»¸ö×Ö½ÚÒª1040us,ËùÒÔTCNT0=0xDEÊÇ¼«ÏŞ£¬(0xFF-0xDE) x 32us=1056us,
	¶¨Ê±£¨0xFF-0xCD) x 32us = 1600usÊÇ±È½ÏÎÈÍ×µÄ¡£¶øÇÒÕâ¸ö¶¨Ê±¶ÔÓÚ¸ßÓÚ9600µÄ²¨ÌØÂÊ¶¼ÊÇÊÊÓÃµÄ¡£
	********************************************************/ 
	
 	TCNT0=0xCD;    // Ò»µ©½øÈëÖĞ¶Ï£¬±íÃ÷Ò»¸öÍêÕûÃüÁî½áÊø£¬
 	TCCR0=0x00;	//ËùÒÔ¹Ø±Õ´®¿Ú½ÓÊÕ³¬Ê±¶¨Ê±Æ÷
  	commandlength=rx_wr_index;//¼ÇÂ¼ÏÂÃüÁîµÄ×Ü³¤¶È   
 	rx_wr_index=0;   //×¼±¸ÏÂÒ»´ÎÃüÁî½ÓÊÕ

        // ÓÃÓÚ°ÑÊÕµ½µÄÊı¾İ½øĞĞ»ØÏÔ 	
//         for (c = 0; c < commandlength; c++)
//                 test(commanddata[c]);
 	  	 
	if(commanddata[0]==0x20)  //×¨ÓĞÃüÁî   commanddata
 	{
 		if(commanddata[4]==0x01)//¸´Î»ÖÇÄÜ¿¨  2000000001   
 		{
 			commandtype=0xFD;
 			responsedata[0]=0x90;  //·µ»Ø9000
 			responsedata[1]=0x00;
 			responselength=0x02;
 			transmitresponse();  
 		}
 		else if(commanddata[4]==0x02)
 		{
 			commandtype=0xFE;    	//¹Ø±ÕÖÇÄÜ¿¨	2000000002 
 			responsedata[0]=0x90; //·µ»Ø9000
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
 	else											//ÖÇÄÜ¿¨ÃüÁî(µÚÒ»,¶ş,Èı,ËÄÀàÃüÁîµÄÇø·ÖÓÉÉÏ²ãÇı¶¯Íê³É,µ¥Æ¬»úÏÖÔÚÖ»ÊÇ×ª·¢)
 	{ 
// 		**Í³Ò»´¦Àí£¬²»·ÖÀà£¬·ÖÀà¹¤×÷ÓÉÉÏ²ãÇı¶¯Íê³É**     
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

		Í¨¹ı´®¿Ú½ÓÊÕÖÇÄÜ¿¨µÄÃüÁî£¬²¢×Ô¶¯ÅĞ¶ÏÒ»¸öÍêÕûÃüÁîÊÇ·ñ½áÊø¡£  

		·½·¨£º
		Àë¿ª´®¿Ú½ÓÊÕÖĞ¶ÏÀïÆô¶¯¶¨Ê±Æ÷£¬½øÈëÖĞ¶ÏÔò¹Ø±Õ¶¨Ê±Æ÷£¬ÕâÑù   
		Ò»µ©¶¨Ê±ÖĞ¶Ï·¢Éú£¬±íÃ÷ÒÑÊÕµ½Ò»¸öÍêÕûÃüÁî¡£         

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
		
	TCCR0=0x00;//¹Ø±Õ´®¿Ú½ÓÊÕ³¬Ê±¶¨Ê±Æ÷
	status=UCSRA;
	data=UDR;
 	if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
   	{
   		commanddata[rx_wr_index]=data;
   		
   		if (++rx_wr_index == RX_BUFFER_SIZE) 
   		{
   		 	rx_wr_index=0;
   		}               
		   		
   		if (++rx_counter == RX_BUFFER_SIZE) //ÅĞ¶Ï½ÓÊÕ»º³åÊÇ·ñÒç³ö
      		{
      			rx_counter=0;
      			rx_buffer_overflow=1;
      		};
   	};    
   	                                   
	TCNT0=0xCD;
	TCCR0=0x04;//Æô¶¯´®¿Ú½ÓÊÕ³¬Ê±¶¨Ê±*/		
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
//ÓÃ×÷¶¨Ê±Æ÷£¬256·ÖÆµ
TCCR0=0x00;   			
TCNT0=0xCD;//205   //256-205==51//ÓÃÓÚÅĞ¶Ï½ÓÊÕCG-100×ª·¢µÄÃüÁîµÄ½áÊø¡£

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
TCNT1L=0xBE; 		//¶¨Ê±104.125uS,½üËÆ9600µÄ²¨ÌØÂÊÊ±µÄÎ»ËÙÂÊ¡£
OCR1AH=0x00;    //ÓÃÓÚÏòÖÇÄÜ¿¨·¢ËÍÊı¾İµÄ·¢ËÍ¶¨Ê±Æ÷¡£
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

/*µ¥Æ¬»ú´®¿ÚÓëAC21C00Ö®¼äµÄ²¨ÌØÂÊÎª9600Ê±µÄÉèÖÃ*/
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
PORTD|=0x01;//Ê¹ÄÜrxd¶ËµÄÉÏÀ­µç×è

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
WDTCR=0x18;		//ÉèÖÃ¿´ÃÅ¹·¶¨Ê±Æ÷£¬¶¨Ê±2.2s¡£   
WDTCR=0x0F;                               

OSCCAL = 0xB0;          // ¾§ÕñĞ£Õı»ú¼Ä´æÆ÷£¬8MHz£¬Îó²î1.8%

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
       			 case  0xFC:        //×ª·¢´Ó´®¿Ú½ÓÊÕµ½µÄ¿¨ÃüÁî
  			{
 //					test(commandlength);											
					j=0;  //ÃüÁîµÄ×Ö½ÚÖ¸Õë¸´Î»£¬Ö¸ÏòÃüÁîµÄcommanddata[0]µÚÒ»¸ö×Ö½Ú									
					TCCR1B=0x01;      
					commandtype=0xFF;//ÔÚ·¢ËÍÍêÃüÁîÇÒ½ÓÊÕ²¢·µ»ØÏìÓ¦ºóÒÑ¾­Áîcommandtype=0xFF,Õâ¸öÖØ¸´£¬ÎªÁË³ÌĞòÎÈ½¡		
					break;
  			}     
  			case  0xFD:          //´¦Àí¸´Î»¿¨ÃüÁî
  			{                     
 					responselength=22;	//¸´Î»ĞÅÏ¢µÄ³¤¶È 
 					Reset();                //¸´Î»º¯Êı
 					transmitresponse(); 	//·µ»Ø¸´Î»ĞÅÏ¢22×Ö½Ú
					commandtype=0xFF;					
					break ;	
  			}
  			case  0xFE:         //´¦Àí¹Ø±Õ¿¨ÃüÁî
  			{       
  				Close(); //¹Ø¿¨º¯Êı
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