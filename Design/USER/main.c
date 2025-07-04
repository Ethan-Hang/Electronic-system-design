	#include "main.h"
	#include "walkmotor.h"
	#include <string.h>

	SysTemPat sys;

	#define MAXERRTIMES 5
	#define usart2_baund 57600 // ����2�����ʣ�����ָ��ģ�鲨���ʸ���

	// Ҫд�뵽STM32 FLASH���ַ�������
	const u8 TEXT_Buffer[] = {0x17, 0x23, 0x6f, 0x60, 0, 0};
	#define TEXT_LENTH sizeof(TEXT_Buffer) // ���鳤��
	#define SIZE TEXT_LENTH / 4 + ((TEXT_LENTH % 4) ? 1 : 0)

	SysPara AS608Para; // ָ��ģ��AS608����
	u16 ValidN;		   // ģ������Чָ�Ƹ���
	//u8 **kbd_tbl;

	void Display_Data(void);		// ��ʾʱ��
	void Add_FR(void);				// ¼ָ��
	void Del_FR(void);				// ɾ��ָ��
	int press_FR(void);				// ˢָ��
	void ShowErrMessage(u8 ensure); // ��ʾȷ���������Ϣ
	int password(void);				// ������
	void SetPassworld(void);		// �޸�����
	void starting(void);			// ����������Ϣ
	u8 MFRC522_lock(void);			// ˢ������
	u8 Add_Rfid(void);				// ¼��
	void Del_Rfid(void);
	void Set_Time(void);
	void Massige(void);
	void SysPartInit(void); // ϵͳ������ʼ��
	int Error; // ������֤��Ϣ

	u8 DisFlag = 1;
	u8 isUnlocked = 0; // ��������״̬��־

	// ���ֵ�ASCII��
	uc8 numberascii[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	// ��ʾ������
	u8 dispnumber5buf[6];
	u8 dispnumber3buf[4];
	u8 dispnumber2buf[3];
	// MFRC522������
	u8 mfrc552pidbuf[18];
	u8 card_pydebuf[2];
	u8 card_numberbuf[5];
	u8 card_key0Abuf[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	u8 card_writebuf[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	u8 card_readbuf[18];
	// SM05-S������
	u8 sm05cmdbuf[15] = {14, 128, 0, 22, 5, 0, 0, 0, 4, 1, 157, 16, 0, 0, 21};
	// extern�������������ⲿ��C�ļ��ﶨ�壬���������ļ���ʹ��
	extern u8 sm05receivebuf[16]; // ���ж�C�ļ��ﶨ��
	extern u8 sm05_OK;			  // ���ж�C�ļ��ﶨ��

	// u8 * week[7]={"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
	u8 *week[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	u8 *setup[7] = {"1��¼��ָ��", "2��ɾ��ָ��", "3���޸�����", "4���޸�ʱ��", "5��¼�뿨Ƭ", "6��ɾ����Ƭ"};

	void DisUnLock(void)
	{
		OLED_Clear();
		Show_Str(20, 10, 128, 24, "������...", 24, 0);
		OLED_Refresh_Gram(); // ������ʾ

		Walkmotor_ON();

		Show_Str(20, 10, 128, 24, "�ѽ�����", 24, 0);
		OLED_Refresh_Gram(); // ������ʾ
		Send_AT_Command_Simple("AT+MQTTPUB=0,\"/sys/k24prW5pSG6/mode002/thing/event/property/post\",\"{\\\"method\\\":\\\"thing.service.property.set\\\"\\,\\\"id\\\":\\\"1649634152\\\"\\,\\\"params\\\":{\\\"LockState\\\":1}\\,\\\"version\\\":\\\"1.0.0\\\"}\",0,0");
		Send_AT_Command_Simple("AT+MQTTPUB=0,\"/sys/k24prW5pSG6/mode002/thing/event/property/post\",\"{\\\"method\\\":\\\"thing.service.property.set\\\"\\,\\\"id\\\":\\\"1649634152\\\"\\,\\\"params\\\":{\\\"LockState\\\":1}\\,\\\"version\\\":\\\"1.0.0\\\"}\",0,0");
		delay_ms(500);
	}

	void DisLock(void)
	{
		OLED_Clear();
		Show_Str(30, 20, 128, 16, "�����У�", 16, 0);
		OLED_Refresh_Gram();			   // ������ʾ
		GPIO_ResetBits(GPIOB, GPIO_Pin_9); // BEEP OFF

		Walkmotor_OFF();
		Show_Str(30, 20, 128, 16, "��������", 16, 0);
		OLED_Show_Font(56, 48, 0); // ��
		OLED_Refresh_Gram();	   // ������ʾ
		Send_AT_Command_Simple("AT+MQTTPUB=0,\"/sys/k24prW5pSG6/mode002/thing/event/property/post\",\"{\\\"method\\\":\\\"thing.service.property.set\\\"\\,\\\"id\\\":\\\"1649634152\\\"\\,\\\"params\\\":{\\\"LockState\\\":0}\\,\\\"version\\\":\\\"1.0.0\\\"}\",0,0");
		delay_ms(500);
	}

int main(void)
{
    u16 set = 0;
    u8 err = 0;
    int key_num;
    int time1;
    int time2;		// ����ʱ��
    char arrow = 0; // ��ͷλ��
    // SysHSI_Init();
    delay_init();									// ��ʱ������ʼ��
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // ����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    uart_init(115200);								// ���ڳ�ʼ��Ϊ115200
    printf("���ڹ�������\r\n");
    Button4_4_Init(); // ��ʼ���밴�����ӵ�Ӳ���ӿ�
    OLED_Init();	  // ��ʾ��ʼ��
    Walkmotor_Init();
    BEEP_Init();			   // ��������ʼ��
    usart2_init(usart2_baund); // ��ʼ��ָ��ģ��
    PS_StaGPIO_Init();
    OLED_Clear();
    AliyunIoT_Connect();

    starting();		  // ������Ϣ  logo
    err = RTC_Init(); // RTC��ʼ��
    if (err)
    {
        OLED_Clear();
        Show_Str(12, 13, 128, 20, "RTC CRY ERR!", 12, 0);
        OLED_Refresh_Gram(); // ������ʾ
        delay_ms(3000);
    }
    SysPartInit(); // ϵͳ������ʼ��
    while (1)
    {
    MAIN:
        OLED_Clear();
        OLED_Show_Font(56, 48, 0); // ��ʾ��ͼ��
        while (!isUnlocked) // ����δ����ʱ���
        {
            time1++;
            Display_Data();

            if (DisFlag == 1)
            {
                DisFlag = 0;
                OLED_Fill(0, 24, 16, 63, 0);
                OLED_Refresh_Gram();
            }

            if ((time1 % 100) == 1)
            {
                time1 = 0;
                MFRC522_Initializtion();
                Error = MFRC522_lock();
                if (Error == 0)
                {
                    isUnlocked = 1;
                    DisUnLock();
                    goto MENU;
                }
                else
                {
                    OLED_Show_Font(56, 48, 0);
                }
            }
            // ָ�ƽ���
            if (PS_Sta)
            {
                while (PS_Sta)
                {
                    Error = press_FR();
                    if (Error == 0)
                    {
                        isUnlocked = 1;
                        goto MENU;
                    }
                    else
                    {
                        OLED_Show_Font(56, 48, 0);
                    }
                }
            }
            // ������
            key_num = Button4_4_Scan();
            if (key_num != -1)
            {
                Error = password();
                if (Error == 0)
                {
                    isUnlocked = 1;
                    DisUnLock();
                    goto MENU;
                }
                else
                {
                    OLED_Show_Font(56, 48, 0);
                }
            }

            delay_ms(1);
        }

    /********************������**************************/

    MENU:
        OLED_Clear();
        isUnlocked = 0; // ����˵�ʱ���ý���״̬
		key_num = -1;
    MENUNOCLR:
        OLED_Fill(0, 0, 20, 48, 0);
        // ��ҳ�˵���ʾ
        if (arrow < 3)
        {
            Show_Str(5, arrow * 16, 128, 16, "->", 16, 0); // ��ʾ��ͷ
            set = 0;
        }
        else
        {
            Show_Str(5, (arrow - 3) * 16, 128, 16, "->", 16, 0);
            set = 3;
        }
        Show_Str(25, 0, 128, 16, setup[set], 16, 0);
        Show_Str(25, 16, 128, 16, setup[set + 1], 16, 0);
        Show_Str(25, 32, 128, 16, setup[set + 2], 16, 0);
        Show_Str(0, 52, 128, 12, "��    ��     ȷ��", 12, 0);
        OLED_Refresh_Gram(); // ������ʾ
        time2 = 0;
        while (1)
        {
            // ��ʱ����
            time2++;
            if (time2 > 100 | key_num == 4)
            {
                OLED_Clear();
                DisLock();
                if (time2 > 100)
                    beep_on_mode2();
                time2 = 0;
                OLED_Clear();
                goto MAIN;
            }
            // ����ѡ��ѡ��
            key_num = Button4_4_Scan();
            if (key_num)
            {
                if (key_num == 13)
                {
                    if (arrow > 0)
                        arrow--;
                    goto MENUNOCLR;
                }
                if (key_num == 15)
                {
                    if (arrow < 5)
                        arrow++;
                    goto MENUNOCLR;
                }
                if (key_num == 16)
                {
                    switch (arrow)
                    {
                    case 0:
                        Add_FR();
                        break; // ¼ָ
                    case 1:
                        Del_FR();
                        break; // ɾָ��
                    case 2:
                        SetPassworld();
                        break; // �޸�����
                    case 3:
                        Set_Time();
                        break; // ����ʱ��
                    case 4:
                        Add_Rfid();
                        break; // ¼�뿨Ƭ
                    case 5:
                        Del_Rfid();
                        break; // ɾ����Ƭ
                    }
                    goto MENU;
                }
            }
            delay_ms(1);
        }
    }
}

	u8 DisErrCnt(void)
	{
		int time = 0;
		u8 buf[64];
		if (sys.errTime > 0) // �����������
		{

			OLED_Clear();

			while (1)
			{
				if (time++ == 10)
				{
					time = 0;

					if (sys.errTime == 0)
					{
						OLED_Clear();
						break;
					}
					Show_Str(0, 16, 128, 16, "��������������", 16, 0);
					sprintf(buf, "��%02d�������", sys.errTime);
					Show_Str(20, 32, 128, 16, buf, 16, 0);
					OLED_Refresh_Gram(); // ������ʾ
				}
				delay_ms(1);
				if (4 == Button4_4_Scan()) // ����
				{
					OLED_Clear();
					return 1;
				}
			}
		}
	}

	// ��ȡ������ֵ
	u16 GET_NUM(void)
	{
		u8 key_num = 0;
		u16 num = 0;
		OLED_ShowNum(78, 32, num, 3, 12);
		OLED_Refresh_Gram(); // ������ʾ
		while (1)
		{
			key_num = Button4_4_Scan();
			if (key_num != -1)
			{

				switch (key_num)
				{
				case 1:
				case 2:
				case 3:
					if (key_num > 0 && key_num < 10 && num < 99) // ��1-9����(��������3λ��)
						num = num * 10 + key_num;
					break;
				case 4: // ����
					return 0xFFFF;
					return -1;

					break;
				case 5:
				case 6:
				case 7:
					if (key_num > 0 && key_num < 10 && num < 99) // ��1-9����(��������3λ��)
						num = num * 10 + key_num - 1;
					break;
				case 8:
					num = num / 10; // ��del����
					break;
				case 9:
				case 10:
				case 11:
					if (key_num > 0 && key_num < 10 && num < 99) // ��1-9����(��������3λ��)
						num = num * 10 + key_num - 2;
					break;
				case 12:
					break; // DIS
				case 13:
				case 15:
					return 0xFF00;
					break;
				case 14:
					num = num * 10;
					break;
				case 16:
					return num;
					break;
				}
				OLED_ShowNum(78, 32, num, 3, 12);
				OLED_Refresh_Gram(); // ������ʾ
			}
		}
	}
	// ������
	int password(void)
	{
		int key_num = 0, i = 0, satus = 0;
		u16 num = 0, num2 = 0, time3 = 0, time;
		u8 pwd[11] = "          ";
		u8 hidepwd[11] = "          ";
		u8 buf[64];
		OLED_Clear(); // ����

		if (DisErrCnt())
			return -1; // �����������

		OLED_Clear(); // ����
		Show_Str(5, 0, 128, 16, "���룺", 16, 0);
		Show_Str(10, 16, 128, 12, " 1   2   3  Bck", 12, 0);
		Show_Str(10, 28, 128, 12, " 4   5   6  Del", 12, 0);
		Show_Str(10, 40, 128, 12, " 7   8   9  Dis", 12, 0);
		Show_Str(10, 52, 128, 12, "Clr  0  Clr  OK", 12, 0);
		OLED_Refresh_Gram(); // ������ʾ

		while (1)
		{
			key_num = Button4_4_Scan();
			if (key_num != -1)
			{
				DisFlag = 1;
				time3 = 0;
				if (key_num != -1)
				{
					DisFlag = 1;
					time3 = 0;
					switch (key_num)
					{
					case 1:
					case 2:
					case 3:
						pwd[i] = key_num + 0x30; // 1-3
						hidepwd[i] = '*';
						i++;
						break;
					case 4: // ����
						OLED_Clear();
						delay_ms(500);
						return -1;

						break;
					case 5:
					case 6:
					case 7:
						pwd[i] = key_num + 0x30 - 1; // 4-6
						hidepwd[i] = '*';
						i++;
						break;
					case 8:
						if (i > 0)
						{
							pwd[--i] = ' '; // ��del����
							hidepwd[i] = ' ';
						}
						break;
					case 9:
					case 10:
					case 11:
						pwd[i] = key_num + 0x30 - 2; // 4-6
						hidepwd[i] = '*';
						i++;
						break;
					case 12:
						satus = !satus;
						break; // DIS
					case 13:
					case 15:
						while (i--)
						{
							pwd[i] = ' '; // ����ա���
							hidepwd[i] = ' ';
						}
						i = 0;
						break;
					case 14:
						pwd[i] = 0x30; // 4-6
						hidepwd[i] = '*';
						i++;
						break;
					case 16:
						goto UNLOCK;
						break;
					}
				}
				if (DisFlag == 1)
				{
					if (satus == 0)
						OLED_ShowString(53, 0, hidepwd, 12);
					else
						OLED_ShowString(53, 0, pwd, 12);
					OLED_Refresh_Gram(); // ������ʾ
				}

				time3++;
				if (time3 % 10 == 0)
				{
					OLED_Clear(); // ����
					return -1;
				}
			}
		}

	UNLOCK:
		for (i = 0; i < 10; i++)
		{ // ��֤��α����
			if (pwd[i] == sys.passwd1[num])
				num++;
			else
				num = 0;
			if (num == 6)
				break;
		}
		for (i = 0; i < 10; i++)
		{ // ��֤����
			if (pwd[i] == sys.passwd2[num2])
				num2++;
			else
				num2 = 0;
			if (num2 == 6)
				break;
		}
		if (num == 6 | num2 == 6)
		{
			OLED_Clear(); // ����
			sys.errCnt = 0;
			return 0;
		}
		else
		{
			sys.errCnt++; // �����������
			if (sys.errCnt > MAXERRTIMES)
				sys.errTime = 30; // 30�벻���ٽ���
			OLED_Clear();		  // ����
			Show_Str(45, 48, 128, 16, "�������", 16, 0);
			OLED_Refresh_Gram(); // ������ʾ
			beep_on_mode1();
			delay_ms(1500);
			OLED_Clear(); // ����
			return -1;
		}
	}

	// ��ʾȷ���������Ϣ
	void ShowErrMessage(u8 ensure)
	{
		Show_Str(0, 48, 128, 12, (u8 *)EnsureMessage(ensure), 12, 0);
		OLED_Refresh_Gram(); // ������ʾ
		delay_ms(1000);
		OLED_ShowString(0, 48, "                   ", 12);

		OLED_Refresh_Gram(); // ������ʾ
	}
	// ¼ָ��
	void Add_FR(void)
	{
		u8 i, ensure, processnum = 0;
		int key_num;
		u16 ID;
		OLED_Clear(); // ����
		while (1)
		{
			key_num = Button4_4_Scan();
			if (key_num == 16)
			{
				OLED_Clear(); // ����
				return;
			}
			switch (processnum)
			{
			case 0:
				// OLED_Clear();//����
				i++;
				Show_Str(0, 0, 128, 16, "=== ¼��ָ�� ===", 16, 0);
				Show_Str(0, 24, 128, 12, "�밴ָ�ƣ�  ", 12, 0);
				Show_Str(104, 52, 128, 12, "����", 12, 0);
				OLED_Refresh_Gram(); // ������ʾ
				ensure = PS_GetImage();
				if (ensure == 0x00)
				{
					BEEP = 0;
					ensure = PS_GenChar(CharBuffer1); // ��������
					BEEP = 1;
					if (ensure == 0x00)
					{
						Show_Str(0, 24, 128, 12, "ָ��������    ", 12, 0);
						OLED_Refresh_Gram(); // ������ʾ
						i = 0;
						processnum = 1; // �����ڶ���
					}
					else
						ShowErrMessage(ensure);
				}
				else
					ShowErrMessage(ensure);
				// OLED_Clear();//����
				break;

			case 1:
				i++;
				Show_Str(0, 24, 128, 12, "���ٰ�һ��ָ��", 12, 0);
				OLED_Refresh_Gram(); // ������ʾ
				ensure = PS_GetImage();
				if (ensure == 0x00)
				{
					BEEP = 0;
					ensure = PS_GenChar(CharBuffer2); // ��������
					BEEP = 1;
					if (ensure == 0x00)
					{
						Show_Str(0, 24, 128, 12, "ָ��������", 12, 0);
						OLED_Refresh_Gram(); // ������ʾ
						i = 0;
						processnum = 2; // ����������
					}
					else
						ShowErrMessage(ensure);
				}
				else
					ShowErrMessage(ensure);
				// OLED_Clear();//����
				break;

			case 2:
				Show_Str(0, 24, 128, 12, "�Ա�����ָ��        ", 12, 0);
				OLED_Refresh_Gram(); // ������ʾ
				ensure = PS_Match();
				if (ensure == 0x00)
				{
					Show_Str(0, 24, 128, 12, "����ָ��һ��       ", 12, 0);
					OLED_Refresh_Gram(); // ������ʾ
					processnum = 3;		 // �������Ĳ�
				}
				else
				{
					Show_Str(0, 24, 128, 12, "�Ա�ʧ�� ����¼    ", 12, 0);
					OLED_Refresh_Gram(); // ������ʾ
					ShowErrMessage(ensure);
					i = 0;
					OLED_Clear();	// ����
					processnum = 0; // ���ص�һ��
				}
				delay_ms(1200);
				// OLED_Clear();//����
				break;

			case 3:
				Show_Str(0, 24, 128, 12, "����ָ��ģ��...    ", 12, 0);
				OLED_Refresh_Gram(); // ������ʾ
				ensure = PS_RegModel();
				if (ensure == 0x00)
				{
					//
					Show_Str(0, 24, 128, 12, "����ָ��ģ��ɹ�!", 12, 0);
					OLED_Refresh_Gram(); // ������ʾ
					processnum = 4;		 // �������岽
				}
				else
				{
					processnum = 0;
					ShowErrMessage(ensure);
				}
				delay_ms(1200);
				break;

			case 4:
				// OLED_Clear();//����
				Show_Str(0, 24, 128, 12, "�����봢��ID:        ", 12, 0);
				Show_Str(122, 52, 128, 12, " ", 12, 0);
				Show_Str(0, 52, 128, 12, "ɾ�� ���      ȷ��", 12, 0);
				OLED_Refresh_Gram(); // ������ʾ
				do
					ID = GET_NUM();
				while (!(ID < AS608Para.PS_max)); // ����ID����С��ģ������������ֵ
				ensure = PS_StoreChar(CharBuffer2, ID); // ����ģ��
				if (ensure == 0x00)
				{
					OLED_Clear_NOupdate(); // ����
					Show_Str(0, 30, 128, 16, "¼ָ�Ƴɹ�!", 16, 0);
					PS_ValidTempleteNum(&ValidN); // ����ָ�Ƹ���
					Show_Str(66, 52, 128, 12, "ʣ��", 12, 0);
					OLED_ShowNum(90, 52, AS608Para.PS_max - ValidN, 3, 12);
					OLED_Refresh_Gram(); // ������ʾ
					delay_ms(1500);
					OLED_Clear();
					return;
				}
				else
				{
					processnum = 0;
					ShowErrMessage(ensure);
				}
				OLED_Clear(); // ����
				break;
			}
			delay_ms(400);
			if (i == 10) // ����5��û�а���ָ���˳�
			{
				OLED_Clear();
				break;
			}
		}
	}

	// ˢָ��
	int press_FR(void)
{
    SearchResult seach;
    u8 ensure;
    char str[256];

    if (DisErrCnt())
        return -1;
    ensure = PS_GetImage();
    printf("PS_GetImage ensure: %x\n", ensure);

    OLED_Clear_NOupdate();
    Show_Str(0, 0, 128, 16, "���ڼ��ָ��", 16, 0);
    OLED_Refresh_Gram();
    if (ensure == 0x00)
    {
        ensure = PS_GenChar(CharBuffer1);
        printf("PS_GenChar ensure: %x\n", ensure);
        if (ensure == 0x00)
        {
            ensure = PS_HighSpeedSearch(CharBuffer1, 0, AS608Para.PS_max, &seach);
            printf("PS_HighSpeedSearch ensure: %x, mathscore: %d\n", ensure, seach.mathscore);
            if (ensure == 0x00 && seach.mathscore >= 40) // ������ֵ�� 40 ����
            {
                OLED_Clear_NOupdate();
                Show_Str(20, 10, 128, 24, "������...", 24, 0);
                OLED_Refresh_Gram();
                Walkmotor_ON();

                Show_Str(20, 10, 128, 24, "�ѽ�����", 24, 0);
                OLED_Refresh_Gram();
                OLED_Show_Font(112, 18, 1);
                sprintf(str, "ID:%d     ƥ���", seach.pageID);
                Show_Str(0, 52, 128, 12, (u8 *)str, 12, 0);
                sprintf(str, ":%d", seach.mathscore);
                Show_Str(96, 52, 128, 12, (u8 *)str, 12, 0);
                OLED_Refresh_Gram();
                delay_ms(1000);
                OLED_Clear();
                return 0;
            }
            else
            {
                sys.errCnt++;
                if (sys.errCnt > MAXERRTIMES)
                    sys.errTime = 30;
                ShowErrMessage(ensure);
                OLED_Refresh_Gram();
                beep_on_mode1();
                OLED_Clear();
                return -1;
            }
        }
        else
            ShowErrMessage(ensure);
    }
    return -1;
}
	// ɾ��ָ��
	void Del_FR(void)
	{
		u8 ensure;
		u16 num;
		OLED_Clear();
		Show_Str(0, 0, 128, 16, "=== ɾ��ָ�� ===", 16, 0);
		Show_Str(0, 16, 128, 12, "����ָ��ID��", 12, 0);
		Show_Str(0, 52, 128, 12, "���� ���    ȷ��ɾ��", 12, 0);
		OLED_Refresh_Gram(); // ������ʾ
		delay_ms(50);

		num = GET_NUM(); // ��ȡ���ص���ֵ
		if (num == 0xFFFF)
			goto MENU; // ������ҳ��
		else if (num == 0xFF00)
			ensure = PS_Empty(); // ���ָ�ƿ�
		else
			ensure = PS_DeletChar(num, 1); // ɾ������ָ��
		if (ensure == 0)
		{
			OLED_Clear();
			Show_Str(0, 20, 128, 12, "ɾ��ָ�Ƴɹ���", 12, 0);
			Show_Str(80, 48, 128, 12, "ʣ��", 12, 0);
			OLED_Refresh_Gram(); // ������ʾ
		}
		else
			ShowErrMessage(ensure);

		OLED_Refresh_Gram();		  // ������ʾ
		PS_ValidTempleteNum(&ValidN); // ����ָ�Ƹ���
		OLED_ShowNum(110, 48, AS608Para.PS_max - ValidN, 3, 12);
		delay_ms(1200);

	MENU:
		OLED_Clear();
	}
	// �޸�����
	void SetPassworld(void)
	{
		int pwd_ch = 0;
		int key_num = 0, i = 0, satus = 0;
		u16 time4 = 0;
		u8 pwd[6] = "      ";
		u8 hidepwd[6] = "      ";
		u8 buf[10];
		OLED_Clear(); // ����
		Show_Str(10, 16, 128, 12, " 1   2   3  Bck", 12, 0);
		Show_Str(10, 28, 128, 12, " 4   5   6  Del", 12, 0);
		Show_Str(10, 40, 128, 12, " 7   8   9  Dis", 12, 0);
		Show_Str(10, 52, 128, 12, "Clr  0  Chg  OK", 12, 0);

		Show_Str(5, 0, 128, 16, "������", 16, 0);
		sprintf((char *)buf, "%d:", pwd_ch + 1);
		Show_Str(5, 48, 128, 16, buf, 16, 0);
		OLED_Refresh_Gram(); // ������ʾ
		while (1)
		{
			key_num = Button4_4_Scan();
			if (key_num != -1)
			{
				DisFlag = 1;
				time4 = 0;
				switch (key_num)
				{
				case 1:
				case 2:
				case 3:
					pwd[i] = key_num + 0x30; // 1-3
					hidepwd[i] = '*';
					i++;
					break;
				case 4: // ����
					OLED_Clear();
					delay_ms(500);
					return;

					break;
				case 5:
				case 6:
				case 7:
					pwd[i] = key_num + 0x30 - 1; // 4-6
					hidepwd[i] = '*';
					i++;
					break;
				case 8:
					if (i > 0)
					{
						pwd[--i] = ' '; // ��del����
						hidepwd[i] = ' ';
					}
					break;
				case 9:
				case 10:
				case 11:
					pwd[i] = key_num + 0x30 - 2; // 4-6
					hidepwd[i] = '*';
					i++;
					break;
				case 12:
					satus = !satus;
					break; // DIS
				case 13:
					sprintf((char *)buf, "%d:", pwd_ch + 1);
					Show_Str(5, 48, 128, 16, buf, 16, 0);
					pwd_ch = !pwd_ch;
				case 15:
					while (i--)
					{
						pwd[i] = ' '; // ����ա���
						hidepwd[i] = ' ';
					}
					i = 0;
					break;
				case 14:
					pwd[i] = 0x30; // 4-6
					hidepwd[i] = '*';
					i++;
					break;
				case 16:
					goto MODIF;
					break;
				}
			}
			if (DisFlag == 1)
				if (satus == 0)
				{
					OLED_ShowString(70, 0, hidepwd, 12);
					OLED_Refresh_Gram(); // ������ʾ
				}
				else
				{
					OLED_ShowString(70, 0, pwd, 12);
					OLED_Refresh_Gram(); // ������ʾ
				}

			// time4++;
			// if (time4 % 10 == 0)
			// {
			// 	OLED_Clear(); // ����
			// 	DisFlag = 1;
			// 	return;
			// }
		}

	MODIF:
		if (pwd_ch == 0)
		{
			memcpy(sys.passwd1, pwd, 7);
			STMFLASH_Write(SYS_SAVEADDR, (u16 *)&sys, sizeof(sys)); // ���浽�ڲ�FLASH

			printf("pwd=%s", sys.passwd1);
		}
		else
		{
			memcpy(sys.passwd2, pwd, 7);
			STMFLASH_Write(SYS_SAVEADDR, (u16 *)&sys, sizeof(sys)); // �������뵽�ڲ�FLASH

			printf("pwd2=%s", sys.passwd1);
		}
		OLED_Clear(); // ����
		Show_Str(0, 48, 128, 16, "�����޸ĳɹ� ��", 16, 0);
		OLED_Refresh_Gram(); // ������ʾ
		delay_ms(1000);
	}
	// ����ʱ��
	void Set_Time(void)
	{

		u16 year;
		u8 mon, dat, wek, hour, min, sec;
		u16 time5 = 0;
		u8 tbuf[40];
		int key_num;
		int st = 0;

		year = calendar.w_year;
		mon = calendar.w_month;
		dat = calendar.w_date;
		wek = calendar.week;
		hour = calendar.hour;
		min = calendar.min;
		sec = calendar.sec;
		OLED_Clear();
		Show_Str(98, 38, 128, 12, "<--", 12, 0);
		Show_Str(0, 52, 128, 12, "��  ��   �л�  ȷ��", 12, 0);

		OLED_Refresh_Gram(); // ������ʾ
		while (1)
		{
			time5++;
			key_num = Button4_4_Scan();
			if (key_num == 12 | time5 == 30)
			{
				OLED_Clear(); // ����
				return;
			}
			if (key_num == 13)
			{
				switch (st)
				{
				case 0:
					if (hour > 0)
						hour--;
					break;
				case 1:
					if (min > 0)
						min--;
					break;
				case 2:
					if (sec > 0)
						sec--;
					break;
				case 3:
					if (wek > 0)
						wek--;
					break;
				case 4:
					if (year > 0)
						year--;
					break;
				case 5:
					if (mon > 0)
						mon--;
					break;
				case 6:
					if (dat > 0)
						dat--;
					break;
				}
			}
			if (key_num == 14)
			{
				switch (st)
				{
				case 0:
					if (hour < 23)
						hour++;
					break;
				case 1:
					if (min < 59)
						min++;
					break;
				case 2:
					if (sec < 59)
						sec++;
					break;
				case 3:
					if (wek < 7)
						wek++;
					break;
				case 4:
					if (year < 2099)
						year++;
					break;
				case 5:
					if (mon < 12)
						mon++;
					break;
				case 6:
					if (dat < 31)
						dat++;
					break;
				}
			}
			if (key_num == 15)
			{
				if (st < 7)
					st++;
				if (st == 7)
					st = 0;
			}
			if (key_num == 16)
			{
				break;
			}
			if (time5 % 5 == 0)
			{
				switch (st) // ��˸
				{
				case 0:
					OLED_ShowString(0, 0, "  ", 24);
					break;
				case 1:
					OLED_ShowString(36, 0, "  ", 24);
					break;
				case 2:
					OLED_ShowString(72, 0, "  ", 24);
					break;
				case 3:
					OLED_ShowString(110, 12, "   ", 12);
					break;
				case 4:
					OLED_ShowString(68, 26, "    ", 12);
					break;
				case 5:
					OLED_ShowString(98, 26, "  ", 12);
					break;
				case 6:
					OLED_ShowString(116, 26, "  ", 12);
					break;
				}
				OLED_Refresh_Gram(); // ������ʾ
			}
			if (time5 % 5 == 0)
			{
				time5 = 0;
				sprintf((char *)tbuf, "%02d:%02d:%02d", hour, min, sec);
				OLED_ShowString(0, 0, tbuf, 24);

				sprintf((char *)tbuf, "%04d-%02d-%02d", year, mon, dat);
				OLED_ShowString(68, 26, tbuf, 12);
				sprintf((char *)tbuf, "%s", week[wek]);
				OLED_ShowString(110, 12, tbuf, 12);
				OLED_Refresh_Gram(); // ������ʾ
			}
			delay_ms(1);
		}

		RTC_Set(year, mon, dat, hour, min, sec);
		OLED_Clear();
		Show_Str(20, 48, 128, 16, "���óɹ���", 16, 0);
		OLED_Refresh_Gram(); // ������ʾ
		delay_ms(1000);
	}

	// ¼���¿�
	u8 Add_Rfid(void)
	{
		u8 ID;
		u16 time6 = 0;
		u8 i, key_num, status = 1, card_size;
		OLED_Clear();
		Show_Str(0, 0, 128, 16, "=== ¼�뿨Ƭ ===", 16, 0);
		Show_Str(0, 20, 128, 12, "������¿�Ƭ��", 12, 0);
		Show_Str(0, 52, 128, 12, "����", 12, 0);
		OLED_Refresh_Gram();	 // ������ʾ
		MFRC522_Initializtion(); // ��ʼ��MFRC522
		while (1)
		{
			AntennaOn();
			status = MFRC522_Request(0x52, card_pydebuf); // Ѱ��
			if (status == 0)							  // ���������
			{
				printf("rc522 ok\r\n");
				Show_Str(0, 38, 128, 12, "�����ɹ���", 12, 0);
				OLED_Refresh_Gram();										   // ������ʾ
				status = MFRC522_Anticoll(card_numberbuf);					   // ��ײ����
				card_size = MFRC522_SelectTag(card_numberbuf);				   // ѡ��
				status = MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf); // �鿨
				status = MFRC522_Write(4, card_writebuf);					   // д����д��ҪС�ģ��ر��Ǹ����Ŀ�3��
				status = MFRC522_Read(4, card_readbuf);						   // ����

				printf("�������кţ�");
				for (i = 0; i < 9; i++)
				{
					printf("%#x ", card_numberbuf[i]);
				}
				printf("\r\n");

				AntennaOff();

				OLED_Clear_NOupdate();
				Show_Str(0, 12, 128, 12, "�����봢��ID(0-9):  ", 12, 0);
				Show_Str(122, 52, 128, 12, " ", 12, 0);
				Show_Str(0, 52, 128, 12, "ɾ�� ���      ȷ��", 12, 0);
				OLED_Refresh_Gram(); // ������ʾ
				do
					ID = GET_NUM();
				while (!(ID < 10)); // ����ID����С���������
				printf("����¼�뿨Ƭ��%d\r\n", ID);
				OLED_Clear_NOupdate();
				Show_Str(0, 38, 128, 12, "����¼��.", 12, 0);
				OLED_Refresh_Gram(); // ������ʾ

				for (i = 0; i < 5; i++)
				{
					sys.cardid[ID][i] = card_numberbuf[i];
				}

				STMFLASH_Write(SYS_SAVEADDR, (u16 *)&sys, sizeof(sys)); // ���浽�ڲ�FLASH

				for (i = 0; i < 10; i++)
					printf("cardid={%X,%X,%X,%X}\r\n", sys.cardid[i][0], sys.cardid[i][1], sys.cardid[i][2], sys.cardid[i][3]);
				Show_Str(0, 38, 128, 12, "¼��ɹ���", 12, 0);
				OLED_Refresh_Gram(); // ������ʾ
				delay_ms(1000);
				OLED_Clear();
				return 0;
			}
			key_num = Button4_4_Scan();
			time6++;
			if (time6 % 50 == 0 | key_num == 13)
			{
				OLED_Clear();
				return 1;
			}
		}
	}
	// rfid����
	u8 MFRC522_lock(void)
	{
		u8 i, j, status = 1, card_size;
		u8 count;
		u8 prtfbuf[64];

		AntennaOn();
		status = MFRC522_Request(0x52, card_pydebuf); // Ѱ��
		if (status == 0)							  // ���������
		{
			if (DisErrCnt())
				return -1; // �����������
			printf("rc522 ok\r\n");
			status = MFRC522_Anticoll(card_numberbuf);					   // ��ײ����
			card_size = MFRC522_SelectTag(card_numberbuf);				   // ѡ��
			status = MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf); // �鿨
			status = MFRC522_Write(4, card_writebuf);					   // д����д��ҪС�ģ��ر��Ǹ����Ŀ�3��
			status = MFRC522_Read(4, card_readbuf);						   // ����

			count = 0;

			for (j = 0; j < 10; j++)
			{
				printf("\r\n��%d �����кţ�", j);
				for (i = 0; i < 5; i++)
				{
					printf("%x=%x    ", card_numberbuf[i], sys.cardid[j][i]);
					if (card_numberbuf[i] == sys.cardid[j][i])
						count++;
				}
				printf("\r\n");
				if (count >= 4)
				{
					sys.errCnt = 0;
					OLED_Clear_NOupdate();
					sprintf(prtfbuf, "RFID:%dƥ��ɹ�", j);
					Show_Str(12, 13, 128, 20, prtfbuf, 12, 0);
					OLED_Refresh_Gram(); // ������ʾ
					delay_ms(500);
					return 0;
				}
				else
					count = 0;
			}
			{
				sys.errCnt++; // �����������

				if (sys.errCnt > MAXERRTIMES)
					sys.errTime = 30; // 30�벻���ٽ���
				OLED_Clear();
				Show_Str(12, 13, 128, 20, "��Ƭ����", 12, 0);
				OLED_Refresh_Gram(); // ������ʾ
				beep_on_mode1();
				OLED_Clear();
				OLED_Show_Font(56, 48, 0); // ��
				DisFlag = 1;
			}

			printf("\n");
		}

		AntennaOff();
		return 1;
	}
void Del_Rfid(void)
{
    u16 ID;
    OLED_Clear();
    Show_Str(0, 0, 128, 16, "=== ɾ����Ƭ ===", 16, 0);
    Show_Str(0, 16, 128, 12, "���뿨ƬID (0-9):", 12, 0);
    Show_Str(0, 52, 128, 12, "���� ���  ȷ��ɾ��", 12, 0);
    OLED_Refresh_Gram();
    delay_ms(50);

    ID = GET_NUM();
    if (ID == 0xFFFF) // ����
        return;
    else if (ID < 10) // ��Ч ID
    {
        memset(sys.cardid[ID], 0, 5); // ��ո� ID �Ŀ�Ƭ��¼
        STMFLASH_Write(SYS_SAVEADDR, (u16 *)&sys, sizeof(sys)); // ���浽 Flash
        OLED_Clear();
        Show_Str(0, 20, 128, 12, "ɾ����Ƭ�ɹ���", 12, 0);
        OLED_Refresh_Gram();
        delay_ms(1200);
    }
    OLED_Clear();
}
	// ��ʾ��Ϣ
	void Massige(void)
	{
//		OLED_Clear();
//		Show_Str(0, 0, 128, 12, "����������ϵͳ", 12, 0);

//		Show_Str(0, 52, 128, 12, "2025-5-16 ", 12, 0);

//		OLED_Refresh_Gram(); // ������ʾ
//		delay_ms(3000);
	}

	// ��ʾʱ�䣨�Ż��棩
	void Display_Data(void)
	{
		static u8 last_sec = 0xFF; // ��ʼ��Ϊ�����ܵ�ֵ
		u8 tbuf[40];

		// ����Ƿ���Ҫ������ʾ����仯��ǿ��ˢ�£�
		if ((last_sec != calendar.sec) || (DisFlag == 1))
		{
			last_sec = calendar.sec;

			// ��ʽ������ʾʱ�� HH:MM:SS
			sprintf((char *)tbuf, "%02d:%02d:%02d", calendar.hour, calendar.min, calendar.sec);
			OLED_ShowString(0, 0, tbuf, 24);

			// ��ʽ������ʾ���� YYYY-MM-DD
			sprintf((char *)tbuf, "%04d-%02d-%02d", calendar.w_year, calendar.w_month, calendar.w_date);
			OLED_ShowString(68, 26, tbuf, 12);

			// ��ʾ����
			sprintf((char *)tbuf, "%s", week[calendar.week]);
			OLED_ShowString(110, 12, tbuf, 12);

			// ������ʾ
			OLED_Refresh_Gram();
			DisFlag = 0; // ���ˢ�±�־
		}
	}

	// ������Ϣ
	void starting(void)
	{
		u8 cnt = 0;
		u8 ensure;
		char str[64];
		u8 key;

		Show_Str(16, 12, 128, 16, "����������ϵͳ", 16, 0);
		OLED_Refresh_Gram(); // ������ʾ

		/*********************************������Ϣ��ʾ***********************************/

		while (PS_HandShake(&AS608Addr)) // ��AS608ģ������
		{
			cnt++;
			if (cnt > 3)
				break;
			delay_ms(100);
		}

		ensure = PS_ValidTempleteNum(&ValidN); // ����ָ�Ƹ���
		if (ensure != 0x00)
			printf("ERR:010\r\n");

		ensure = PS_ReadSysPara(&AS608Para); // ������
	}

	void SysPartInit(void) // ϵͳ������ʼ��
	{
		STMFLASH_Read(SYS_SAVEADDR, (u16 *)&sys, sizeof(sys)); // ��ȡ
		if (sys.HZCFlag != 112233)
		{
			memset(&sys, 0, sizeof(sys));
			sys.HZCFlag = 112233;
			strcpy((char *)sys.passwd1, "123456");					// ����
			strcpy((char *)sys.passwd2, "112233");					// ����
			STMFLASH_Write(SYS_SAVEADDR, (u16 *)&sys, sizeof(sys)); // ���浽�ڲ�FLASH
			printf("��ʼ�����óɹ�\r\n");
		}
		else
		{
			printf("��ӭʹ������������\r\n");
		}
	}
