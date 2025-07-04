#include "MFRC522.h"

// 全局变量定义
u8  irq_regdata;    // 中断寄存器数据
u16 wait_count;     // 等待计数
u8  error_regdata;  // 错误寄存器数据
u8  last_bitsdata;  // 最后位数据

// 功能描述：微秒级延时函数
// 输入参数：count -- 延时微秒数
void Delay1_us(u16 count)
{
  u16 i;
  
  for(i=0;i<count;i++)
  {
		u8 us=12;
		
		while(us--);
  }
}

// 功能描述：初始化STM32的SPI3接口，用于MFRC522通信
void STM32_SPI3_Init(void) 
{ 
	GPIO_InitTypeDef  GPIO_InitStructure;
	 
	// 使能GPIOB和GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA, ENABLE);

	// 配置GPIOA的引脚1、5、6、7为推挽输出（用于SPI3的CS、SCK、MOSI、RST）
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // 推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 速度为50MHz
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	// 配置GPIOA的引脚4为下拉输入（用于MISO）
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;   // 下拉输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);	 
	
	MFRC522_CS(1);  // 片选引脚置高电平，取消MFRC522的选中状态
}

// 功能描述：通过SPI3发送一个字节数据
// 输入参数：val -- 要发送的字节数据
void SPI3_Send(u8 val)  
{ 
	u8 recv_data = 0, i = 0;
	
	RC522_SCK = 0;  // 时钟引脚置低
	for(i=0; i<8; i++) // 从高位到低位逐位发送
	{
		// 设置MOSI引脚状态
		if(val & (1<<(7-i))) // 如果当前位为1
		{
			RC522_MOSI = 1; // MOSI引脚置高
		}
		else
		{
			RC522_MOSI = 0; // MOSI引脚置低
		}
		delay_us(5); // 延时5微秒，确保信号稳定
		
		RC522_SCK = 1;  // 时钟引脚置高，触发数据发送
		delay_us(5);     // 延时5微秒，等待数据传输
		// 在高电平期间读取MISO引脚数据
		if(RC522_MISO == 1) // 如果MISO为1
		{
			recv_data |= (1<<(7-i)); // 将接收到的位存入recv_data
		}
		
		RC522_SCK = 0; // 时钟引脚置低，完成一个位传输
	}
}

// 功能描述：通过SPI3接收一个字节数据
// 返回值：接收到的字节数据
u8 SPI3_Receive(void)  
{ 
	u8 recv_data = 0, i = 0;
	
	RC522_SCK = 0;  // 时钟引脚置低
	for(i=0; i<8; i++) // 从高位到低位逐位接收
	{
		RC522_MOSI = 0; // MOSI引脚置低（接收模式不需要发送数据）
		
		delay_us(5);    // 延时5微秒，确保信号稳定
		
		RC522_SCK = 1; // 时钟引脚置高，触发数据接收
		delay_us(5);    // 延时5微秒，等待数据传输
		// 在高电平期间读取MISO引脚数据
		if(RC522_MISO == 1) // 如果MISO为1
		{
			recv_data |= (1<<(7-i)); // 将接收到的位存入recv_data
		}
		
		RC522_SCK = 0; // 时钟引脚置低，完成一个位传输
	}
	
	return recv_data; // 返回接收到的字节数据
}

// 功能描述：向MFRC522的指定寄存器写入一个字节数据
// 输入参数：addr -- 寄存器地址；val -- 要写入的数据
void Write_MFRC522(u8 addr, u8 val) 
{
	// 地址格式：0XXXXXX0（最低位为0表示写操作）
	MFRC522_CS(0);   // 片选引脚置低，选中MFRC522
	SPI3_Send((addr<<1)&0x7E); // 发送寄存器地址
	SPI3_Send(val);   // 发送数据
	MFRC522_CS(1);   // 片选引脚置高，取消选中
}

// 功能描述：从MFRC522的指定寄存器读取一个字节数据
// 输入参数：addr -- 寄存器地址
// 返回值：读取到的字节数据
u8 Read_MFRC522(u8 addr) 
{  
	u8 val;
	// 地址格式：1XXXXXX0（最高位为1表示读操作）
	MFRC522_CS(0);   // 片选引脚置低，选中MFRC522
	SPI3_Send(((addr<<1)&0x7E)|0x80); // 发送寄存器地址（读操作）
	val = SPI3_Receive(); // 接收数据
	MFRC522_CS(1);   // 片选引脚置高，取消选中
	return val;       // 返回读取到的数据
}

// 功能描述：设置MFRC522寄存器的指定位
// 输入参数：reg -- 寄存器地址；mask -- 要置位的掩码
void SetBitMask(u8 reg, u8 mask)   
{     
	u8 tmp=0;
	tmp = Read_MFRC522(reg); // 读取当前寄存器值
	Write_MFRC522(reg, tmp | mask); // 将指定位设置为1
}

// 功能描述：清除MFRC522寄存器的指定位
// 输入参数：reg -- 寄存器地址；mask -- 要清除的位掩码
void ClearBitMask(u8 reg, u8 mask)   
{     
	u8 tmp=0;
	tmp = Read_MFRC522(reg); // 读取当前寄存器值
	Write_MFRC522(reg, tmp & (~mask)); // 将指定位清除为0
}

// 功能描述：开启MFRC522天线
// 注意：每次启动或关闭天线之间需至少1ms的间隔
void AntennaOn(void) 
{  
	u8 temp;
	temp = Read_MFRC522(TxControlReg); // 读取天线控制寄存器
	if ((temp & 0x03) == 0) // 如果天线未开启
	{   
		SetBitMask(TxControlReg, 0x03); // 开启天线
	}
}

// 功能描述：关闭MFRC522天线
// 注意：每次启动或关闭天线之间需至少1ms的间隔
void AntennaOff(void) 
{  
	ClearBitMask(TxControlReg, 0x03); // 关闭天线
}

// 功能描述：复位MFRC522模块
void MFRC522_Reset(void) 
{ 
	// 外部硬件复位
	MFRC522_Rst(1); // 复位引脚置高
	Delay1_us(1);   // 延时1微秒
	MFRC522_Rst(0); // 复位引脚置低
	Delay1_us(1);   // 延时1微秒
	MFRC522_Rst(1); // 复位引脚置高
	Delay1_us(1);   // 延时1微秒
	// 内部软复位
	Write_MFRC522(CommandReg, PCD_RESETPHASE); // 执行复位命令
}

// 功能描述：初始化MFRC522模块
void MFRC522_Initializtion(void) 
{
	STM32_SPI3_Init(); // 初始化SPI3接口
	MFRC522_Reset();   // 复位MFRC522
	// 配置定时器：TPrescaler*TReloadVal/6.78MHz = 25ms
	Write_MFRC522(TModeReg, 0x8D);     // 设置定时器模式，TAuto=1，低4位为预分频高4位
	Write_MFRC522(TPrescalerReg, 0x3E); // 预分频低8位
	Write_MFRC522(TReloadRegL, 0x32);   // 定时器低8位
	Write_MFRC522(TReloadRegH, 0x00);   // 定时器高8位
	Write_MFRC522(TxAutoReg, 0x40);     // 设置100%ASK调制
	Write_MFRC522(ModeReg, 0x3D);       // 设置CRC初始值0x6363
	Write_MFRC522(CommandReg, 0x00);    // 启动MFRC522
	AntennaOn();                        // 开启天线
}

// 功能描述：通过MFRC522与ISO14443卡进行通信
// 输入参数：command -- MFRC522命令字
//           sendData -- 发送到卡的数据
//           sendLen -- 发送数据长度
//           backData -- 接收到的卡返回数据
//           backLen -- 返回数据的位长度
// 返回值：成功返回MI_OK，失败返回错误码
u8 MFRC522_ToCard(u8 command, u8 *sendData, u8 sendLen, u8 *backData, u16 *backLen) 
{
	u8  status = MI_ERR;
	u8  irqEn = 0x00;
	u8  waitIRq = 0x00;
	u8  lastBits;
	u8  n;
	u16 i;
	// 根据命令设置中断参数
	switch (command)     
	{         
		case PCD_AUTHENT:   // 认证卡密码
			irqEn = 0x12;   // 允许认证相关中断
			waitIRq = 0x10; // 等待认证完成中断
			break;
		case PCD_TRANSCEIVE: // 发送FIFO数据
			irqEn = 0x77;   // 允许发送接收相关中断
			waitIRq = 0x30; // 等待发送接收完成中断
			break;      
		default:    
			break;     
	}
	Write_MFRC522(ComIEnReg, irqEn|0x80); // 使能中断请求
	ClearBitMask(ComIrqReg, 0x80);        // 清除所有中断标志
	SetBitMask(FIFOLevelReg, 0x80);       // 清空FIFO
	Write_MFRC522(CommandReg, PCD_IDLE);  // 设置为空闲状态
	// 向FIFO写入发送数据
	for (i=0; i<sendLen; i++)
		Write_MFRC522(FIFODataReg, sendData[i]);
	// 执行命令
	Write_MFRC522(CommandReg, command);
	// 如果是发送命令，启动数据发送
	if (command == PCD_TRANSCEIVE)
		SetBitMask(BitFramingReg, 0x80);  // StartSend=1，开始发送
	// 等待数据接收完成
	i = 2000; // 最大等待时间25ms
	do      
	{        
		n = Read_MFRC522(ComIrqReg); // 读取中断寄存器
		i--;
	} while ((i!=0) && !(n&0x01) && !(n&waitIRq)); // 等待中断或超时
	// 停止发送
	ClearBitMask(BitFramingReg, 0x80); // StartSend=0
	// 检查是否在25ms内完成通信
	if (i != 0)     
	{            
		if(!(Read_MFRC522(ErrorReg) & 0x1B)) // 检查错误：溢出、冲突、CRC、协议
		{            
			if (n & irqEn & 0x01) // 如果发生超时
				status = MI_NOTAGERR; // 无卡错误
			if (command == PCD_TRANSCEIVE) // 如果是发送接收命令
			{                 
				n = Read_MFRC522(FIFOLevelReg); // 读取FIFO中数据量
				lastBits = Read_MFRC522(ControlReg) & 0x07; // 读取最后接收的位数
				if (lastBits!=0)                         
					*backLen = (n-1)*8 + lastBits; // 计算返回数据长度
				else
					*backLen = n*8; // 整字节长度
				if (n == 0)                         
					n = 1;                        
				if (n > MAX_LEN)         
					n = MAX_LEN; // 限制最大长度
				// 读取FIFO中的返回数据
				for (i=0; i<n; i++)                 
					backData[i] = Read_MFRC522(FIFODataReg); 
			}
			status = MI_OK; // 通信成功
		}
		else
			status = MI_ERR; // 通信错误
	}	
	Write_MFRC522(ControlReg, 0x80); // 停止定时器
	Write_MFRC522(CommandReg, PCD_IDLE); // 设置为空闲状态
	return status; // 返回通信状态
}

// 功能描述：寻卡并读取卡片类型
// 输入参数：reqMode -- 寻卡模式
//           TagType -- 返回的卡片类型
//           0x4400 = Mifare_UltraLight
//           0x0400 = Mifare_One(S50)
//           0x0200 = Mifare_One(S70)
//           0x0800 = Mifare_Pro(X)
//           0x4403 = Mifare_DESFire
// 返回值：成功返回MI_OK
u8 MFRC522_Request(u8 reqMode, u8 *TagType)
{  
	u8  status;    
	u16 backBits; // 接收到的数据位数
	Write_MFRC522(BitFramingReg, 0x07); // 设置发送最后7位
	TagType[0] = reqMode; // 设置寻卡命令
	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits); 
	if ((status != MI_OK) || (backBits != 0x10)) // 检查状态和返回位数
	{       
		status = MI_ERR; // 寻卡失败
	}
	return status; // 返回寻卡结果
}

// 功能描述：防冲突检测，获取卡片序列号
// 输入参数：serNum -- 返回4字节卡序列号，第5字节为校验字节
// 返回值：成功返回MI_OK
u8 MFRC522_Anticoll(u8 *serNum) 
{     
    u8  status;     
    u8  i;     
    u8  serNumCheck=0;     
    u16 unLen;
    //           
    ClearBitMask(Status2Reg, 0x08); // 清除Crypto1On标志，禁用加密通信
    ClearBitMask(CollReg, 0x80);    // 清除防冲突寄存器标志
    Write_MFRC522(BitFramingReg, 0x00); // 设置发送完整字节
    serNum[0] = PICC_ANTICOLL1;     
    serNum[1] = 0x20;     
    status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);
    //      
    if (status == MI_OK)
    {   
        // 校验卡序列号   
        for(i=0; i<4; i++)   
            serNumCheck ^= serNum[i];
        if(serNumCheck != serNum[i])        
            status = MI_ERR;
    }
    SetBitMask(CollReg, 0x80); // 设置防冲突寄存器
    return status;
}
// 功能描述：计算CRC校验
// 输入参数：pIndata -- 输入数据；len -- 数据长度；pOutData -- CRC结果
void CalulateCRC(u8 *pIndata, u8 len, u8 *pOutData) 
{     
	u16 i;
	u8  n;
	ClearBitMask(DivIrqReg, 0x04); // 清除CRC中断标志
	SetBitMask(FIFOLevelReg, 0x80); // 清空FIFO
	Write_MFRC522(CommandReg, PCD_IDLE); // 设置为空闲状态
	// 向FIFO写入数据
	for (i=0; i<len; i++)
		Write_MFRC522(FIFODataReg, *(pIndata+i));
	Write_MFRC522(CommandReg, PCD_CALCCRC); // 开始CRC计算
	// 等待CRC计算完成
	i = 1000;     
	do      
	{         
		n = Read_MFRC522(DivIrqReg);         
		i--;     
	} while ((i!=0) && !(n&0x04)); // 等待CRC中断
	// 读取CRC结果
	pOutData[0] = Read_MFRC522(CRCResultRegL); // 低字节
	pOutData[1] = Read_MFRC522(CRCResultRegH); // 高字节
	Write_MFRC522(CommandReg, PCD_IDLE); // 设置为空闲状态
}

// 功能描述：选择卡片并获取卡容量
// 输入参数：serNum -- 卡序列号
// 返回值：成功返回卡容量，失败返回0
u8 MFRC522_SelectTag(u8 *serNum) 
{     
	u8  i;     
	u8  status;     
	u8  size;     
	u16 recvBits;     
	u8  buffer[9];
	buffer[0] = PICC_ANTICOLL1; // 防冲突命令
	buffer[1] = 0x70;           // 选择命令
	buffer[6] = 0x00;           // 校验字节初值
	for (i=0; i<4; i++)					
	{
		buffer[i+2] = *(serNum+i); // 填充序列号
		buffer[6] ^= *(serNum+i);  // 计算校验字节
	}
	CalulateCRC(buffer, 7, &buffer[7]); // 计算CRC
	ClearBitMask(Status2Reg, 0x08);     // 清除状态标志
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
	if ((status == MI_OK) && (recvBits == 0x18)) // 检查状态和返回位数
		size = buffer[0]; // 获取卡容量
	else    
		size = 0; // 选择失败
	return size; // 返回卡容量
}

// 功能描述：验证卡片密码
// 输入参数：authMode -- 验证模式（0x60=A密钥，0x61=B密钥）
//           BlockAddr -- 块地址
//           Sectorkey -- 扇区密码
//           serNum -- 卡序列号（4字节）
// 返回值：成功返回MI_OK
u8 MFRC522_Auth(u8 authMode, u8 BlockAddr, u8 *Sectorkey, u8 *serNum) 
{     
	u8  status;     
	u16 recvBits;     
	u8  i;  
	u8  buff[12];    
	// 构造验证数据：验证模式+块地址+扇区密码+卡序列号
	buff[0] = authMode;   // 验证模式
	buff[1] = BlockAddr;  // 块地址
	for (i=0; i<6; i++)
		buff[i+2] = *(Sectorkey+i); // 扇区密码
	for (i=0; i<4; i++)
		buff[i+8] = *(serNum+i);    // 卡序列号
	status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
	if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08))) // 检查验证状态
		status = MI_ERR;
	return status; // 返回验证结果
}

// 功能描述：读取指定块的数据
// 输入参数：blockAddr -- 块地址；recvData -- 存储读取的数据
// 返回值：成功返回MI_OK
u8 MFRC522_Read(u8 blockAddr, u8 *recvData) 
{     
	u8  status;     
	u16 unLen;
	recvData[0] = PICC_READ; // 读命令
	recvData[1] = blockAddr; // 块地址
	CalulateCRC(recvData, 2, &recvData[2]); // 计算CRC
	status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
	if ((status != MI_OK) || (unLen != 0x90)) // 检查状态和返回长度
		status = MI_ERR;
	return status; // 返回读取结果
}

// 功能描述：向指定块写入16字节数据
// 输入参数：blockAddr -- 块地址；writeData -- 要写入的16字节数据
// 返回值：成功返回MI_OK
u8 MFRC522_Write(u8 blockAddr, u8 *writeData) 
{     
	u8  status;     
	u16 recvBits;     
	u8  i;  
	u8  buff[18];
	buff[0] = PICC_WRITE; // 写命令
	buff[1] = blockAddr;  // 块地址
	CalulateCRC(buff, 2, &buff[2]); // 计算CRC
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);
	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) // 检查写命令响应
		status = MI_ERR;
	if (status == MI_OK)     
	{         
		for (i=0; i<16; i++) // 准备16字节数据
			buff[i] = *(writeData+i);
		CalulateCRC(buff, 16, &buff[16]); // 计算CRC
		status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);           
		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) // 检查数据写入响应
			status = MI_ERR;         
	}          
	return status; // 返回写入结果
}

// 功能描述：命令卡片进入休眠状态
void MFRC522_Halt(void) 
{    
	u16 unLen;     
	u8  buff[4];
	buff[0] = PICC_HALT; // 休眠命令
	buff[1] = 0;         // 填充字节
	CalulateCRC(buff, 2, &buff[2]); // 计算CRC
	MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen); // 执行休眠命令
}
