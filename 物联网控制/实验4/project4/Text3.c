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

// 独立按键
sbit KEY_SET = P3^1;    // 设置键 K1
sbit KEY_ADD = P3^2;    // 加键 K2
sbit KEY_SUB = P3^3;    // 减键 K3

// 段码表
unsigned char code seg[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};

// 时间参数
unsigned int EW_GREEN_TIME = 10;   // 东西绿灯时间
unsigned int SN_GREEN_TIME = 10;   // 南北绿灯时间
unsigned int YELLOW_TIME   = 3;    // 黄灯时间

unsigned int second = 10;          // 当前倒计时
unsigned char state = 0;           // 0:东西绿 1:东西黄 2:南北绿 3:南北黄
unsigned int cnt = 0;              // 1ms计数

bit SET_MODE = 0;                 // 0:运行模式 1:设置模式
unsigned char set_item = 0;        // 设置项 0:东西绿 1:南北绿 2:黄灯

// 函数声明
void Timer0_Init(void);
void delay(unsigned int ms);
void Display(unsigned int num);
void SetLight(void);
void KeyScan(void);

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

// 按键扫描
void KeyScan(void)
{
    if(KEY_SET == 0)        // 设置键
    {
        delay(10);
        if(KEY_SET == 0)
        {
            SET_MODE = !SET_MODE;
            if(SET_MODE == 1)
            {
                set_item = 0;          // 进入设置，从东西绿灯开始
            }
            else
            {
                // 退出设置，恢复正常倒计时
                second = EW_GREEN_TIME;
                state = 0;
                SetLight();
            }
            while(KEY_SET == 0);
        }
    }

    if(SET_MODE)           // 设置模式下
    {
        if(KEY_ADD == 0)   // 加键
        {
            delay(10);
            if(KEY_ADD == 0)
            {
                switch(set_item)
                {
                    case 0: if(EW_GREEN_TIME < 99) EW_GREEN_TIME++; break;
                    case 1: if(SN_GREEN_TIME < 99) SN_GREEN_TIME++; break;
                    case 2: if(YELLOW_TIME < 99) YELLOW_TIME++; break;
                }
                while(KEY_ADD == 0);
            }
        }

        if(KEY_SUB == 0)   // 减键
        {
            delay(10);
            if(KEY_SUB == 0)
            {
                switch(set_item)
                {
                    case 0: if(EW_GREEN_TIME > 1) EW_GREEN_TIME--; break;
                    case 1: if(SN_GREEN_TIME > 1) SN_GREEN_TIME--; break;
                    case 2: if(YELLOW_TIME > 1) YELLOW_TIME--; break;
                }
                while(KEY_SUB == 0);
            }
        }

        // 短按切换设置项（可选）
        // 如需自动切换，可在主循环中增加计时切换
    }
}

// 主函数
void main(void)
{
    Timer0_Init();
    SetLight();
    
    while(1)
    {
        KeyScan();
        
        if(SET_MODE)    // 设置模式
        {
            switch(set_item)
            {
                case 0: Display(EW_GREEN_TIME); break;
                case 1: Display(SN_GREEN_TIME); break;
                case 2: Display(YELLOW_TIME); break;
            }
            
            // 短按设置键切换设置项（可选）
            // 此处可添加长按切换逻辑
        }
        else            // 运行模式
        {
            Display(second);
            
            switch(state)
            {
                case 0: if(second==0){second=YELLOW_TIME; state=1; SetLight();} break;
                case 1: if(second==0){second=SN_GREEN_TIME; state=2; SetLight();} break;
                case 2: if(second==0){second=YELLOW_TIME; state=3; SetLight();} break;
                case 3: if(second==0){second=EW_GREEN_TIME; state=0; SetLight();} break;
            }
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
        if(!SET_MODE && second>0) second--;
    }
}