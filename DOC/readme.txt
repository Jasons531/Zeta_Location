
	
1��Ӳ��IO�ӿ�ͼ��

								stm32L072CBT6
SX1278						 _ _ _ _ _ _ _ _ _ _ _ _ _ _ 
SPI     NSS	  --------	PA4 |					  		|
		SCK	  --------	PA5 |    				  		|
		MISO  --------	PA6 |					 		|
		MOSI  --------	PA7 |					  		|
						    |					  		|
EXti	DIO0  --------	PB1 |                    		|
		DIO1  --------	PB2 |					  		|
		DIO2  --------	PB10|					  		|
		DIO3  --------	PB11|					 		|
		DIO4  --------	NC	|					  		|
		DIO5  --------	NC	|					 		|
							|					  		|
CPIO	RESET --------	PB0 |					  		|
		LoRa_Power ---  PB12|					 		|
							|					 		|
							|					 		|
GPS	(UART2)					|					 		|	
		TX	  --------  PA2	|					  		|	
		RX	  --------  PA3	|					  		|	
GPS_Power_ON  --------  PB7	|					  		|									
							|					  		|
485	(UART5)					|					 		|	
		485_TX	------	PB3	|					  		|	
		485_RX	------	PB4	|					  		|	
		485_DE	------	PB5	|					  		|
		12V_ON	------  PA8	|					  		|	
							|					  		|
							|					 		|	
DEBUG(UART1)				|					  		|
		TX   ---------	PA9	|					  		|
		RX	 ---------  PA10|					  		|
							|					  		|
I2C							|					  		|
		I2C2_SDA ----- PB14	|					  		|
		I2C2_SCL ----- PB13	|					  		|
							|					  		|
��Դ����ʹ��  -------- PB9	|					  		|
							|					  		|
							|					  		|
							|					  		|
							|_ _ _ _ _ _ _ _ _ _ _ _ _ _|	



�汾˵����

��1����ZETA-Location-V0.1
���ܣ�
��1�����߷�ʽ�����֣�
��1.1����PA0��Դ���ش��ڴ���ģʽ���л�������͹���ģʽ
��1.2����PA0�������Ѻ�������ģʽ, RTC��PC13���߻��ѣ���Ӧ������͹���ģʽ


�𾴵Ĺ���Ա��
??? ���ã�
??? ��¼��Ϣ�Ѿ�������£�
??? �û�����nbcx
??? ���룺ca722db37e1a10f6

??? �����ڵ�¼����ҳ������Ͻǽ��������޸�,Ϊ�������˺Ű�ȫ,����ʹ�ù��ڼ򵥵����롣

??? Ϊ�������õ�ʹ�����飬�����Ƽ���ʹ��Google Chrome�����

??? ��Ϣ����ϵͳ��ַ:?http://www.zeta-alliance.com:25450/teamcms/homePage



11 03 04 01 99 00 FC 3A 60 

00 50 03 01 11 02 05 03 02 01 77 00 FC 02 90 00 F2 00 00 

RS485_IDE_LEN+4
SaveRs485s[index].MainBox.ExpendBox[ExpId].SensorToLen -= 4; ///���˵�4�ֽڣ��������ͱ�ʾ

A4014D
A4021E


��λ��Ϣ��

$GPGLL,2232.9085,N,11356.5973,E,111334.000,A,A*51
$GPGLL,2232.9080,N,11356.5993,E,111335.000,A,A*5B
$GPGLL,2232.9077,N,11356.6003,E,111336.000,A,A*53
$GPGLL,2232.9073,N,11356.6016,E,111337.000,A,A*52
$GPGLL,2232.9070,N,11356.6025,E,111337.205,A,A*56
$GPGLL,2232.9064,N,11356.6032,E,111339.000,A,A*5C
$GPGLL,2232.9058,N,11356.6037,E,111340.000,A,A*58
$GPGLL,2232.9051,N,11356.6043,E,111341.000,A,A*53
$GPGLL,2232.9045,N,11356.6053,E,111342.000,A,A*54
$GPGLL,2232.9038,N,11356.6063,E,111343.000,A,A*5C
$GPGLL,2232.9034,N,11356.6071,E,111343.206,A,A*57
$GPGLL,2232.9028,N,11356.6081,E,111345.000,A,A*57
$GPGLL,2232.9023,N,11356.6089,E,111346.000,A,A*57
$GPGLL,2232.9020,N,11356.6098,E,111347.000,A,A*55
$GPGLL,2232.9016,N,11356.6104,E,111348.000,A,A*5B
$GPGLL,2232.9012,N,11356.6112,E,111349.000,A,A*59
$GPGLL,2232.9010,N,11356.6123,E,111349.206,A,A*5D
$GPGLL,2232.9007,N,11356.6133,E,111351.000,A,A*57
$GPGLL,2232.9006,N,11356.6144,E,111352.000,A,A*55
$GPGLL,2232.9006,N,11356.6153,E,111353.000,A,A*52

����ͨ���汾��֤
char str[100] ="$GPGLL,2232.9085,N,11356.5973,E,111334.000,A,A*51";
char GPLL[10];
char N_Data[15] ;
char N;
char E_Data[15] ;

double data_N = 0;
double data_E = 0;

sscanf(str, "%[^,]%*[,] %[^,]%*[,] %[^,]%*[,] %[^,]%*[,]", GPLL,N_Data, &N, E_Data);     ////ȡ����,��ֹ��ͬʱ����,

printf("The lowercase is: %s %s %c %s\r\n", GPLL,N_Data,N,E_Data);

sscanf(N_Data, "%lf", &data_N);
sscanf(E_Data, "%lf", &data_E);

SetGpsAck.EastSpend = data_E * 10000;
SetGpsAck.NorthSpend = data_N * 10000;

printf("E_Data = %.4f N_Data = %.4f E = %d, N = %d\r\n",data_N,data_E, SetGpsAck.EastSpend,SetGpsAck.NorthSpend);

SetGpsAck.South = true;
SetGpsAck.West = true;

printf("state = 0x%02x\r\n",0x30 | (SetGpsAck.West << 1) | (SetGpsAck.South << 0));

ZetaSendBuf.Buf[len++] = 0x30 | (SetGpsAck.West << 1) | (SetGpsAck.South << 0);

ZetaSendBuf.Buf[1] = (SetGpsAck.EastSpend >> 20)&0xFF; ///28bitȡ��8bit��ע���32bitȡ����
ZetaSendBuf.Buf[2] = (SetGpsAck.EastSpend >> 12)&0xFF;
ZetaSendBuf.Buf[3] = (SetGpsAck.EastSpend >> 4)&0xFF;

ZetaSendBuf.Buf[4] = (((SetGpsAck.EastSpend >> 0) & 0xF) << 4) | ((SetGpsAck.NorthSpend >> 24) & 0xF); ///28bitȡ��4bit

ZetaSendBuf.Buf[5] = (SetGpsAck.NorthSpend >> 16) & 0xFF;
ZetaSendBuf.Buf[6] = (SetGpsAck.NorthSpend >> 8) & 0xFF;
ZetaSendBuf.Buf[7] = (SetGpsAck.NorthSpend >> 0) & 0xFF;

for(uint8_t i = 0; i < 8; ++i)
{
printf("buf[%d]%02X\r\n",i,ZetaSendBuf.Buf[i]);
}




ZetaSendBuf: FF 00 0C 02 30 6C 4D FB 61 54 B4 93 
start send data
FF 00 0C 02 30 6C 4D FB 61 54 B4 93 
ff 00 04 01 


Program Size: Code=29780 RO-data=940 RW-data=320 ZI-data=3968  




