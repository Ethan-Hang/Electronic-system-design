#include "MFRC522.h"

// ȫ�ֱ�������
u8  irq_regdata;    // �жϼĴ�������
u16 wait_count;     // �ȴ�����
u8  error_regdata;  // ����Ĵ�������
u8  last_bitsdata;  // ���λ����

// ����������΢�뼶��ʱ����
// ���������count -- ��ʱ΢����
void Delay1_us(u16 count)
{
  u16 i;
  
  for(i=0;i<count;i++)
  {
		u8 us=12;
		
		while(us--);
  }
}

// ������������ʼ��STM32��SPI3�ӿڣ�����MFRC522ͨ��
void STM32_SPI3_Init(void) 
{ 
	GPIO_InitTypeDef  GPIO_InitStructure;
	 
	// ʹ��GPIOB��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA, ENABLE);

	// ����GPIOA������1��5��6��7Ϊ�������������SPI3��CS��SCK��MOSI��RST��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // �������ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // �ٶ�Ϊ50MHz
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	// ����GPIOA������4Ϊ�������루����MISO��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;   // ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);	 
	
	MFRC522_CS(1);  // Ƭѡ�����øߵ�ƽ��ȡ��MFRC522��ѡ��״̬
}

// ����������ͨ��SPI3����һ���ֽ�����
// ���������val -- Ҫ���͵��ֽ�����
void SPI3_Send(u8 val)  
{ 
	u8 recv_data = 0, i = 0;
	
	RC522_SCK = 0;  // ʱ�������õ�
	for(i=0; i<8; i++) // �Ӹ�λ����λ��λ����
	{
		// ����MOSI����״̬
		if(val & (1<<(7-i))) // �����ǰλΪ1
		{
			RC522_MOSI = 1; // MOSI�����ø�
		}
		else
		{
			RC522_MOSI = 0; // MOSI�����õ�
		}
		delay_us(5); // ��ʱ5΢�룬ȷ���ź��ȶ�
		
		RC522_SCK = 1;  // ʱ�������øߣ��������ݷ���
		delay_us(5);     // ��ʱ5΢�룬�ȴ����ݴ���
		// �ڸߵ�ƽ�ڼ��ȡMISO��������
		if(RC522_MISO == 1) // ���MISOΪ1
		{
			recv_data |= (1<<(7-i)); // �����յ���λ����recv_data
		}
		
		RC522_SCK = 0; // ʱ�������õͣ����һ��λ����
	}
}

// ����������ͨ��SPI3����һ���ֽ�����
// ����ֵ�����յ����ֽ�����
u8 SPI3_Receive(void)  
{ 
	u8 recv_data = 0, i = 0;
	
	RC522_SCK = 0;  // ʱ�������õ�
	for(i=0; i<8; i++) // �Ӹ�λ����λ��λ����
	{
		RC522_MOSI = 0; // MOSI�����õͣ�����ģʽ����Ҫ�������ݣ�
		
		delay_us(5);    // ��ʱ5΢�룬ȷ���ź��ȶ�
		
		RC522_SCK = 1; // ʱ�������øߣ��������ݽ���
		delay_us(5);    // ��ʱ5΢�룬�ȴ����ݴ���
		// �ڸߵ�ƽ�ڼ��ȡMISO��������
		if(RC522_MISO == 1) // ���MISOΪ1
		{
			recv_data |= (1<<(7-i)); // �����յ���λ����recv_data
		}
		
		RC522_SCK = 0; // ʱ�������õͣ����һ��λ����
	}
	
	return recv_data; // ���ؽ��յ����ֽ�����
}

// ������������MFRC522��ָ���Ĵ���д��һ���ֽ�����
// ���������addr -- �Ĵ�����ַ��val -- Ҫд�������
void Write_MFRC522(u8 addr, u8 val) 
{
	// ��ַ��ʽ��0XXXXXX0�����λΪ0��ʾд������
	MFRC522_CS(0);   // Ƭѡ�����õͣ�ѡ��MFRC522
	SPI3_Send((addr<<1)&0x7E); // ���ͼĴ�����ַ
	SPI3_Send(val);   // ��������
	MFRC522_CS(1);   // Ƭѡ�����øߣ�ȡ��ѡ��
}

// ������������MFRC522��ָ���Ĵ�����ȡһ���ֽ�����
// ���������addr -- �Ĵ�����ַ
// ����ֵ����ȡ�����ֽ�����
u8 Read_MFRC522(u8 addr) 
{  
	u8 val;
	// ��ַ��ʽ��1XXXXXX0�����λΪ1��ʾ��������
	MFRC522_CS(0);   // Ƭѡ�����õͣ�ѡ��MFRC522
	SPI3_Send(((addr<<1)&0x7E)|0x80); // ���ͼĴ�����ַ����������
	val = SPI3_Receive(); // ��������
	MFRC522_CS(1);   // Ƭѡ�����øߣ�ȡ��ѡ��
	return val;       // ���ض�ȡ��������
}

// ��������������MFRC522�Ĵ�����ָ��λ
// ���������reg -- �Ĵ�����ַ��mask -- Ҫ��λ������
void SetBitMask(u8 reg, u8 mask)   
{     
	u8 tmp=0;
	tmp = Read_MFRC522(reg); // ��ȡ��ǰ�Ĵ���ֵ
	Write_MFRC522(reg, tmp | mask); // ��ָ��λ����Ϊ1
}

// �������������MFRC522�Ĵ�����ָ��λ
// ���������reg -- �Ĵ�����ַ��mask -- Ҫ�����λ����
void ClearBitMask(u8 reg, u8 mask)   
{     
	u8 tmp=0;
	tmp = Read_MFRC522(reg); // ��ȡ��ǰ�Ĵ���ֵ
	Write_MFRC522(reg, tmp & (~mask)); // ��ָ��λ���Ϊ0
}

// ��������������MFRC522����
// ע�⣺ÿ��������ر�����֮��������1ms�ļ��
void AntennaOn(void) 
{  
	u8 temp;
	temp = Read_MFRC522(TxControlReg); // ��ȡ���߿��ƼĴ���
	if ((temp & 0x03) == 0) // �������δ����
	{   
		SetBitMask(TxControlReg, 0x03); // ��������
	}
}

// �����������ر�MFRC522����
// ע�⣺ÿ��������ر�����֮��������1ms�ļ��
void AntennaOff(void) 
{  
	ClearBitMask(TxControlReg, 0x03); // �ر�����
}

// ������������λMFRC522ģ��
void MFRC522_Reset(void) 
{ 
	// �ⲿӲ����λ
	MFRC522_Rst(1); // ��λ�����ø�
	Delay1_us(1);   // ��ʱ1΢��
	MFRC522_Rst(0); // ��λ�����õ�
	Delay1_us(1);   // ��ʱ1΢��
	MFRC522_Rst(1); // ��λ�����ø�
	Delay1_us(1);   // ��ʱ1΢��
	// �ڲ���λ
	Write_MFRC522(CommandReg, PCD_RESETPHASE); // ִ�и�λ����
}

// ������������ʼ��MFRC522ģ��
void MFRC522_Initializtion(void) 
{
	STM32_SPI3_Init(); // ��ʼ��SPI3�ӿ�
	MFRC522_Reset();   // ��λMFRC522
	// ���ö�ʱ����TPrescaler*TReloadVal/6.78MHz = 25ms
	Write_MFRC522(TModeReg, 0x8D);     // ���ö�ʱ��ģʽ��TAuto=1����4λΪԤ��Ƶ��4λ
	Write_MFRC522(TPrescalerReg, 0x3E); // Ԥ��Ƶ��8λ
	Write_MFRC522(TReloadRegL, 0x32);   // ��ʱ����8λ
	Write_MFRC522(TReloadRegH, 0x00);   // ��ʱ����8λ
	Write_MFRC522(TxAutoReg, 0x40);     // ����100%ASK����
	Write_MFRC522(ModeReg, 0x3D);       // ����CRC��ʼֵ0x6363
	Write_MFRC522(CommandReg, 0x00);    // ����MFRC522
	AntennaOn();                        // ��������
}

// ����������ͨ��MFRC522��ISO14443������ͨ��
// ���������command -- MFRC522������
//           sendData -- ���͵���������
//           sendLen -- �������ݳ���
//           backData -- ���յ��Ŀ���������
//           backLen -- �������ݵ�λ����
// ����ֵ���ɹ�����MI_OK��ʧ�ܷ��ش�����
u8 MFRC522_ToCard(u8 command, u8 *sendData, u8 sendLen, u8 *backData, u16 *backLen) 
{
	u8  status = MI_ERR;
	u8  irqEn = 0x00;
	u8  waitIRq = 0x00;
	u8  lastBits;
	u8  n;
	u16 i;
	// �������������жϲ���
	switch (command)     
	{         
		case PCD_AUTHENT:   // ��֤������
			irqEn = 0x12;   // ������֤����ж�
			waitIRq = 0x10; // �ȴ���֤����ж�
			break;
		case PCD_TRANSCEIVE: // ����FIFO����
			irqEn = 0x77;   // �����ͽ�������ж�
			waitIRq = 0x30; // �ȴ����ͽ�������ж�
			break;      
		default:    
			break;     
	}
	Write_MFRC522(ComIEnReg, irqEn|0x80); // ʹ���ж�����
	ClearBitMask(ComIrqReg, 0x80);        // ��������жϱ�־
	SetBitMask(FIFOLevelReg, 0x80);       // ���FIFO
	Write_MFRC522(CommandReg, PCD_IDLE);  // ����Ϊ����״̬
	// ��FIFOд�뷢������
	for (i=0; i<sendLen; i++)
		Write_MFRC522(FIFODataReg, sendData[i]);
	// ִ������
	Write_MFRC522(CommandReg, command);
	// ����Ƿ�������������ݷ���
	if (command == PCD_TRANSCEIVE)
		SetBitMask(BitFramingReg, 0x80);  // StartSend=1����ʼ����
	// �ȴ����ݽ������
	i = 2000; // ���ȴ�ʱ��25ms
	do      
	{        
		n = Read_MFRC522(ComIrqReg); // ��ȡ�жϼĴ���
		i--;
	} while ((i!=0) && !(n&0x01) && !(n&waitIRq)); // �ȴ��жϻ�ʱ
	// ֹͣ����
	ClearBitMask(BitFramingReg, 0x80); // StartSend=0
	// ����Ƿ���25ms�����ͨ��
	if (i != 0)     
	{            
		if(!(Read_MFRC522(ErrorReg) & 0x1B)) // �������������ͻ��CRC��Э��
		{            
			if (n & irqEn & 0x01) // ���������ʱ
				status = MI_NOTAGERR; // �޿�����
			if (command == PCD_TRANSCEIVE) // ����Ƿ��ͽ�������
			{                 
				n = Read_MFRC522(FIFOLevelReg); // ��ȡFIFO��������
				lastBits = Read_MFRC522(ControlReg) & 0x07; // ��ȡ�����յ�λ��
				if (lastBits!=0)                         
					*backLen = (n-1)*8 + lastBits; // ���㷵�����ݳ���
				else
					*backLen = n*8; // ���ֽڳ���
				if (n == 0)                         
					n = 1;                        
				if (n > MAX_LEN)         
					n = MAX_LEN; // ������󳤶�
				// ��ȡFIFO�еķ�������
				for (i=0; i<n; i++)                 
					backData[i] = Read_MFRC522(FIFODataReg); 
			}
			status = MI_OK; // ͨ�ųɹ�
		}
		else
			status = MI_ERR; // ͨ�Ŵ���
	}	
	Write_MFRC522(ControlReg, 0x80); // ֹͣ��ʱ��
	Write_MFRC522(CommandReg, PCD_IDLE); // ����Ϊ����״̬
	return status; // ����ͨ��״̬
}

// ����������Ѱ������ȡ��Ƭ����
// ���������reqMode -- Ѱ��ģʽ
//           TagType -- ���صĿ�Ƭ����
//           0x4400 = Mifare_UltraLight
//           0x0400 = Mifare_One(S50)
//           0x0200 = Mifare_One(S70)
//           0x0800 = Mifare_Pro(X)
//           0x4403 = Mifare_DESFire
// ����ֵ���ɹ�����MI_OK
u8 MFRC522_Request(u8 reqMode, u8 *TagType)
{  
	u8  status;    
	u16 backBits; // ���յ�������λ��
	Write_MFRC522(BitFramingReg, 0x07); // ���÷������7λ
	TagType[0] = reqMode; // ����Ѱ������
	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits); 
	if ((status != MI_OK) || (backBits != 0x10)) // ���״̬�ͷ���λ��
	{       
		status = MI_ERR; // Ѱ��ʧ��
	}
	return status; // ����Ѱ�����
}

// ��������������ͻ��⣬��ȡ��Ƭ���к�
// ���������serNum -- ����4�ֽڿ����кţ���5�ֽ�ΪУ���ֽ�
// ����ֵ���ɹ�����MI_OK
u8 MFRC522_Anticoll(u8 *serNum) 
{     
    u8  status;     
    u8  i;     
    u8  serNumCheck=0;     
    u16 unLen;
    //           
    ClearBitMask(Status2Reg, 0x08); // ���Crypto1On��־�����ü���ͨ��
    ClearBitMask(CollReg, 0x80);    // �������ͻ�Ĵ�����־
    Write_MFRC522(BitFramingReg, 0x00); // ���÷��������ֽ�
    serNum[0] = PICC_ANTICOLL1;     
    serNum[1] = 0x20;     
    status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);
    //      
    if (status == MI_OK)
    {   
        // У�鿨���к�   
        for(i=0; i<4; i++)   
            serNumCheck ^= serNum[i];
        if(serNumCheck != serNum[i])        
            status = MI_ERR;
    }
    SetBitMask(CollReg, 0x80); // ���÷���ͻ�Ĵ���
    return status;
}
// ��������������CRCУ��
// ���������pIndata -- �������ݣ�len -- ���ݳ��ȣ�pOutData -- CRC���
void CalulateCRC(u8 *pIndata, u8 len, u8 *pOutData) 
{     
	u16 i;
	u8  n;
	ClearBitMask(DivIrqReg, 0x04); // ���CRC�жϱ�־
	SetBitMask(FIFOLevelReg, 0x80); // ���FIFO
	Write_MFRC522(CommandReg, PCD_IDLE); // ����Ϊ����״̬
	// ��FIFOд������
	for (i=0; i<len; i++)
		Write_MFRC522(FIFODataReg, *(pIndata+i));
	Write_MFRC522(CommandReg, PCD_CALCCRC); // ��ʼCRC����
	// �ȴ�CRC�������
	i = 1000;     
	do      
	{         
		n = Read_MFRC522(DivIrqReg);         
		i--;     
	} while ((i!=0) && !(n&0x04)); // �ȴ�CRC�ж�
	// ��ȡCRC���
	pOutData[0] = Read_MFRC522(CRCResultRegL); // ���ֽ�
	pOutData[1] = Read_MFRC522(CRCResultRegH); // ���ֽ�
	Write_MFRC522(CommandReg, PCD_IDLE); // ����Ϊ����״̬
}

// ����������ѡ��Ƭ����ȡ������
// ���������serNum -- �����к�
// ����ֵ���ɹ����ؿ�������ʧ�ܷ���0
u8 MFRC522_SelectTag(u8 *serNum) 
{     
	u8  i;     
	u8  status;     
	u8  size;     
	u16 recvBits;     
	u8  buffer[9];
	buffer[0] = PICC_ANTICOLL1; // ����ͻ����
	buffer[1] = 0x70;           // ѡ������
	buffer[6] = 0x00;           // У���ֽڳ�ֵ
	for (i=0; i<4; i++)					
	{
		buffer[i+2] = *(serNum+i); // ������к�
		buffer[6] ^= *(serNum+i);  // ����У���ֽ�
	}
	CalulateCRC(buffer, 7, &buffer[7]); // ����CRC
	ClearBitMask(Status2Reg, 0x08);     // ���״̬��־
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
	if ((status == MI_OK) && (recvBits == 0x18)) // ���״̬�ͷ���λ��
		size = buffer[0]; // ��ȡ������
	else    
		size = 0; // ѡ��ʧ��
	return size; // ���ؿ�����
}

// ������������֤��Ƭ����
// ���������authMode -- ��֤ģʽ��0x60=A��Կ��0x61=B��Կ��
//           BlockAddr -- ���ַ
//           Sectorkey -- ��������
//           serNum -- �����кţ�4�ֽڣ�
// ����ֵ���ɹ�����MI_OK
u8 MFRC522_Auth(u8 authMode, u8 BlockAddr, u8 *Sectorkey, u8 *serNum) 
{     
	u8  status;     
	u16 recvBits;     
	u8  i;  
	u8  buff[12];    
	// ������֤���ݣ���֤ģʽ+���ַ+��������+�����к�
	buff[0] = authMode;   // ��֤ģʽ
	buff[1] = BlockAddr;  // ���ַ
	for (i=0; i<6; i++)
		buff[i+2] = *(Sectorkey+i); // ��������
	for (i=0; i<4; i++)
		buff[i+8] = *(serNum+i);    // �����к�
	status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
	if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08))) // �����֤״̬
		status = MI_ERR;
	return status; // ������֤���
}

// ������������ȡָ���������
// ���������blockAddr -- ���ַ��recvData -- �洢��ȡ������
// ����ֵ���ɹ�����MI_OK
u8 MFRC522_Read(u8 blockAddr, u8 *recvData) 
{     
	u8  status;     
	u16 unLen;
	recvData[0] = PICC_READ; // ������
	recvData[1] = blockAddr; // ���ַ
	CalulateCRC(recvData, 2, &recvData[2]); // ����CRC
	status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
	if ((status != MI_OK) || (unLen != 0x90)) // ���״̬�ͷ��س���
		status = MI_ERR;
	return status; // ���ض�ȡ���
}

// ������������ָ����д��16�ֽ�����
// ���������blockAddr -- ���ַ��writeData -- Ҫд���16�ֽ�����
// ����ֵ���ɹ�����MI_OK
u8 MFRC522_Write(u8 blockAddr, u8 *writeData) 
{     
	u8  status;     
	u16 recvBits;     
	u8  i;  
	u8  buff[18];
	buff[0] = PICC_WRITE; // д����
	buff[1] = blockAddr;  // ���ַ
	CalulateCRC(buff, 2, &buff[2]); // ����CRC
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);
	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) // ���д������Ӧ
		status = MI_ERR;
	if (status == MI_OK)     
	{         
		for (i=0; i<16; i++) // ׼��16�ֽ�����
			buff[i] = *(writeData+i);
		CalulateCRC(buff, 16, &buff[16]); // ����CRC
		status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);           
		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) // �������д����Ӧ
			status = MI_ERR;         
	}          
	return status; // ����д����
}

// �������������Ƭ��������״̬
void MFRC522_Halt(void) 
{    
	u16 unLen;     
	u8  buff[4];
	buff[0] = PICC_HALT; // ��������
	buff[1] = 0;         // ����ֽ�
	CalulateCRC(buff, 2, &buff[2]); // ����CRC
	MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen); // ִ����������
}
