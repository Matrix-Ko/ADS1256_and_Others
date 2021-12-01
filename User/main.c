/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : ADS1256, 8通道带PGA的24bit ADC
*              实验目的：
*                1.	学习ADS1256, 8通道带PGA的24bit ADC。
*              实验内容： 
*                1. 启动一个自动重装软件定时器，每100ms翻转一次LED2。
*                2.	上电后插入ADS1256模块到右上角CN26（2*6P双排母）, 程序每1秒打印一次数据
*              注意事项：
*                1. 打印方式：
*                   方式一：
*                     默认使用串口打印，可以使用SecureCRT或者H7-TOOL上位机串口助手查看打印信息，
*                     波特率115200，数据位8，奇偶校验位无，停止位1。
*                   方式二：
*                     使用RTT打印，可以使用SEGGE RTT或者H7-TOOL RTT打印。
*                     MDK AC5，MDK AC6或IAR通过使能bsp.h文件中的宏定义为1即可
*                     #define Enable_RTTViewer  1
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2021-09-17   Eric2013     1. CMSIS软包版本 V5.8.0
*                                         2. HAL库版本 V1.10.0
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/	
#include "bsp.h"			 	/* 底层硬件驱动 */
#include "arm_math.h"
#include "arm_const_structs.h"


/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"ADS1256(8通道带PGA的24bit ADC)"
#define EXAMPLE_DATE	"2021-09-17"
#define DEMO_VER		"1.0"


/* 仅允许本文件内调用的函数声明 */
static void PrintfLogo(void);
static void PrintfHelp(void);


/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参: 无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
	uint8_t i;
	int32_t iTemp;
	float fTemp;

	
	bsp_Init();		/* 硬件初始化 */
	PrintfLogo();	/* 打印例程信息到串口1 */

	PrintfHelp();	/* 打印操作提示信息 */
	
	
	bsp_DelayMS(500);	/* 等待上电稳定，等基准电压电路稳定, bsp_InitADS1256() 内部会进行自校准 */

	bsp_InitADS1256();	/* 初始化配置ADS1256.  PGA=1, DRATE=30KSPS, BUFEN=1, 输入正负5V */
	
	
	/* 打印芯片ID (通过读ID可以判断硬件接口是否正常) , 正常时状态寄存器的高4bit = 3 */
#if 0
	{
		uint8_t id;

		id = ADS1256_ReadChipID();

		if (id != 3)
		{
			printf("Error, ASD1256 Chip ID = 0x%X\r\n", id);
		}
		else
		{
			printf("Ok, ASD1256 Chip ID = 0x%X\r\n", id);
		}
	}
#endif
	
	ADS1256_CfgADC(ADS1256_GAIN_1, ADS1256_30SPS);	/* 配置ADC参数： 增益1:1, 数据输出速率 30Hz */

	ADS1256_StartScan();	/* 启动中断扫描模式, 轮流采集8个通道的ADC数据. 通过 ADS1256_GetAdc() 函数来读取这些数据 */
	
	bsp_StartAutoTimer(0, 1000);	/* 启动1个100ms的自动重装的定时器 */

	/* 进入主程序循环体 */
	while (1)
	{
		bsp_Idle();		/* 这个函数在bsp.c文件。用户可以修改这个函数实现CPU休眠和喂狗 */
		

		if (bsp_CheckTimer(0))	/* 判断定时器超时时间 */
		{
			/* 每隔1000ms 进来一次 */
			bsp_LedToggle(2);	/* 翻转LED的状态 */
			
			/* 打印采集数据 */
			for (i = 0; i < 8; i++)
			{

				iTemp = ((int64_t)g_tADS1256.AdcNow[i] * 2500000) / 4194303; /* 计算实际电压值（近似估算的），如需准确，请进行校准 */
				
				fTemp = (float)iTemp / 1000000;   

				printf("CH%d=%07d(%fV) ", i, g_tADS1256.AdcNow[i], fTemp);

				if(i == 3)
				{
					printf("\r\n");
				}
			}
			
			printf("\r\n\r\n");
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: PrintfHelp
*	功能说明: 显示操作提示菜单
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfHelp(void)
{
	printf("1. 启动一个自动重装软件定时器，每100ms翻转一次LED2\r\n");
	printf("2. 上电后插入ADS1256模块到右上角CN26（2*6P双排母）, 程序每1秒打印一次数据\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: PrintfLogo
*	功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("*************************************************************\n\r");
	
	/* 检测CPU ID */
	{
		uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
		
		CPU_Sn0 = *(__IO uint32_t*)(0x1FF1E800);
		CPU_Sn1 = *(__IO uint32_t*)(0x1FF1E800 + 4);
		CPU_Sn2 = *(__IO uint32_t*)(0x1FF1E800 + 8);

		printf("\r\nCPU : STM32H743XIH6, BGA240, 主频: %dMHz\r\n", 480);
		printf("UID = %08X %08X %08X\n\r", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	}

	printf("\n\r");
	printf("*************************************************************\n\r");
	printf("* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	/* 打印ST的HAL库版本 */
	printf("* HAL库版本  : V1.10.0 (STM32H7xx HAL Driver)\r\n");
	printf("* \r\n");	/* 打印一行空格 */
	printf("* QQ    : 1295744630 \r\n");
	printf("* 旺旺  : armfly\r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* 微信公众号: armfly_com \r\n");
	printf("* 淘宝店: armfly.taobao.com\r\n");
	printf("* Copyright www.armfly.com 安富莱电子\r\n");
	printf("*************************************************************\n\r");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
