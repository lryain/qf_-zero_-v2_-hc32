#include "ddl.h"
#include "lpt.h"
#include "lpm.h"
#include "gpio.h"


/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static volatile uint32_t u32LptTestFlag = 0;


/*******************************************************************************
 * Lpt中断服务函数
 ******************************************************************************/
void LptInt(void)
{
    if (TRUE == Lpt_GetIntFlag())
    {
        Lpt_ClearIntFlag();
        u32LptTestFlag = 0x01;
    }
}

/*******************************************************************************
 * Lpt计数功能测试 (自由计数)
 ******************************************************************************/
en_result_t LptCntTest(void)
{
    stc_lpt_config_t stcConfig;
    en_result_t      enResult = Error;
    uint16_t         u16ArrData = 0xE000;
    volatile uint32_t u32LptTestFlagTmp = 0;
    
    stc_lpm_config_t stcLpmCfg;
    
    stcConfig.pfnLpTimCb = LptInt;
    
    stcConfig.enGateP  = LptPositive; 
    stcConfig.enGate   = LptGateDisable;
    stcConfig.enTckSel = LptIRC32K;
    stcConfig.enTog    = LptTogDisable;
    stcConfig.enCT     = LptCounter;
    stcConfig.enMD     = LptMode1;
    
    //config GPIO
    Gpio_SetFunc_LPTIM_EXT_P03();
    
    Gpio_InitIO(2, 5, GpioDirIn);
    Gpio_InitIO(2, 6, GpioDirOut);
    Gpio_SetIO(2, 6, FALSE);
    
    if (Ok != Lpt_Init(&stcConfig))
    {
        enResult = Error;
    }
    
    //Lpm Cfg
    stcLpmCfg.enSEVONPEND   = SevPndDisable;
    stcLpmCfg.enSLEEPDEEP   = SlpDpEnable;
    stcLpmCfg.enSLEEPONEXIT = SlpExtDisable;
    Lpm_Config(&stcLpmCfg);
    
    //Lpt中断使能
    Lpt_ClearIntFlag();
    Lpt_EnableIrq();
    EnableNvic(LPTIM_IRQn, 3, TRUE); 
    
    //设置重载值，启动运行
    Lpt_ARRSet(u16ArrData);
    Lpt_Run();
    
    
    //判断P25，如果为高电平则，进入低功耗模式……
    //注：若芯片处于低功耗模式，则芯片无法使用SWD进行调式和下载功能。
    //故如需要继续下载调试其他程序，需要将P25接低电平。
#if 0
    if (TRUE == Gpio_GetIO(2, 5))
    {
        Gpio_SetIO(2, 6, TRUE);
        Lpm_GotoLpmMode();
    }
#endif    
    //低功耗模式下，继续计数，直到溢出产生中断，退出低功耗模式。
    while(1)
    {
    	u32LptTestFlagTmp = u32LptTestFlag;
        if (0x01 == u32LptTestFlagTmp)
        {
            u32LptTestFlag = 0;
            u32LptTestFlagTmp = 0;
            Lpt_Stop();
            enResult = Ok;
            
            Gpio_SetIO(2, 6, FALSE);
            break;
        }
    }
    
    return enResult;
}

/**
 ******************************************************************************
 ** \brief  Main function of project
 **
 ** \return uint32_t return value, if needed
 **
 ** This sample
 **
 ******************************************************************************/

int32_t main(void)
{
    volatile uint8_t u8TestFlag = 0;
    
    //CLK INIT
    stc_clk_config_t stcClkCfg;
    stcClkCfg.enClkSrc  = ClkRCH;
    stcClkCfg.enHClkDiv = ClkDiv1;
    stcClkCfg.enPClkDiv = ClkDiv1;
    
    Clk_Init(&stcClkCfg);
    
    //使能RCL
    Clk_Enable(ClkRCL, TRUE);
    
    //使能Lpt,GPIO外设时钟
    Clk_SetPeripheralGate(ClkPeripheralLpTim, TRUE);
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);
    if (Ok != LptCntTest())
    {
        u8TestFlag |= 0x01;
    }
        
    while (1);
}

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


