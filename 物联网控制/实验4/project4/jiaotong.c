#include <reg51.h>

// 数码管位选
sbit LSA = P2^2;
sbit LSB = P2^3;
sbit LSC = P2^4;

// 交通灯
sbit EW_GREEN = P1^0;   // 东西绿灯
sbit EW_YELLOW = P1^1;  // 东西黄灯
sbit EW_RED = P1^2;     // 东西红灯
sbit SN_GREEN = P1^3;   // 南北绿灯
sbit SN_YELLOW = P1^4;  // 南北黄灯
sbit SN_RED = P1^5;     // 南北红灯

// 段码表
unsigned char code seg[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};

unsigned int second = 30;   // 倒计时
unsigned char state = 0;    // 0:东西绿 1:东西黄 2:南北绿 3:南北黄
unsigned int cnt = 0;       // 1ms计数

// 定时器初始化
void Timer0_Init(void)
{
    TMOD |= 0x01;
    TH0 = 0xFC;
    TL0 = 0x18;
    EA = 1;
    ET0 = 1;
    TR0 = 1;
}

// 延时
void delay(unsigned int ms)
{
    unsigned int i,j;
    for(i=0;i<ms;i++)
        for(j=0;j<120;j++);
}

// 显示数字（两位）
void Display(unsigned int num)
{
    unsigned char shi, ge;
    shi = num / 10;
    ge = num % 10;
    
    // 十位
    LSA=0; LSB=0; LSC=0;
    P0 = seg[shi];
    delay(2);
    
    // 个位
    LSA=1; LSB=0; LSC=0;
    P0 = seg[ge];
    delay(2);
}

// 设置灯状态
void SetLight(void)
{
    // 全部熄灭
    EW_GREEN=EW_YELLOW=EW_RED=0;
    SN_GREEN=SN_YELLOW=SN_RED=0;
    
    switch(state)
    {
        case 0: EW_GREEN=1; SN_RED=1; break;   // 东西绿
        case 1: EW_YELLOW=1; SN_RED=1; break;  // 东西黄
        case 2: SN_GREEN=1; EW_RED=1; break;   // 南北绿
        case 3: SN_YELLOW=1; EW_RED=1; break;  // 南北黄
    }
}

// 主函数
void main(void)
{
    Timer0_Init();
    SetLight();
    
    while(1)
    {
        Display(second);
        
        switch(state)
        {
            case 0: if(second==0){second=5; state=1; SetLight();} break;
            case 1: if(second==0){second=30; state=2; SetLight();} break;
            case 2: if(second==0){second=5; state=3; SetLight();} break;
            case 3: if(second==0){second=30; state=0; SetLight();} break;
        }
    }
}

// 定时器中断
void Timer0_ISR(void) interrupt 1
{
    TH0=0xFC;
    TL0=0x18;
    cnt++;
    if(cnt>=1000)
    {
        cnt=0;
        if(second>0) second--;
    }
}