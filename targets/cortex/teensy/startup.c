///@file
/**
 * @file bootdata.c
 * @author Paul Stoffregen (https://github.com/PaulStoffregen)
 * @author Oscar Kromhout (https://github.com/OscarKro)
 * @brief This file was originally made by Paul stoffregen and contains all the booting and startup instructions for the imxrt1062 on the Teensy 40 board. Oscar Kromhout edited a lot (mainly removed) of it to work with BMPTK
 * @version 0.1
 * @date 2020-10-08
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "mimxrt1062.h"

// from the linker
extern unsigned long _stextload;
extern unsigned long _stext;
extern unsigned long _etext;
extern unsigned long _sdataload;
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sbss;
extern unsigned long _ebss;
extern unsigned long _flexram_bank_config;
extern unsigned long _estack;


static void memory_copy(uint32_t *dest, const uint32_t *src, uint32_t *dest_end);
static void memory_clear(uint32_t *dest, uint32_t *dest_end);
/**
 * @brief Function to initialize/configurate the necessery PLL's and their respective PFD's
 * 
 */
static void init_pll();


extern int main (void);

__attribute__((section(".startup"), optimize("no-tree-loop-distribute-patterns"), naked))
/**
 * @brief This function is the startup
 * 
 */
void ResetHandler(void)
{

	// pin 13 - if startup crashes, use this to turn on the LED early for troubleshooting
	// IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03] = 5;
	// IOMUXC_GPR->GPR27 = (1<<3);
	// GPIO7->GDIR |= (1<<3);
	// GPIO7->DR_SET |= (1<<3);


	// Initialize memory
	memory_copy(&_stext, &_stextload, &_etext); // copy the memory from the load adress (flash) of text, to the itcm
	memory_copy(&_sdata, &_sdataload, &_edata); // copy the memory from the load adress (flash, data) to the DTCM
	memory_clear(&_sbss, &_ebss); // clear the bss (initialize with all zeros)

	init_pll();
	// Turn on the fast GPIO ports ( gpio 1-5 are standard speed, 6-9 are high speed, 1 & 6 share the same chip pad, 2 & 7 too, etc)
	IOMUXC_GPR->GPR26 = 0xFFFFFFFF;
	IOMUXC_GPR->GPR27 = 0xFFFFFFFF;
	IOMUXC_GPR->GPR28 = 0xFFFFFFFF;
	IOMUXC_GPR->GPR29 = 0xFFFFFFFF;

	// TODO: SET THE SYSTEM TICK ON?!
	// TODO: FLOATING POINT UNIT ON!

	main(); // call the main from the user
	
	// when returned from the main, loop till hell freezes over, but do something to eliminate unidentified behaviour
	volatile int a = 0;
	while (1)
	{
		if (a == 100000)
		{
			a = 0;
		}
		a++;
	}
}

__attribute__((section(".startup"), optimize("no-tree-loop-distribute-patterns")))
static void memory_copy(uint32_t *dest, const uint32_t *src, uint32_t *dest_end)
{
	if (dest == src) return;
	while (dest < dest_end) {
		*dest++ = *src++;
	}
}

__attribute__((section(".startup"), optimize("no-tree-loop-distribute-patterns")))
static void memory_clear(uint32_t *dest, uint32_t *dest_end)
{
	while (dest < dest_end) {
		*dest++ = 0;
	}
}

__attribute__((section(".startup"), optimize("no-tree-loop-distribute-patterns")))
static void init_pll()
{
	//calculate the right setting for the arm clock. Fout = Fin * div_sel / 2. So: 650 Mhz (the minimum) = 24 * ? / 2 = 54. 
	//Because there is a divider in between the pll1 and arm cpu and we want 480 mhz (the max of the cpu), 
	//the formula becomes: 960 Mhz = 24 * ? / 2 = 80. 80 binary = 0b1010000
	CCM_ANALOG->PLL_ARM |= (0b1<<16); //bypass the pll to the oscillator
	CCM_ANALOG->PLL_ARM &= ~(0X7F); // set the first 7 bits to zero
	CCM_ANALOG->PLL_ARM |= 0b1010000; // write 80 for 480 mhz in the register
	CCM_ANALOG->PLL_ARM &= ~(0b1 << 16); // set the pll to the arm clock again
	//TODO: IMPLEMENT OTHER PLL'S LIKE USB AND SUCH, FOR PLL'S WITH PFD SEE P4. "Configuration of phase fractional dividers" FROM NXP
}



