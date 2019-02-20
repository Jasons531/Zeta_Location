


版本说明：

【V1.6.3】：ZETA-Location-V1.6
功能：
【1】：增加更改加速度传感器灵敏度指令

【V1.6.2】：ZETA-Location-V1.5.2
功能：
【1】：MMA8452INT_1 IO更改为PC13

【V1.6】：ZETA-Location-V1.5.1
功能：
【1】：解决移动报警、停止条件时间溢出问题；


【V1.6】：ZETA-Location-V1.5
功能：
【1】：降低外部中断EXTI0_1_IRQn优先级 < uart_gps；
【2】：MMA8452q延时降低为80ms

【V1.5】：ZETA-Location-V1.5
功能：
【1】：优化定位器软件框架；
【2】：添加添加上电读取flash
【3】：增加心跳周期获取GPS位置信息

【V1.4】：ZETA-Location-V1.4
功能：
【1】：添加休眠时间对齐心跳时间，确保心跳周期完整性

【V1.3】：ZETA-Location-V1.3
功能：
【1】：解决RTC内部冲突机制

【V1.2】：ZETA-Location-V1.2
功能：
【1】：优化MMA8452q加速度传感器功耗，单机功耗：10ua，加入mma8452q功耗36ua


【V1.1】：ZETA-Location-V1.1
功能：
【1】：添加MMA8452q加速度传感器，使用中断方式：配置中断需要配置CTRL_REG3、CTRL_REG4、CTRL_REG5，同时配置三个寄存器；
解决MMA8452Q中断触发失败，原因：CTRL_REG3：IPOL bit位：IPOL = 0 :该位代表中断脉冲当前是一直处于高电平，MCU端采用下降沿触发，否则失败，IPOL = 1，则为下降沿触发

【V1.0】：ZETA-Location-V1.0
功能：
【1】休眠方式分两种：
【1.1】：PA0电源开关处于待机模式，切换进入最低功耗模式
【1.2】：PA0待机唤醒后处于休眠模式, RTC、PC13休眠唤醒，相应处于最低功耗模式


尊敬的管理员：
??? 您好！
??? 登录信息已经变更如下：
??? 用户名：nbcx
??? 密码：ca722db37e1a10f6

??? 您可在登录后，于页面的右上角进行密码修改,为了您的账号安全,请勿使用过于简单的密码。

??? 为了您更好的使用体验，我们推荐您使用Google Chrome浏览器

??? 信息管理系统地址:?http://www.zeta-alliance.com:25450/teamcms/homePage



11 03 04 01 99 00 FC 3A 60 

00 50 03 01 11 02 05 03 02 01 77 00 FC 02 90 00 F2 00 00 

RS485_IDE_LEN+4
SaveRs485s[index].MainBox.ExpendBox[ExpId].SensorToLen -= 4; ///过滤掉4字节，数据类型表示

A4014D
A4021E


定位信息：

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

测试通过版本验证
char str[100] ="$GPGLL,2232.9085,N,11356.5973,E,111334.000,A,A*51";
char GPLL[10];
char N_Data[15] ;
char N;
char E_Data[15] ;

double data_N = 0;
double data_E = 0;

sscanf(str, "%[^,]%*[,] %[^,]%*[,] %[^,]%*[,] %[^,]%*[,]", GPLL,N_Data, &N, E_Data);     ////取数到,截止，同时过滤,

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

ZetaSendBuf.Buf[1] = (SetGpsAck.EastSpend >> 20)&0xFF; ///28bit取高8bit，注意非32bit取运算
ZetaSendBuf.Buf[2] = (SetGpsAck.EastSpend >> 12)&0xFF;
ZetaSendBuf.Buf[3] = (SetGpsAck.EastSpend >> 4)&0xFF;

ZetaSendBuf.Buf[4] = (((SetGpsAck.EastSpend >> 0) & 0xF) << 4) | ((SetGpsAck.NorthSpend >> 24) & 0xF); ///28bit取高4bit

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




