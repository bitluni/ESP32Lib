/*
	Author: bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/

#ifdef ESP32

#include "soc/gpio_periph.h"
#include "I2S.h"

#include "esp_heap_caps.h"
#include "soc/soc.h"
#include "soc/gpio_sig_map.h"
#include "soc/i2s_reg.h"
#include "soc/i2s_struct.h"
#include "soc/io_mux_reg.h"
#include "soc/rtc.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "rom/lldesc.h"
#include "rom/gpio.h"

#include "../Tools/Log.h"
#include <soc/rtc.h>
#include <driver/rtc_io.h>

#include "driver/dac.h"


i2s_dev_t *i2sDevices[] = {&I2S0, &I2S1};

I2S::I2S(const int i2sIndex)
{
	const periph_module_t deviceModule[] = {PERIPH_I2S0_MODULE, PERIPH_I2S1_MODULE};
	this->i2sIndex = i2sIndex;
	//enable I2S peripheral
	periph_module_enable(deviceModule[i2sIndex]);
	interruptHandle = 0;
	dmaBufferDescriptorCount = 0;
	dmaBufferDescriptorActive = 0;
	dmaBufferDescriptors = 0;
	stopSignal = false;
}

void IRAM_ATTR I2S::interruptStatic(void *arg)
{
	volatile i2s_dev_t &i2s = *i2sDevices[((I2S *)arg)->i2sIndex];

	//CLEAR THE INTERRUPT FLAG
	//i2s object not safely accesed in DRAM or IRAM
	//i2s.int_clr.val = i2s.int_raw.val;
	//using REG_WRITE to clear the interrupt instead
	//note: there are still other alternatives, see i2s driver .c file
	//inside the i2s_intr_handler_default() function
	REG_WRITE(I2S_INT_CLR_REG(((I2S *)arg)->i2sIndex), (REG_READ(I2S_INT_RAW_REG(((I2S *)arg)->i2sIndex)) & 0xffffffc0) | 0x3f);

	//UPDATE dmaBufferDescriptorActive
	((I2S *)arg)->dmaBufferDescriptorActive = (REG_READ(I2S_OUT_EOF_DES_ADDR_REG(((I2S *)arg)->i2sIndex)) - (uint32_t)((I2S *)arg)->dmaBufferDescriptors)/sizeof(DMABufferDescriptor);
	//Actually, it is not the active one, but the one just finished: index of the buffer that just triggered EOF

	//CALL FUNCTION THAT MUST BE EXECUTED DURING INTERRUPT
	//the call to the overloaded (or any) non-static member function definitely breaks the IRAM rule
	// causing an exception when concurrently accessing the flash (or flash-filesystem) or wifi
	//the reason is unknown but probably related with the compiler instantiation mechanism
	//(note: defining the code of the [member] interrupt function outside the class declaration,
	// and with IRAM flag does not avoid the crash)
	//((I2S *)arg)->interrupt();
	if(((I2S *)arg)->interruptStaticChild)
		((I2S *)arg)->interruptStaticChild(arg);
}

void I2S::reset()
{
	volatile i2s_dev_t &i2s = *i2sDevices[i2sIndex];
	const unsigned long lc_conf_reset_flags = I2S_IN_RST_M | I2S_OUT_RST_M | I2S_AHBM_RST_M | I2S_AHBM_FIFO_RST_M;
	i2s.lc_conf.val |= lc_conf_reset_flags;
	i2s.lc_conf.val &= ~lc_conf_reset_flags;

	const uint32_t conf_reset_flags = I2S_RX_RESET_M | I2S_RX_FIFO_RESET_M | I2S_TX_RESET_M | I2S_TX_FIFO_RESET_M;
	i2s.conf.val |= conf_reset_flags;
	i2s.conf.val &= ~conf_reset_flags;
	while (i2s.state.rx_fifo_reset_back)
		;
}

void I2S::i2sStop()
{
	volatile i2s_dev_t &i2s = *i2sDevices[i2sIndex];
	esp_intr_disable((intr_handle_t)interruptHandle);
	reset();
	i2s.conf.rx_start = 0;
	i2s.conf.tx_start = 0;
}

void I2S::startTX()
{
	volatile i2s_dev_t &i2s = *i2sDevices[i2sIndex];
	DEBUG_PRINTLN("I2S TX");
	esp_intr_disable((intr_handle_t)interruptHandle);
	reset();
    i2s.lc_conf.val    = I2S_OUT_DATA_BURST_EN | I2S_OUTDSCR_BURST_EN;
	dmaBufferDescriptorActive = 0;
	i2s.out_link.addr = (uint32_t)firstDescriptorAddress();
	i2s.out_link.start = 1;
	i2s.int_clr.val = i2s.int_raw.val;
	i2s.int_ena.val = 0;
	if(useInterrupt())
	{
		i2s.int_ena.out_eof = 1;
		//enable interrupt
		esp_intr_enable((intr_handle_t)interruptHandle);
	}
	//start transmission
	i2s.conf.tx_start = 1;
}

void I2S::startRX()
{
	volatile i2s_dev_t &i2s = *i2sDevices[i2sIndex];
	DEBUG_PRINTLN("I2S RX");
	esp_intr_disable((intr_handle_t)interruptHandle);
	reset();
	dmaBufferDescriptorActive = 0;
	i2s.rx_eof_num = dmaBufferDescriptors[0].sampleCount();	//TODO: replace with cont of sample to be recorded
	i2s.in_link.addr = (uint32_t)firstDescriptorAddress();
	i2s.in_link.start = 1;
	i2s.int_clr.val = i2s.int_raw.val;
	i2s.int_ena.val = 0;
	i2s.int_ena.in_done = 1;
	esp_intr_enable((intr_handle_t)interruptHandle);
	i2s.conf.rx_start = 1;
}

void I2S::resetDMA()
{
	volatile i2s_dev_t &i2s = *i2sDevices[i2sIndex];
	i2s.lc_conf.in_rst = 1;
	i2s.lc_conf.in_rst = 0;
	i2s.lc_conf.out_rst = 1;
	i2s.lc_conf.out_rst = 0;
}

void I2S::resetFIFO()
{
	volatile i2s_dev_t &i2s = *i2sDevices[i2sIndex];
	i2s.conf.rx_fifo_reset = 1;
	i2s.conf.rx_fifo_reset = 0;
	i2s.conf.tx_fifo_reset = 1;
	i2s.conf.tx_fifo_reset = 0;
}

DMABufferDescriptor *I2S::firstDescriptorAddress() const
{
	return &dmaBufferDescriptors[0];
}

bool I2S::useInterrupt()
{ 
	return false; 
};

void I2S::getClockSetting(long *sampleRate, int *n, int *a, int *b, int *div)
{
	if(sampleRate)
		*sampleRate = 2000000;
	if(n)
		*n = 2;
	if(a)
		*a = 1;
	if(b)
		*b = 0;
	if(div)
		*div = 1;
}


bool I2S::initParallelInputMode(const int *pinMap, long sampleRate, const int bitCount, int wordSelect, int baseClock)
{
	volatile i2s_dev_t &i2s = *i2sDevices[i2sIndex];
	//route peripherals
	const int deviceBaseIndex[] = {I2S0I_DATA_IN0_IDX, I2S1I_DATA_IN0_IDX};
	const int deviceClockIndex[] = {I2S0I_BCK_IN_IDX, I2S1I_BCK_IN_IDX};
	const int deviceWordSelectIndex[] = {I2S0I_WS_IN_IDX, I2S1I_WS_IN_IDX};
	const periph_module_t deviceModule[] = {PERIPH_I2S0_MODULE, PERIPH_I2S1_MODULE};
	//works only since indices of the pads are sequential
	for (int i = 0; i < bitCount; i++)
		if (pinMap[i] > -1)
		{
			PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[pinMap[i]], PIN_FUNC_GPIO);
			gpio_set_direction((gpio_num_t)pinMap[i], (gpio_mode_t)GPIO_MODE_DEF_INPUT);
			gpio_matrix_in(pinMap[i], deviceBaseIndex[i2sIndex] + i, false);
		}
	if (baseClock > -1)
		gpio_matrix_in(baseClock, deviceClockIndex[i2sIndex], false);
	if (wordSelect > -1)
		gpio_matrix_in(wordSelect, deviceWordSelectIndex[i2sIndex], false);

	//enable I2S peripheral
	periph_module_enable(deviceModule[i2sIndex]);

	//reset i2s
	i2s.conf.rx_reset = 1;
	i2s.conf.rx_reset = 0;
	i2s.conf.tx_reset = 1;
	i2s.conf.tx_reset = 0;

	resetFIFO();
	resetDMA();

	//parallel mode
	i2s.conf2.val = 0;
	i2s.conf2.lcd_en = 1;
	//from technical datasheet figure 64
	//i2s.conf2.lcd_tx_sdx2_en = 1;
	//i2s.conf2.lcd_tx_wrx2_en = 1;

	i2s.sample_rate_conf.val = 0;
	i2s.sample_rate_conf.rx_bits_mod = 16;

	//maximum rate
	i2s.clkm_conf.val = 0;
	i2s.clkm_conf.clka_en = 0;
	i2s.clkm_conf.clkm_div_num = 6; //3//80000000L / sampleRate;
	i2s.clkm_conf.clkm_div_a = 6;   // 0;
	i2s.clkm_conf.clkm_div_b = 1;   // 0;
	i2s.sample_rate_conf.rx_bck_div_num = 2;

	i2s.fifo_conf.val = 0;
	i2s.fifo_conf.rx_fifo_mod_force_en = 1;
	i2s.fifo_conf.rx_fifo_mod = 1; //byte packing 0A0B_0B0C = 0, 0A0B_0C0D = 1, 0A00_0B00 = 3,
	i2s.fifo_conf.rx_data_num = 32;
	i2s.fifo_conf.dscr_en = 1; //fifo will use dma

	i2s.conf1.val = 0;
	i2s.conf1.tx_stop_en = 1;
	i2s.conf1.tx_pcm_bypass = 1;

	i2s.conf_chan.val = 0;
	i2s.conf_chan.rx_chan_mod = 0;

	//high or low (stereo word order)
	i2s.conf.rx_right_first = 1;

	i2s.timing.val = 0;

	//clear serial mode flags
	i2s.conf.rx_msb_right = 0;
	i2s.conf.rx_msb_shift = 0;
	i2s.conf.rx_mono = 0;
	i2s.conf.rx_short_sync = 0;

	//allocate disabled i2s interrupt
	const int interruptSource[] = {ETS_I2S0_INTR_SOURCE, ETS_I2S1_INTR_SOURCE};
	if(useInterrupt())
		esp_intr_alloc(interruptSource[i2sIndex], ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM, &interruptStatic, this, reinterpret_cast<intr_handle_t*>(&interruptHandle));
	return true;
}
bool I2S::initParallelOutputMode(const int *pinMap, long sampleRate, const int bitCount, int wordSelect, int baseClock)
{
	return initParallelOutputMode(pinMap, 0, bitCount, sampleRate, bitCount, wordSelect, baseClock);
}

bool I2S::initParallelOutputMode(const int *pinMap, const int *pinMapBit, const int pinCount, long sampleRate, const int bitCount, int wordSelect, int baseClock)
{
	volatile i2s_dev_t &i2s = *i2sDevices[i2sIndex];
	//route peripherals
	//in parallel mode only upper 16 bits are interesting in this case
	const int deviceBaseIndex[] = {I2S0O_DATA_OUT0_IDX, I2S1O_DATA_OUT0_IDX};
	const int deviceClockIndex[] = {I2S0O_BCK_OUT_IDX, I2S1O_BCK_OUT_IDX};
	const int deviceWordSelectIndex[] = {I2S0O_WS_OUT_IDX, I2S1O_WS_OUT_IDX};
	const periph_module_t deviceModule[] = {PERIPH_I2S0_MODULE, PERIPH_I2S1_MODULE};
	//works only since indices of the pads are sequential
	for (int i = 0; i < pinCount; i++)
		if (pinMap[i] > -1)
		{
			PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[pinMap[i]], PIN_FUNC_GPIO);
			gpio_set_direction((gpio_num_t)pinMap[i], (gpio_mode_t)GPIO_MODE_DEF_OUTPUT);
			//rtc_gpio_set_drive_capability((gpio_num_t)pinMap[i], (gpio_drive_cap_t)GPIO_DRIVE_CAP_3 );
			if(pinMapBit)
			{
				if(i2sIndex == 1)
				{
					if(bitCount == 16)
						gpio_matrix_out(pinMap[i], deviceBaseIndex[i2sIndex] + pinMapBit[i] + 8, false, false);
					else
						gpio_matrix_out(pinMap[i], deviceBaseIndex[i2sIndex] + pinMapBit[i], false, false);
				}
				else
				{
					//there is something odd going on here in the two different I2S
					//the configuration seems to differ. Use i2s1 for high frequencies.
					gpio_matrix_out(pinMap[i], deviceBaseIndex[i2sIndex] + pinMapBit[i] + 24 - bitCount, false, false);
				}
			}
			else
			{ 
				if(i2sIndex == 1)
				{
					if(bitCount == 16)
						gpio_matrix_out(pinMap[i], deviceBaseIndex[i2sIndex] + i + 8, false, false);
					else
						gpio_matrix_out(pinMap[i], deviceBaseIndex[i2sIndex] + i, false, false);
				}
				else
				{
					//there is something odd going on here in the two different I2S
					//the configuration seems to differ. Use i2s1 for high frequencies.
					gpio_matrix_out(pinMap[i], deviceBaseIndex[i2sIndex] + i + 24 - bitCount, false, false);
				}
			}
		}
	if (baseClock > -1)
		gpio_matrix_out(baseClock, deviceClockIndex[i2sIndex], false, false);
	if (wordSelect > -1)
		gpio_matrix_out(wordSelect, deviceWordSelectIndex[i2sIndex], false, false);

		//enable I2S peripheral
	periph_module_enable(deviceModule[i2sIndex]);

	//reset i2s
	i2s.conf.tx_reset = 1;
	i2s.conf.tx_reset = 0;
	i2s.conf.rx_reset = 1;
	i2s.conf.rx_reset = 0;

	resetFIFO();
	resetDMA();

	//parallel mode
	i2s.conf2.val = 0;
	i2s.conf2.lcd_en = 1;
	//from technical datasheet figure 64
	i2s.conf2.lcd_tx_wrx2_en = 1;
	i2s.conf2.lcd_tx_sdx2_en = 0;

	i2s.sample_rate_conf.val = 0;
	i2s.sample_rate_conf.tx_bits_mod = bitCount;
	//clock setup
	int clockN = 2, clockA = 1, clockB = 0, clockDiv = 1;
	if(sampleRate == 0)
		getClockSetting(&sampleRate, &clockN, &clockA, &clockB, &clockDiv);
	if(sampleRate > 0)
	{
		//xtal is 40M
		//chip revision 0
		//fxtal * (sdm2 + 4) / (2 * (odir + 2))
		//chip revision 1
		//fxtal * (sdm2 + (sdm1 / 256) + (sdm0 / 65536) + 4) / (2 * (odir + 2))
		//fxtal * (sdm2 + (sdm1 / 256) + (sdm0 / 65536) + 4) needs to be btween 350M and 500M
		//rtc_clk_apll_enable(enable, sdm0, sdm1, sdm2, odir);
		//                           0-255 0-255  0-63  0-31
		//sdm seems to be simply a fixpoint number with 16bits fractional part
		//freq = 40000000L * (4 + sdm) / (2 * (odir + 2))
		//sdm = freq / (20000000L / (odir + 2)) - 4;
		long freq = sampleRate * 2 * (bitCount / 8);
		int sdm, sdmn;
		int odir = -1;
		do
		{	
			odir++;
			sdm = long((double(freq) / (20000000. / (odir + 2))) * 0x10000) - 0x40000;
			sdmn = long((double(freq) / (20000000. / (odir + 2 + 1))) * 0x10000) - 0x40000;
		}while(sdm < 0x8c0ecL && odir < 31 && sdmn < 0xA1fff); //0xA7fffL doesn't work on all mcus 
		//DEBUG_PRINTLN(sdm & 255);
		//DEBUG_PRINTLN((sdm >> 8) & 255);
		//DEBUG_PRINTLN(sdm >> 16);
		//DEBUG_PRINTLN(odir);
		//sdm = 0xA1fff;
		//odir = 0;
		if(sdm > 0xA1fff) sdm = 0xA1fff;
		//rtc_clk_apll_enable(true, sdm & 255, (sdm >> 8) & 255, sdm >> 16, odir);
		rtc_clk_apll_enable(true);
		rtc_clk_apll_coeff_set(odir, sdm & 255, (sdm >> 8) & 255, sdm >> 16);
	}

	i2s.clkm_conf.val = 0;
	i2s.clkm_conf.clka_en = sampleRate > 0 ? 1 : 0;
	i2s.clkm_conf.clkm_div_num = clockN;
	i2s.clkm_conf.clkm_div_a = clockA;
	i2s.clkm_conf.clkm_div_b = clockB;
	i2s.sample_rate_conf.tx_bck_div_num = clockDiv;

	i2s.fifo_conf.val = 0;
	i2s.fifo_conf.tx_fifo_mod_force_en = 1;
	i2s.fifo_conf.tx_fifo_mod = 1;  //byte packing 0A0B_0B0C = 0, 0A0B_0C0D = 1, 0A00_0B00 = 3,
	i2s.fifo_conf.tx_data_num = 32; //fifo length
	i2s.fifo_conf.dscr_en = 1;		//fifo will use dma

	i2s.conf1.val = 0;
	i2s.conf1.tx_stop_en = 0;
	i2s.conf1.tx_pcm_bypass = 1;

	i2s.conf_chan.val = 0;
	i2s.conf_chan.tx_chan_mod = 1;

	//high or low (stereo word order)
	i2s.conf.tx_right_first = 1;

	i2s.timing.val = 0;

	//clear serial mode flags
	i2s.conf.tx_msb_right = 0;
	i2s.conf.tx_msb_shift = 0;
	i2s.conf.tx_mono = 0;
	i2s.conf.tx_short_sync = 0;

	//allocate disabled i2s interrupt
	const int interruptSource[] = {ETS_I2S0_INTR_SOURCE, ETS_I2S1_INTR_SOURCE};
	if(useInterrupt())
		esp_intr_alloc(interruptSource[i2sIndex], ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM, &interruptStatic, this, reinterpret_cast<intr_handle_t*>(&interruptHandle));
	return true;
}

void I2S::enableDAC(int selectedDACs)
{
	volatile i2s_dev_t &i2s = *i2sDevices[i2sIndex];
	i2s.conf2.lcd_en = 1;
	i2s.conf.tx_right_first = 1;
	i2s.conf2.camera_en = 0;
	dac_i2s_enable();
	dac_output_enable(selectedDACs==1?DAC_CHANNEL_1:DAC_CHANNEL_2);
}

void I2S::setAPLLClock(long sampleRate, int bitCount)
{
	//xtal is 40M
	//chip revision 0
	//fxtal * (sdm2 + 4) / (2 * (odir + 2))
	//chip revision 1
	//fxtal * (sdm2 + (sdm1 / 256) + (sdm0 / 65536) + 4) / (2 * (odir + 2))
	//fxtal * (sdm2 + (sdm1 / 256) + (sdm0 / 65536) + 4) needs to be btween 350M and 500M
	//rtc_clk_apll_enable(enable, sdm0, sdm1, sdm2, odir);
	//                           0-255 0-255  0-63  0-31
	//sdm seems to be simply a fixpoint number with 16bits fractional part
	//freq = 40000000L * (4 + sdm) / (2 * (odir + 2))
	//sdm = freq / (20000000L / (odir + 2)) - 4;
	long freq = sampleRate * 2 * (bitCount / 8);
	int sdm, sdmn;
	int odir = -1;
	do
	{	
		odir++;
		sdm = long((double(freq) / (20000000. / (odir + 2))) * 0x10000) - 0x40000;
		sdmn = long((double(freq) / (20000000. / (odir + 2 + 1))) * 0x10000) - 0x40000;
	}while(sdm < 0x8c0ecL && odir < 31 && sdmn < 0xA1fff); //0xA7fffL doesn't work on all mcus 
	//DEBUG_PRINTLN(sdm & 255);
	//DEBUG_PRINTLN((sdm >> 8) & 255);
	//DEBUG_PRINTLN(sdm >> 16);
	//DEBUG_PRINTLN(odir);
	//sdm = 0xA1fff;
	//odir = 0;
	if(sdm > 0xA1fff) sdm = 0xA1fff;
	//rtc_clk_apll_enable(true, sdm & 255, (sdm >> 8) & 255, sdm >> 16, odir);
	rtc_clk_apll_enable(true);
	rtc_clk_apll_coeff_set(odir, sdm & 255, (sdm >> 8) & 255, sdm >> 16);
}

void I2S::setClock(long sampleRate, int bitCount, bool useAPLL)
{
	volatile i2s_dev_t &i2s = *i2sDevices[i2sIndex];
	int factor = 1;
	if(bitCount > 8)
		factor = 2;
	else if(bitCount > 16)
		factor = 4;
	i2s.clkm_conf.val = 0;
	i2s.sample_rate_conf.val = 0;
	i2s.sample_rate_conf.tx_bits_mod = bitCount;

	if(useAPLL)
	{
		setAPLLClock(sampleRate, bitCount);
		i2s.clkm_conf.clka_en = 1;
		i2s.clkm_conf.clkm_div_num = 2; //clockN;
		i2s.clkm_conf.clkm_div_a = 1;   //clockA;
		i2s.clkm_conf.clkm_div_b = 0;   //clockB;
		i2s.sample_rate_conf.tx_bck_div_num = 1;
	}
	else
	{
		i2s.clkm_conf.clkm_div_num = 40000000L / (sampleRate * factor); //clockN;
		i2s.clkm_conf.clkm_div_a = 1;   //clockA;
		i2s.clkm_conf.clkm_div_b = 0;   //clockB;
		i2s.sample_rate_conf.tx_bck_div_num = 1;
	}
}

bool I2S::initSerialOutputMode(int dataPin, const int bitCount, int wordSelect, int baseClock, long sampleRate)
{
	volatile i2s_dev_t &i2s = *i2sDevices[i2sIndex];
	//route peripherals
	//in parallel mode only upper 16 bits are interesting in this case
	const int deviceBaseIndex[] = {I2S0O_DATA_OUT0_IDX, I2S1O_DATA_OUT0_IDX};
	const int deviceClockIndex[] = {I2S0O_BCK_OUT_IDX, I2S1O_BCK_OUT_IDX};
	const int deviceWordSelectIndex[] = {I2S0O_WS_OUT_IDX, I2S1O_WS_OUT_IDX};
	const periph_module_t deviceModule[] = {PERIPH_I2S0_MODULE, PERIPH_I2S1_MODULE};
	
	//works only since indices of the pads are sequential
	//rtc_gpio_set_drive_capability((gpio_num_t)dataPin, (gpio_drive_cap_t)GPIO_DRIVE_CAP_3 );
	
	//serial output on 23, input on 15
	gpio_matrix_out(dataPin, deviceBaseIndex[i2sIndex] + 23, false, false);

	if (baseClock > -1)
		gpio_matrix_out(baseClock, deviceClockIndex[i2sIndex], false, false);
	if (wordSelect > -1)
		gpio_matrix_out(wordSelect, deviceWordSelectIndex[i2sIndex], false, false);

	//reset i2s
	i2s.conf.tx_reset = 1;
	i2s.conf.tx_reset = 0;
	i2s.conf.rx_reset = 1;
	i2s.conf.rx_reset = 0;

	resetFIFO();
	resetDMA();

	//parallel mode
	i2s.conf2.val = 0;
	i2s.conf2.lcd_en = 0;


	i2s.fifo_conf.val = 0;
	i2s.fifo_conf.tx_fifo_mod_force_en = 1;
	i2s.fifo_conf.tx_fifo_mod = 2;  //byte packing 
	i2s.fifo_conf.tx_data_num = 32; //fifo length
	i2s.fifo_conf.dscr_en = 1;		//fifo will use dma

	i2s.conf_chan.val = 0;
	i2s.conf_chan.tx_chan_mod = 0;

	i2s.conf1.val = 0;
	i2s.conf1.tx_stop_en = 0;
	i2s.conf1.tx_pcm_bypass = 1;

	i2s.timing.val = 0;

	//high or low (stereo word order)
	i2s.conf.tx_right_first = 1;
	//clear serial mode flags
	i2s.conf.tx_msb_right = 1;
	i2s.conf.tx_msb_shift = 0;
	i2s.conf.tx_mono = 0;
	i2s.conf.tx_short_sync = 0;

	if(sampleRate != -1)
	{
		setClock(sampleRate*bitCount, bitCount, true);
		i2s.sample_rate_conf.tx_bits_mod = 32; //this should be revised to a formula based on bitCount
		i2s.conf_chan.tx_chan_mod = 0;
	}

	//allocate disabled i2s interrupt
	const int interruptSource[] = {ETS_I2S0_INTR_SOURCE, ETS_I2S1_INTR_SOURCE};
	if(useInterrupt())
		esp_intr_alloc(interruptSource[i2sIndex], ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM, &interruptStatic, this, reinterpret_cast<intr_handle_t*>(&interruptHandle));
	return true;
}

/// simple ringbuffer of blocks of size bytes each
void I2S::allocateDMABuffers(int count, int bytes)
{
	dmaBufferDescriptorCount = count;
	dmaBufferDescriptors = DMABufferDescriptor::allocateDescriptors(count);
	for (int i = 0; i < dmaBufferDescriptorCount; i++)
	{
		dmaBufferDescriptors[i].setBuffer(DMABufferDescriptor::allocateBuffer(bytes, true), bytes);
		if (i)
			dmaBufferDescriptors[i - 1].next(dmaBufferDescriptors[i]);
	}
	dmaBufferDescriptors[dmaBufferDescriptorCount - 1].next(dmaBufferDescriptors[0]);
}

void I2S::deleteDMABuffers()
{
	if (!dmaBufferDescriptors)
		return;
	for (int i = 0; i < dmaBufferDescriptorCount; i++)
		free(dmaBufferDescriptors[i].buffer());
	free(dmaBufferDescriptors);
	dmaBufferDescriptors = 0;
	dmaBufferDescriptorCount = 0;
}

void I2S::stop()
{
	stopSignal = true;
	while (stopSignal)
		;
}

#endif
