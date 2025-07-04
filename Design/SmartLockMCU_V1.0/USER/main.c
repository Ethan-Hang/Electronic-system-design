
#include "main.h"

#include <string.h>

SysTemPat sys;

#define MAXERRTIMES 5
#define usart2_baund  57600//串口2波特率，根据指纹模块波特率更改

//要写入到STM32 FLASH的字符串数组
const u8 TEXT_Buffer[]={0x17,0x23,0x6f,0x60,0,0};
#define TEXT_LENTH sizeof(TEXT_Buffer)	 		  	//数组长度	
#define SIZE TEXT_LENTH/4+((TEXT_LENTH%4)?1:0)
#define FLASH_SAVE_ADDR  0X0802C124 	//设置FLASH 保存地址(必须为偶数，且所在扇区,要大于本代码所占用到的扇区.
										//否则,写操作的时候,可能会导致擦除整个扇区,从而引起部分程序丢失.引起死机.

SysPara AS608Para;//指纹模块AS608参数
u16 ValidN;//模块内有效指纹个数
u8** kbd_tbl;

void Display_Data(void);//显示时间
void Add_FR(void);	//录指纹
void Del_FR(void);	//删除指纹
int press_FR(void);//刷指纹
void ShowErrMessage(u8 ensure);//显示确认码错误信息
int password(void);//密码锁
void SetPassworld(void);//修改密码
void starting(void);//开机界面信息
u8 MFRC522_lock(void);//刷卡解锁
u8 Add_Rfid(void);		//录入
void Set_Time(void);
void Massige(void);
void SysPartInit(void );   //系统参数初始化 
//u8 Pwd[7]="      ";  //解锁密码1
//u8 Pwd2[7]="      ";  //解锁密码2
//u8 cardid[6]={0,0,0,0,0,0};  //卡号1
int Error;  //密码验证信息


u8 DisFlag = 1;



//数字的ASCII码
uc8 numberascii[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
//显示缓冲区
u8  dispnumber5buf[6];
u8  dispnumber3buf[4];
u8  dispnumber2buf[3];
//MFRC522数据区
u8  mfrc552pidbuf[18];
u8  card_pydebuf[2];
u8  card_numberbuf[5];
u8  card_key0Abuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
u8  card_writebuf[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
u8  card_readbuf[18];
//SM05-S数据区
u8  sm05cmdbuf[15]={14,128,0,22,5,0,0,0,4,1,157,16,0,0,21};
//extern声明变量已在外部的C文件里定义，可以在主文件中使用
extern u8  sm05receivebuf[16];	//在中断C文件里定义
extern u8  sm05_OK;							//在中断C文件里定义

//u8 * week[7]={"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
u8 * week[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
u8 * setup[7]={"1、录入指纹","2、删除指纹","3、修改密码","4、修改时间","5、录入卡片","6、查看信息"};


void DisUnLock(void )
{
	OLED_Clear();
	Show_Str(20,10,128,24,"解锁中...",24,0);	
	OLED_Refresh_Gram();//更新显示
	Walkmotor_ON();
	Show_Str(20,10,128,24,"已解锁！",24,0);
	OLED_Refresh_Gram();//更新显示
	delay_ms(1500);
}

void DisLock(void )
{
	OLED_Clear();
	Show_Str(30,20,128,16,"锁定中！",16,0);
	OLED_Refresh_Gram();//更新显示
	Walkmotor_OFF();
	Show_Str(30,20,128,16,"已锁定！",16,0);
	OLED_Show_Font(56,48,0);//锁
	OLED_Refresh_Gram();//更新显示
	delay_ms(1000);
}



 int main(void)
 {			
	u16 set=0;
	 u8 err=0;
	int key_num;
	int time1;
	int time2;		//锁屏时间
	char arrow=0;  //箭头位子
	//SysHSI_Init();
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 //串口初始化为9600
	printf("串口功能正常\r\n");
	Button4_4_Init();          //初始化与按键连接的硬件接口
	OLED_Init();    			//显示初始化
	Walkmotor_Init();        //步进电机初始化
	BEEP_Init();			//蜂鸣器初始化
	usart2_init(usart2_baund);           //初始化指纹模块
	PS_StaGPIO_Init();
	OLED_Clear(); 
	 
	starting();//开机信息  logo
	err = RTC_Init();	  			//RTC初始化
	if(err)
	{
		OLED_Clear(); 
		Show_Str(12,13,128,20,"RTC CRY ERR!",12,0); 
		OLED_Refresh_Gram();//更新显示
		delay_ms(3000);
	}
	SysPartInit();   //系统参数初始化 
 	while(1)
	{
//锁屏界面
MAIN:
			OLED_Clear(); 
			OLED_Show_Font(56,48,0);//显示锁图标
			while(1)
			{
				time1++;Display_Data();//时间显示：每1000ms更新一次显示数据
				
				if(DisFlag == 1)
				{
					DisFlag = 0;
					OLED_Fill(0,24,16,63,0);
					OLED_Refresh_Gram();//更新显示
				}
				
				if((time1%100)==1)
				{
					//MFRC522解锁
						time1=0;
						MFRC522_Initializtion();			
						Error=MFRC522_lock();
						if(Error==0)
						{
							goto MENU;	
						}
						else 
						{
							OLED_Show_Font(56,48,0);//锁
						}
						
						//手机蓝牙解锁密码1
						Error=usart1_cherk((char*)sys.passwd1);         
						if(Error==0){ 
							OLED_Clear_NOupdate();
							Show_Str(12,13,128,20,"蓝牙密码1：正确",12,0); 
							OLED_Refresh_Gram();//更新显示
							delay_ms(800);
							DisUnLock();
							goto MENU;	
						}
						else 
						{
//							OLED_Clear_NOupdate();
//							Show_Str(12,13,128,12,"蓝牙密码：错误！",12,0); 
//							OLED_Refresh_Gram();//更新显示
//							delay_ms(800);
//							OLED_Show_Font(56,48,0);//锁
						}
						
						//手机蓝牙解锁密码2
						Error=usart1_cherk((char*)sys.passwd2);         
						if(Error==0){
							sys.errCnt = 0;
							OLED_Clear_NOupdate();
							Show_Str(12,13,128,12,"蓝牙密码2：正确",12,0); 
							OLED_Refresh_Gram();//更新显示
							delay_ms(800);
							DisUnLock();
							goto MENU;	
						}
						else 
						{
							//OLED_Show_Font(56,48,0);//锁
						}
						
				}
				//指纹解锁
				if(PS_Sta)	 //检测PS_Sta状态，如果有手指按下
				{
						while(PS_Sta){
						Error=press_FR();//刷指纹
						if(Error==0)
						{
							//DisUnLock();
							goto MENU;   //跳到解锁界面
						}								
						else 
						{
							OLED_Show_Font(56,48,0);//锁
						}
					}
				}
				//密码锁
				key_num=Button4_4_Scan();	//按键扫描
				if(key_num!=-1)
				{
					Error=password();//密码解锁函数
					if(Error==0)
					{
						goto MENU;	//跳到解锁界面
					}
					else 
					{
						OLED_Show_Font(56,48,0);//锁
					}
				}
				
				delay_ms(1);
				
				
				
			}
			/********************主界面**************************/

MENU:
			OLED_Clear();
MENUNOCLR:
			OLED_Fill(0,0,20,48,0);
			//主页菜单显示
			if(arrow<3){
				Show_Str(5,arrow*16,128,16,"->",16,0);//显示箭头
				set=0;}
			else {
				Show_Str(5,(arrow-3)*16,128,16,"->",16,0);
				set=3;}
			Show_Str(25,0,128,16,setup[set],16,0);
			Show_Str(25,16,128,16,setup[set+1],16,0);
			Show_Str(25,32,128,16,setup[set+2],16,0);
			Show_Str(0,52,128,12,"上    下     确定",12,0);
			OLED_Refresh_Gram();//更新显示
			time2=0;
			while(1)
			{
						//超时锁屏
						time2++;
						if(time2>10000 | key_num==4){  
							
							OLED_Clear();
								DisLock();
								if(time2>10000)beep_on_mode2();
								time2 =0;
//								delay_ms(1000);
								OLED_Clear();
								goto MAIN;
						}
						//手机蓝牙锁定
						if(memcmp(USART_RX_BUF,"LOCK",4)==0)	{
//							USART_RX_STA=0;
//							memset(USART_RX_BUF,0,USART_REC_LEN);
							DisLock();
							goto MAIN;
						}
						
						//功能选项选择
						key_num=Button4_4_Scan();	
						if(key_num)
						{
							if(key_num==13){
								if(arrow>0)arrow--;
								goto MENUNOCLR;
							}
							if(key_num==15){
								if(arrow<5)arrow++;
								goto MENUNOCLR;
							}
							if(key_num==16){
								switch(arrow)
								{
									case 0:Add_FR();		break;//录指
									case 1:Del_FR();		break;//删指纹
									case 2:SetPassworld();break;//修改密码
									case 3:Set_Time(); break;  //设置时间
									case 4:Add_Rfid(); break;  //录入卡片
									case 5:Massige(); break;  //显示信息
//									
								}
								goto MENU;
							}		
						}delay_ms(1);
			}	
	}//while
			
			
			
			
		
		 
 }
 
 u8 DisErrCnt(void)
 {
	 int time=0;
	 u8 buf[64];
	 if(sys.errTime>0)//错误次数计数
	{
		
		OLED_Clear();
		while(1)
		{
			if(time++ == 1000)
			{
				time = 0;
				if(sys.errTime==0)
				{
					OLED_Clear();
					break;
				}
				Show_Str(0,16,128,16,"密码错误次数过多",16,0);
				sprintf(buf,"请%02d秒后重试", sys.errTime);
				Show_Str(20,32,128,16,buf,16,0);
				OLED_Refresh_Gram();//更新显示
			}
			delay_ms(1);
			if(4 == Button4_4_Scan())//返回
			{
				OLED_Clear();
				return 1;
			}
		}
	}
 }
 

//获取键盘数值
u16 GET_NUM(void)
{
	u8  key_num=0;
	u16 num=0;
		OLED_ShowNum(78,32,num,3,12);
	OLED_Refresh_Gram();//更新显示
	while(1)
	{
		key_num=Button4_4_Scan();	
		if(key_num != -1)
		{
//			if(key_num==13)return 0xFFFF;//‘返回’键
//			if(key_num==14)return 0xFF00;//		
//			if(key_num>0&&key_num<10&&num<99)//‘1-9’键(限制输入3位数)
//				num =num*10+key_num;		
//			if(key_num==15)num =num/10;//‘Del’键			
//			if(key_num==10&&num<99)num =num*10;//‘0’键
//			if(key_num==16)return num;  //‘Enter’键
			
			switch(key_num)
				{
					case 1:
					case 2:
					case 3:
						if(key_num>0&&key_num<10&&num<99)//‘1-9’键(限制输入3位数)
						num =num*10+key_num;		
					break;
					case 4://返回
						return 0xFFFF;
					return -1;
						
					break;
					case 5:
					case 6:
					case 7:
						if(key_num>0&&key_num<10&&num<99)//‘1-9’键(限制输入3位数)
						num =num*10+key_num-1;
					break;
					case 8:num =num/10;//‘del’键
					break;
					case 9:
					case 10:
					case 11:
						if(key_num>0&&key_num<10&&num<99)//‘1-9’键(限制输入3位数)
						num =num*10+key_num-2;
					break;
					case 12: break;//DIS
					case 13:
					case 15:
						return 0xFF00;
					break;
					case 14:num =num*10;
					break;
					case 16:return num;
					break;
				}
		OLED_ShowNum(78,32,num,3,12);
	OLED_Refresh_Gram();//更新显示
		}
	}	
}
//密码锁
int password(void)
{
	int  key_num=0,i=0,satus=0;
	u16 num=0,num2=0,time3=0,time;
	u8 pwd[11]="          ";
	u8 hidepwd[11]="          ";
	u8 buf[64];
	OLED_Clear();//清屏

	if(DisErrCnt())return -1;//错误次数超限
	
	OLED_Clear();//清屏
	Show_Str(5,0,128,16,"密码：",16,0);
	Show_Str(10,16,128,12," 1   2   3  Bck",12,0);
	Show_Str(10,28,128,12," 4   5   6  Del",12,0);
	Show_Str(10,40,128,12," 7   8   9  Dis",12,0);
	Show_Str(10,52,128,12,"Clr  0  Clr  OK",12,0);
	OLED_Refresh_Gram();//更新显示
//	Show_Str(102,36,128,12,"显示",12,0);
//	Show_Str(0,52,128,12,"删除 清空   返回 确认",12,0);
	while(1)
	{
		key_num=Button4_4_Scan();	
		if(key_num != -1)
		{		
			DisFlag = 1;
			time3=0;
			if(key_num != -1)
			{	
				DisFlag = 1;
				time3=0;
				switch(key_num)
				{
					case 1:
					case 2:
					case 3:
						pwd[i]=key_num+0x30; //1-3
						hidepwd[i]='*';
						i++;
					break;
					case 4://返回
						OLED_Clear();
						delay_ms(500);
					return -1;
						
					break;
					case 5:
					case 6:
					case 7:
						pwd[i]=key_num+0x30-1; //4-6
						hidepwd[i]='*';
						i++;
					break;
					case 8:
						if( i > 0){
							pwd[--i]=' ';  //‘del’键
							hidepwd[i]=' '; 
						}
					break;
					case 9:
					case 10:
					case 11:
						pwd[i]=key_num+0x30-2; //4-6
						hidepwd[i]='*';
						i++;
					break;
					case 12:satus=!satus; break;//DIS
					case 13:
					case 15:
						while(i--){
						pwd[i]=' ';  //‘清空’键
						hidepwd[i]=' '; 
						}
						i=0;
					break;
					case 14:
						pwd[i]=0x30; //4-6
						hidepwd[i]='*';
						i++;
					break;
					case 16:
						goto UNLOCK;
					break;
				}
		}
		if(DisFlag == 1)
		{
		if(satus==0)OLED_ShowString(53,0,hidepwd,12);
		else OLED_ShowString(53,0,pwd,12);
		OLED_Refresh_Gram();//更新显示
		}
		
		time3++;
		if(time3%1000==0){
			OLED_Clear();//清屏
			return -1;
		}
	}
}

UNLOCK:	
		for(i=0; i<10; i++){   //验证虚伪密码
			if(pwd[i]==sys.passwd1[num])num++;
				else num=0;
			if(num==6)
				break;
		}
		for(i=0; i<10; i++){   //验证密码
			if(pwd[i]==sys.passwd2[num2])num2++;
				else num2=0;
			if(num2==6)
				break;
		}
		if(num==6 | num2==6){
			DisUnLock();
			OLED_Clear();//清屏
			sys.errCnt = 0;
			return 0;
		}
		else {
			sys.errCnt++;//错误次数计数
			if(sys.errCnt>MAXERRTIMES)
				sys.errTime = 30; //30秒不能再解锁
			OLED_Clear();//清屏
			Show_Str(45,48,128,16,"密码错误！",16,0);
			OLED_Refresh_Gram();//更新显示
			beep_on_mode1();
			delay_ms(1500);
			OLED_Clear();//清屏
			return -1;
		}
	
}


//显示确认码错误信息
void ShowErrMessage(u8 ensure)
{
	Show_Str(0,48,128,12,(u8*)EnsureMessage(ensure),12,0);	
	OLED_Refresh_Gram();//更新显示
	delay_ms(1000);
	OLED_ShowString(0,48,"                   ",12);	
	
	OLED_Refresh_Gram();//更新显示
}
//录指纹
void Add_FR(void)
{
	u8 i,ensure ,processnum=0;
	int key_num;
	u16 ID;
	OLED_Clear();//清屏
	while(1)
	{
		key_num=Button4_4_Scan();	
		if(key_num==16){
			OLED_Clear();//清屏
			return ;
		}
		switch (processnum)
		{
			case 0:
				//OLED_Clear();//清屏
				i++;
				Show_Str(0,0,128,16,"=== 录入指纹 ===",16,0);
				Show_Str(0,24,128,12,"请按指纹！  ",12,0);	
				Show_Str(104,52,128,12,"返回",12,0);		
				OLED_Refresh_Gram();//更新显示	
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					BEEP=1;
					ensure=PS_GenChar(CharBuffer1);//生成特征
					BEEP=0;
					if(ensure==0x00)
					{
						Show_Str(0,24,128,12,"指纹正常！    ",12,0);
						OLED_Refresh_Gram();//更新显示	
						i=0;
						processnum=1;//跳到第二步						
					}else ShowErrMessage(ensure);				
				}else ShowErrMessage(ensure);
				//OLED_Clear();//清屏
				break;
			
			case 1:
				i++;
				Show_Str(0,24,128,12,"请再按一次指纹",12,0);
				OLED_Refresh_Gram();//更新显示		
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					BEEP=1;
					ensure=PS_GenChar(CharBuffer2);//生成特征
					BEEP=0;
					if(ensure==0x00)
					{
						Show_Str(0,24,128,12,"指纹正常！",12,0);	
						OLED_Refresh_Gram();//更新显示
						i=0;
						processnum=2;//跳到第三步
					}else ShowErrMessage(ensure);	
				}else ShowErrMessage(ensure);		
				//OLED_Clear();//清屏
				break;

			case 2:		
				Show_Str(0,24,128,12,"对比两次指纹        ",12,0);
				OLED_Refresh_Gram();//更新显示
				ensure=PS_Match();
				if(ensure==0x00) 
				{
					Show_Str(0,24,128,12,"两次指纹一样       ",12,0);
					OLED_Refresh_Gram();//更新显示
					processnum=3;//跳到第四步
				}
				else 
				{
					Show_Str(0,24,128,12,"对比失败 请重录    ",12,0);	
					OLED_Refresh_Gram();//更新显示
					ShowErrMessage(ensure);
					i=0;
					OLED_Clear();//清屏
					processnum=0;//跳回第一步		
				}
				delay_ms(1200);
				//OLED_Clear();//清屏
				break;

			case 3:
			Show_Str(0,24,128,12,"生成指纹模板...    ",12,0);
			OLED_Refresh_Gram();//更新显示	
				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
//					
					Show_Str(0,24,128,12,"生成指纹模板成功!",12,0);
					OLED_Refresh_Gram();//更新显示
					processnum=4;//跳到第五步
				}else {processnum=0;ShowErrMessage(ensure);}
				delay_ms(1200);
				break;
				
			case 4:	
				//OLED_Clear();//清屏
			Show_Str(0,24,128,12,"请输入储存ID:        ",12,0);
			Show_Str(122,52,128,12," ",12,0);
			Show_Str(0,52,128,12,"删除 清空      确认",12,0);
			OLED_Refresh_Gram();//更新显示
				do
					ID=GET_NUM();
				while(!(ID<AS608Para.PS_max));//输入ID必须小于模块容量最大的数值
				ensure=PS_StoreChar(CharBuffer2,ID);//储存模板
				if(ensure==0x00) 
				{			
          OLED_Clear_NOupdate();//清屏
					Show_Str(0,30,128,16,"录指纹成功!",16,0);	
					PS_ValidTempleteNum(&ValidN);//读库指纹个数
					Show_Str(66,52,128,12,"剩余",12,0);
					OLED_ShowNum(90,52,AS608Para.PS_max-ValidN,3,12);
					OLED_Refresh_Gram();//更新显示
					delay_ms(1500);
					OLED_Clear();	
					return ;
				}else {processnum=0;ShowErrMessage(ensure);}
				OLED_Clear();//清屏					
				break;				
		}
		delay_ms(400);
		if(i==10)//超过5次没有按手指则退出
		{
			OLED_Clear();
			break;
		}				
	}
}

//刷指纹
int press_FR(void)
{
	SearchResult seach;
	u8 ensure;
	char str[256];
	
	
	if(DisErrCnt())return -1;//错误次数超限
	ensure=PS_GetImage();
	
	OLED_Clear_NOupdate();
	Show_Str(0,0,128,16,"正在检测指纹",16,0);
	OLED_Refresh_Gram();//更新显示
	if(ensure==0x00)//获取图像成功 
	{	
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //生成特征成功
		{		
			
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)//搜索成功
			{				
				OLED_Clear_NOupdate();
				Show_Str(20,10,128,24,"解锁中...",24,0);	
				OLED_Refresh_Gram();//更新显示
				Walkmotor_ON();
				Show_Str(20,10,128,24,"已解锁！",24,0);
				OLED_Refresh_Gram();//更新显示
				OLED_Show_Font(112,18,1);//开锁				
				//str=mymalloc(SRAMIN,2000);
				sprintf(str,"ID:%d     匹配分",seach.pageID);
				Show_Str(0,52,128,12,(u8*)str,12,0);	
				sprintf(str,":%d",seach.mathscore);
				Show_Str(96,52,128,12,(u8*)str,12,0);	
				//myfree(SRAMIN,str);
				OLED_Refresh_Gram();//更新显示
				delay_ms(1800);
				OLED_Clear();
				return 0;
			}
			else {
				sys.errCnt++;//错误次数计数
				if(sys.errCnt>MAXERRTIMES)
					sys.errTime = 30; //30秒不能再解锁
				ShowErrMessage(ensure);	
				OLED_Refresh_Gram();//更新显示
				beep_on_mode1();
				OLED_Clear();
				return -1;
			}				
	  }
		else
			ShowErrMessage(ensure);
		
	OLED_Refresh_Gram();//更新显示
	 delay_ms(2000);
		OLED_Clear();
		
	}
	return -1;	
}

//删除指纹
void Del_FR(void)
{
	u8  ensure;
	u16 num;
	OLED_Clear();
	Show_Str(0,0,128,16,"=== 删除指纹 ===",16,0);	
	Show_Str(0,16,128,12,"输入指纹ID：",12,0);
	Show_Str(0,52,128,12,"返回 清空    确认删除",12,0);
	OLED_Refresh_Gram();//更新显示
	delay_ms(50);
//	AS608_load_keyboard(0,170,(u8**)kbd_delFR);
	num=GET_NUM();//获取返回的数值
	if(num==0xFFFF)
		goto MENU ; //返回主页面
	else if(num==0xFF00)
		ensure=PS_Empty();//清空指纹库
	else 
		ensure=PS_DeletChar(num,1);//删除单个指纹
	if(ensure==0)
	{
		OLED_Clear();
		Show_Str(0,20,128,12,"删除指纹成功！",12,0);		
		Show_Str(80,48,128,12,"剩余",12,0);		
	OLED_Refresh_Gram();//更新显示
	}
  else
		ShowErrMessage(ensure);	
	
	OLED_Refresh_Gram();//更新显示
	PS_ValidTempleteNum(&ValidN);//读库指纹个数
	OLED_ShowNum(110,48,AS608Para.PS_max-ValidN,3,12);
	delay_ms(1200);
	
MENU:	
	OLED_Clear();
}
//修改密码
void SetPassworld(void)
{
	int pwd_ch=0;
	int  key_num=0,i=0,satus=0;
	u16 time4=0;
	u8 pwd[6]="      ";
	u8 hidepwd[6]="      ";
	u8 buf[10];
	OLED_Clear();//清屏
	Show_Str(10,16,128,12," 1   2   3  Bck",12,0);
	Show_Str(10,28,128,12," 4   5   6  Del",12,0);
	Show_Str(10,40,128,12," 7   8   9  Dis",12,0);
	Show_Str(10,52,128,12,"Clr  0  Chg  OK",12,0);
	
	
	Show_Str(5,0,128,16,"新密码",16,0);
	sprintf((char*)buf,"%d:",pwd_ch+1);
	Show_Str(5,48,128,16,buf,16,0);
	OLED_Refresh_Gram();//更新显示
	while(1)
	{
		key_num=Button4_4_Scan();	
		if(key_num != -1)
		{	
			DisFlag = 1;
			time4=0;
			switch(key_num)
			{
				case 1:
				case 2:
				case 3:
					pwd[i]=key_num+0x30; //1-3
					hidepwd[i]='*';
					i++;
				break;
				case 4://返回
					OLED_Clear();
					delay_ms(500);
				return ;
					
				break;
				case 5:
				case 6:
				case 7:
					pwd[i]=key_num+0x30-1; //4-6
					hidepwd[i]='*';
					i++;
				break;
				case 8:
					if( i > 0){
						pwd[--i]=' ';  //‘del’键
						hidepwd[i]=' '; 
					}
				break;
				case 9:
				case 10:
				case 11:
					pwd[i]=key_num+0x30-2; //4-6
					hidepwd[i]='*';
					i++;
				break;
				case 12:satus=!satus; break;//DIS
				case 13:
					sprintf((char*)buf,"%d:",pwd_ch+1);
					Show_Str(5,48,128,16,buf,16,0);
					pwd_ch = !pwd_ch;
				case 15:
					while(i--){
					pwd[i]=' ';  //‘清空’键
					hidepwd[i]=' '; 
					}
					i=0;
				break;
				case 14:
					pwd[i]=0x30; //4-6
					hidepwd[i]='*';
					i++;
				break;
				case 16:
					goto MODIF;
				break;
			}
		}
		if(DisFlag == 1)
		if(satus==0)
		{
			OLED_ShowString(70,0,hidepwd,12);
			OLED_Refresh_Gram();//更新显示
		}
		else 
		{
			OLED_ShowString(70,0,pwd,12);
			OLED_Refresh_Gram();//更新显示
		}
		
		time4++;
		if(time4%1000==0){
			OLED_Clear();//清屏
			DisFlag = 1;
			return ;
		}
	}	
	
	
MODIF:
	if(pwd_ch==0)
	{
		memcpy(sys.passwd1,pwd,7);
		STMFLASH_Write(SYS_SAVEADDR,(u16*)&sys,sizeof(sys));//保存到内部FLASH
		
		//STMFLASH_Read(SYS_SAVEADDR,(u16*)&sys,sizeof(sys)); //读取
		printf("pwd=%s",sys.passwd1);
	}
	else
	{		
		memcpy(sys.passwd2,pwd,7);
		STMFLASH_Write(SYS_SAVEADDR,(u16*)&sys,sizeof(sys));//保存密码到内部FLASH
//		STMFLASH_Write(0X08090004,(u32*)pwd,2);//保存密码到内部eeprom
		//STMFLASH_Read(SYS_SAVEADDR,(u16*)&sys,sizeof(sys)); //读取密码2
		printf("pwd2=%s",sys.passwd1);
	}
	OLED_Clear();//清屏
	Show_Str(0,48,128,16,"密码修改成功 ！",16,0);
	OLED_Refresh_Gram();//更新显示
	delay_ms(1000);
}
//设置时间
void Set_Time(void)
{
//	RTC_TimeTypeDef RTC_TimeStruct;
//	RTC_DateTypeDef calendar;
	u16 year;
	u8 mon,dat,wek,hour,min,sec;
	u16 time5=0;
	u8 tbuf[40];
	int key_num;
	int st=0;
	
//	RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);
//	RTC_GetDate(RTC_Format_BIN, &calendar);
	year=calendar.w_year;
	mon=calendar.w_month;
	dat=calendar.w_date;
	wek=calendar.week;
	hour=calendar.hour;
	min=calendar.min;
	sec=calendar.sec;
	OLED_Clear();
	Show_Str(98,38,128,12,"<--",12,0);
	Show_Str(0,52,128,12,"减  加   切换  确定",12,0);
	
	OLED_Refresh_Gram();//更新显示
	while(1)
	{
		time5++;
		key_num=Button4_4_Scan();	
			if(key_num==12 | time5==3000){
				OLED_Clear();//清屏
				return ;
			}
			if(key_num==13){
				switch(st)
				{
					case 0:if(hour>0)hour--;break;
					case 1:if(min>0)min--;break;
					case 2:if(sec>0)sec--;break;
					case 3:if(wek>0)wek--;break;
					case 4:if(year>0)year--;break;
					case 5:if(mon>0)mon--;break;
					case 6:if(dat>0)dat--;break;
				}
			}
			if(key_num==14){
				switch(st)
				{
					case 0:if(hour<23)hour++;break;
					case 1:if(min<59)min++;break;
					case 2:if(sec<59)sec++;break;
					case 3:if(wek<7)wek++;break;
					case 4:if(year<2099)year++;break;
					case 5:if(mon<12)mon++;break;
					case 6:if(dat<31)dat++;break;
				}
			}
			if(key_num==15){
				if(st<7)st++;
				if(st==7)st=0;
			}
			if(key_num==16){
				break;
			}
		if(time5%250==0)
		{
			switch(st)			//闪烁
				{
					case 0:OLED_ShowString(0,0,"  ",24);break;
					case 1:OLED_ShowString(36,0,"  ",24);break;
					case 2:OLED_ShowString(72,0,"  ",24);break;
					case 3:OLED_ShowString(110,12,"   ",12);break;
					case 4:OLED_ShowString(68,26,"    ",12);break;
					case 5:OLED_ShowString(98,26,"  ",12);break;
					case 6:OLED_ShowString(116,26,"  ",12);break;
				}
			OLED_Refresh_Gram();//更新显示
		}
		if(time5%500==0)
		{
			time5=0;
			sprintf((char*)tbuf,"%02d:%02d:%02d",hour,min,sec); 
			OLED_ShowString(0,0,tbuf,24);	
			//RTC_GetDate(RTC_Format_BIN, &calendar);
			sprintf((char*)tbuf,"%04d-%02d-%02d",year,mon,dat); 
			OLED_ShowString(68,26,tbuf,12);		
			sprintf((char*)tbuf,"%s",week[wek]); 
			OLED_ShowString(110,12,tbuf,12);	
			OLED_Refresh_Gram();//更新显示
		}delay_ms(1);
	}
//	RTC_Set_Time(hour,min,sec,RTC_H12_AM);	//设置时间
//	RTC_Set_Date(year,mon,dat,wek);		//设置日期
	RTC_Set(year,mon,dat,hour, min,sec);
	OLED_Clear();
	Show_Str(20,48,128,16,"设置成功！",16,0);
	OLED_Refresh_Gram();//更新显示
	delay_ms(1000);
}

//录入新卡
u8 Add_Rfid(void)
{
	u8 ID;
	u16 time6=0;
	u8 i,key_num,status=1,card_size;
	OLED_Clear();
	Show_Str(0,0,128,16,"=== 录入卡片 ===",16,0);	
	Show_Str(0,20,128,12,"请放入新卡片：",12,0);	
	Show_Str(0,52,128,12,"返回",12,0);
			OLED_Refresh_Gram();//更新显示
	MFRC522_Initializtion();			//初始化MFRC522
	while(1)
	{
		AntennaOn();
		status=MFRC522_Request(0x52, card_pydebuf);			//寻卡
		if(status==0)		//如果读到卡
		{
			printf("rc522 ok\r\n");
			Show_Str(0,38,128,12,"读卡成功！",12,0);
			OLED_Refresh_Gram();//更新显示
			status=MFRC522_Anticoll(card_numberbuf);			//防撞处理			
			card_size=MFRC522_SelectTag(card_numberbuf);	//选卡
			status=MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//验卡
			status=MFRC522_Write(4, card_writebuf);				//写卡（写卡要小心，特别是各区的块3）
			status=MFRC522_Read(4, card_readbuf);					//读卡
			//printf("卡的类型：%#x %#x",card_pydebuf[0],card_pydebuf[1]);
			//卡序列号显，最后一字节为卡的校验码
			printf("卡的序列号：");
			for(i=0;i<5;i++)
			{
				printf("%#x ",card_numberbuf[i]);
			}
			printf("\r\n");
			//卡容量显示，单位为Kbits
			//printf("卡的容量：%dKbits\n",card_size);
			AntennaOff();
			
			OLED_Clear_NOupdate();
			Show_Str(0,12,128,12,"请输入储存ID(0-9):  ",12,0);
			Show_Str(122,52,128,12," ",12,0);
			Show_Str(0,52,128,12,"删除 清空      确认",12,0);
			OLED_Refresh_Gram();//更新显示
				do
					ID=GET_NUM();
				while(!(ID<10));//输入ID必须小于最大容量
			printf("正在录入卡片：%d\r\n",ID);
			OLED_Clear_NOupdate();
			Show_Str(0,38,128,12,"正在录入.",12,0);
			OLED_Refresh_Gram();//更新显示
//			memcpy(sys.cardid[ID],card_numberbuf,5);
			for(i=0;i<5;i++)
			{
				sys.cardid[ID][i] = card_numberbuf[i];
			}
				
			STMFLASH_Write(SYS_SAVEADDR,(u16*)&sys,sizeof(sys));//保存到内部FLASH
//			STMFLASH_Write(0X080f0004,(u32*)card_numberbuf,2);
//			STMFLASH_Read(0X080f0004,(u32*)cardid,1); //读取卡号1
			for(i=0;i<10;i++)
			printf("cardid={%X,%X,%X,%X}\r\n",sys.cardid[i][0],sys.cardid[i][1],sys.cardid[i][2],sys.cardid[i][3]);
			Show_Str(0,38,128,12,"录入成功！",12,0);
			OLED_Refresh_Gram();//更新显示
			delay_ms(1000);
			OLED_Clear();
			return 0;
		}
		key_num=Button4_4_Scan();	
		time6++;
		if(time6%5000==0 | key_num==13)
		{
			OLED_Clear();
			return 1;
		}
	}
}
//rfid卡锁
u8 MFRC522_lock(void)
{
	u8 i,j,status=1,card_size;
	u8 count;
	u8 prtfbuf[64];
	
	AntennaOn();
  status=MFRC522_Request(0x52, card_pydebuf);			//寻卡
	if(status==0)		//如果读到卡
	{
		if(DisErrCnt())return -1;//错误次数超限
		printf("rc522 ok\r\n");
		status=MFRC522_Anticoll(card_numberbuf);			//防撞处理			
		card_size=MFRC522_SelectTag(card_numberbuf);	//选卡
		status=MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//验卡
		status=MFRC522_Write(4, card_writebuf);				//写卡（写卡要小心，特别是各区的块3）
		status=MFRC522_Read(4, card_readbuf);					//读卡
		//MFRC522_Halt();															//使卡进入休眠状态
		//卡类型显示
		
		//printf("卡的类型：%#x %#x",card_pydebuf[0],card_pydebuf[1]);
		

		//卡序列号显，最后一字节为卡的校验码
		
		count=0;
		
		for(j=0;j<10;j++){
		printf("\r\n卡%d 的序列号：",j);
			for(i=0;i<5;i++)
			{
				printf("%x=%x    ",card_numberbuf[i],sys.cardid[j][i]);
				if(card_numberbuf[i]==sys.cardid[j][i])count++;
			}
		printf("\r\n");
			if(count>=4)
			{
				sys.errCnt = 0;
				OLED_Clear_NOupdate();
				sprintf(prtfbuf,"RFID:%d匹配成功",j);
				Show_Str(12,13,128,20,prtfbuf,12,0); 
				OLED_Refresh_Gram();//更新显示
				delay_ms(500);
				DisUnLock();
				return 0;
			}else count=0;
		}
		{
			sys.errCnt++;//错误次数计数
			
			if(sys.errCnt>MAXERRTIMES)
				sys.errTime = 30; //30秒不能再解锁
			OLED_Clear(); 
			Show_Str(12,13,128,20,"卡片错误",12,0); 
			OLED_Refresh_Gram();//更新显示
			beep_on_mode1();
			OLED_Clear(); 
			OLED_Show_Font(56,48,0);//锁
			DisFlag = 1;
		}
		
		printf("\n");
		//卡容量显示，单位为Kbits
		
		//printf("卡的容量：%dKbits\n",card_size);
		
		
		
		//读一个块的数据显示
//		printf("卡数据：\n");
//		for(i=0;i<2;i++)		//分两行显示
//		{
//			for(j=0;j<9;j++)	//每行显示8个
//			{
//				printf("%#x ",card_readbuf[j+i*9]);
//			}
//			printf("\n");
//		}

	}	
	
	AntennaOff();
	return 1;
}
//显示信息
void Massige(void)
{
	OLED_Clear();
	Show_Str(0,0,128,12,"智能能门锁系统",12,0); 
//	Show_Str(0,13,128,12,"作者：化作尘",12,0); 
//	Show_Str(0,26,128,12,"bilibili: 化作尘",12,0);
//	Show_Str(0,39,128,12,"CSDN: 化作尘",12,0); 
	Show_Str(0,52,128,12,"2022-02-25 ",12,0); 
	
	OLED_Refresh_Gram();//更新显示
	delay_ms(3000);
}

 
 //显示时间
void Display_Data(void)
{
	static u8 t=1;	
	u8 tbuf[40];
	if(t!=calendar.sec)
	{
		t=calendar.sec;
		sprintf((char*)tbuf,"%02d:%02d:%02d",calendar.hour,calendar.min,calendar.sec); 
		OLED_ShowString(0,0,tbuf,24);	
		//printf(tbuf);
		sprintf((char*)tbuf,"%04d-%02d-%02d",calendar.w_year,calendar.w_month,calendar.w_date); 
		OLED_ShowString(68,26,tbuf,12);		
		//printf(tbuf);
		sprintf((char*)tbuf,"%s",week[calendar.week]); 
		//printf(tbuf);
		OLED_ShowString(110,12,tbuf,12);
		DisFlag = 1;//更新显示
	}
}
 
 //开机信息
void starting(void)
{
	u8 cnt = 0;
	u8 ensure;
	char str[64];			  
	u8 key;
//	OLED_Show_Image(0);	//image
//	OLED_Refresh_Gram();//更新显示
	Show_Str(16,12,128,16,"智能门锁系统",16,0);
	OLED_Refresh_Gram();//更新显示
	delay_ms(3000);
	
/*********************************开机信息提示***********************************/
	OLED_Clear();
	Show_Str(0,0,128,12,"fingerprint system!",12,0); 
	Show_Str(0,12,128,12,"connect to as608",12,0);
	OLED_Refresh_Gram();//更新显示
	while(PS_HandShake(&AS608Addr))//与AS608模块握手
	{
		cnt++;if(cnt>10)break;
		delay_ms(400);
		Show_Str(0,24,128,12,"connect failed! ",12,0);
	OLED_Refresh_Gram();//更新显示
		delay_ms(800);
		Show_Str(0,24,128,12,"connect to as608  ",12,0);	
		printf("connect to as608..\r\n");
	OLED_Refresh_Gram();//更新显示
	}
	if(cnt>10)Show_Str(0,24,128,12,"connect failed!",12,0);	
	
	OLED_Refresh_Gram();//更新显示
	sprintf(str,"baud:%d  addr:%x",usart2_baund,AS608Addr);
	Show_Str(0,36,128,12,(u8*)str,12,0);
	OLED_Refresh_Gram();//更新显示
	ensure=PS_ValidTempleteNum(&ValidN);//读库指纹个数
	if(ensure!=0x00)
		printf("ERR:010\r\n");
		//ShowErrMessage(ensure);//显示确认码错误信息	
	ensure=PS_ReadSysPara(&AS608Para);  //读参数 
//	if(ensure==0x00)
//	{
		sprintf(str,"capacity:%d  Lv: %d",AS608Para.PS_max-ValidN,AS608Para.PS_level);
		Show_Str(0,48,128,12,(u8*)str,12,0);
		OLED_Refresh_Gram();//更新显示
//	}
//	else
//		ShowErrMessage(ensure);	//显示确认码错误信息	
	
	delay_ms(1000);
//

	OLED_Clear();
}

 
void SysPartInit(void )   //系统参数初始化 
{
		STMFLASH_Read(SYS_SAVEADDR,(u16*)&sys,sizeof(sys)); //读取
		if(sys.HZCFlag != 980706)
		{
			memset(&sys,0,sizeof(sys));
			sys.HZCFlag = 980706;
			strcpy((char *)sys.passwd1, "123456");//密码
			strcpy((char *)sys.passwd2, "980706");//密码
			STMFLASH_Write(SYS_SAVEADDR,(u16*)&sys,sizeof(sys));//保存到内部FLASH
			printf("初始化配置成功\r\n");
		}else
		{
			printf("欢迎使用化作尘智能门锁\r\n");
		}
}
 


 