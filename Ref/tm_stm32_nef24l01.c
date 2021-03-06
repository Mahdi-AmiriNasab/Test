/**	
 * |----------------------------------------------------------------------
 * | Copyright (c) 2016 Tilen Majerle
 * |  
 * | Permission is hereby granted, free of charge, to any person
 * | obtaining a copy of this software and associated documentation
 * | files (the "Software"), to deal in the Software without restriction,
 * | including without limitation the rights to use, copy, modify, merge,
 * | publish, distribute, sublicense, and/or sell copies of the Software, 
 * | and to permit persons to whom the Software is furnished to do so, 
 * | subject to the following conditions:
 * | 
 * | The above copyright notice and this permission notice shall be
 * | included in all copies or substantial portions of the Software.
 * | 
 * | THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * | EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * | OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * | AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * | HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * | WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * | FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * | OTHER DEALINGS IN THE SOFTWARE.
 * |----------------------------------------------------------------------
 */
#include "tm_stm32_nrf24l01.h"
#include "usbd_cdc_if.h"
#include "stdio.h"
#define 	SPI_DLY 	1


typedef struct {
	uint8_t PayloadSize;				//Payload size
	uint8_t Channel;					//Channel selected
	TM_NRF24L01_OutputPower_t OutPwr;	//Output power (enumerated)
	TM_NRF24L01_DataRate_t DataRate;	//Data rate    (enumerated)
} TM_NRF24L01_t;

/* Private functions */
void TM_NRF24L01_InitPins(void);
void TM_NRF24L01_WriteBit(uint8_t reg, uint8_t bit, uint8_t value);
uint8_t TM_NRF24L01_ReadBit(uint8_t reg, uint8_t bit);
uint8_t TM_NRF24L01_ReadRegister(uint8_t reg);
void TM_NRF24L01_ReadRegisterMulti(uint8_t reg, uint8_t* data, uint8_t count);
void TM_NRF24L01_WriteRegisterMulti(uint8_t reg, uint8_t *data, uint8_t count);
void TM_NRF24L01_SoftwareReset(void);
uint8_t TM_NRF24L01_RxFifoEmpty(void);

/* NRF structure */
static TM_NRF24L01_t TM_NRF24L01_Struct;

//void TM_NRF24L01_InitPins(void) {
//	/* Init pins */
//	/* CNS pin */
//	TM_GPIO_Init(NRF24L01_CSN_PORT, NRF24L01_CSN_PIN, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);
//	
//	/* CE pin */
//	TM_GPIO_Init(NRF24L01_CE_PORT, NRF24L01_CE_PIN, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);
//	
//	/* CSN high = disable SPI */
//	NRF24L01_CSN_HIGH;
//	
//	/* CE low = disable TX/RX */
//	NRF24L01_CE_LOW;
//}



uint8_t TM_NRF24L01_Init(uint8_t channel, uint8_t payload_size) {
	/* Initialize CE and CSN pins */
	//TM_NRF24L01_InitPins();
	/* Initialize SPI */
	//TM_SPI_Init(NRF24L01_SPI, NRF24L01_SPI_PINS);
	
	/* Max payload is 32bytes */
	if (payload_size > 32)
		payload_size = 32;
	
	/* Fill structure */
	TM_NRF24L01_Struct.Channel = !channel; /* Set channel to some different value for TM_NRF24L01_SetChannel() function to prevent rewrite the channel */
	TM_NRF24L01_Struct.PayloadSize = payload_size;
	TM_NRF24L01_Struct.OutPwr = TM_NRF24L01_OutputPower_M6dBm; //chaned 0 to 6
	TM_NRF24L01_Struct.DataRate = TM_NRF24L01_DataRate_1M;     //changed 250 to 1M
	
	NRF24L01_CE_LOW;
	HAL_Delay(1);
	NRF24L01_CSN_HIGH;
	HAL_Delay(1);
	
	HAL_Delay(50); // wait for a while to boot the device
	
	/* Reset nRF24L01+ to power on registers values */
	//TM_NRF24L01_SoftwareReset();
	
	/* Config register */
	 // Reset NRF_CONFIG and enable 16-bit CRC.
	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG,0x0C/* NRF24L01_CONFIG*/);
	
	// Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make testing a little easier
  // WARNING: If this is ever lowered, either 250KBS mode with AA is broken or maximum packet
  // sizes must never be used. See documentation for a more complete explanation.
	//write_register(SETUP_RETR,(delay&0xf)<<ARD | (count&0xf)<<ARC); added form the reference
	TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_RETR, 	0x5f);	
	
	/* Set RF settings (2mbps, output power)(data rate . output power) */
	TM_NRF24L01_SetRF(TM_NRF24L01_Struct.DataRate, TM_NRF24L01_Struct.OutPwr);

	/*According to the reference*/
	// Disable dynamic payloads, to match dynamic_payloads_enabled setting - Reset value is 0
	NRF24L01_CSN_LOW;
	HAL_SPI_Transmit(&NRF24L01_SPI ,(uint8_t *)NRF24L01_ACTIVATE_MASK ,1 ,100);
	HAL_Delay(1);
	HAL_SPI_Transmit(&NRF24L01_SPI ,(uint8_t *)0x73 ,1 ,100);
	HAL_Delay(1);
	NRF24L01_CSN_HIGH;
	TM_NRF24L01_WriteRegister(NRF24L01_REG_FEATURE, 0);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_DYNPD, 0); 
	
	/*According to the reference*/
	// Reset current status
  // Notice reset and flush is the last thing we do
	TM_NRF24L01_WriteRegister(NRF24L01_REG_STATUS, 		NRF24L01_REG_DEFAULT_VAL_STATUS);
	
	/* Channel select */
	TM_NRF24L01_SetChannel(channel);
	
	// Flush buffers
	NRF24L01_FLUSH_TX;
	NRF24L01_FLUSH_RX;
	
	/* Set pipeline to max possible 32 bytes */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P0, TM_NRF24L01_Struct.PayloadSize); // Auto-ACK pipe
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P1, TM_NRF24L01_Struct.PayloadSize); // Data payload pipe
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P2, TM_NRF24L01_Struct.PayloadSize);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P3, TM_NRF24L01_Struct.PayloadSize);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P4, TM_NRF24L01_Struct.PayloadSize);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P5, TM_NRF24L01_Struct.PayloadSize);
	
	/* Enable auto-acknowledgment for all pipes */
	//TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_AA, 0x3F);
	
	/* Enable RX addresses */
	//TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_RXADDR, 0x3F);

	/* Auto retransmit delay: 1000 (4x250) us and Up to 15 retransmit trials */
	//TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_RETR, 0x4F);
	
	/* Dynamic length configurations: No dynamic length */
	//TM_NRF24L01_WriteRegister(NRF24L01_REG_DYNPD, (0 << NRF24L01_DPL_P0) | (0 << NRF24L01_DPL_P1) | (0 << NRF24L01_DPL_P2) | (0 << NRF24L01_DPL_P3) | (0 << NRF24L01_DPL_P4) | (0 << NRF24L01_DPL_P5));
	
	/* Clear FIFOs */
	NRF24L01_FLUSH_TX;
	NRF24L01_FLUSH_RX;
	
	
	/* Clear interrupts */
	//TM_NRF24L01_Clear_Interrupts();
	
	/* Go to PTX mode */
	TM_NRF24L01_PowerUpTx();
	
	/* Return OK */
	return 1;
}

void TM_NRF24L01_SetMyAddress(uint8_t *adr) {
	NRF24L01_CE_LOW;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, adr, 5);
	NRF24L01_CE_HIGH;
}

void TM_NRF24L01_SetTxAddress(uint8_t *adr) {
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, adr, 5);
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_TX_ADDR, adr, 5);
}

void TM_NRF24L01_WriteBit(uint8_t reg, uint8_t bit, uint8_t value) {
	uint8_t tmp;
	/* Read register */
	tmp = TM_NRF24L01_ReadRegister(reg);
	/* Make operation */
	if (value) {
		tmp |= 1 << bit;
	} else {
		tmp &= ~(1 << bit);
	}
	/* Write back */
	TM_NRF24L01_WriteRegister(reg, tmp);
}

uint8_t TM_NRF24L01_ReadBit(uint8_t reg, uint8_t bit) {
	uint8_t tmp;
	tmp = TM_NRF24L01_ReadRegister(reg);
	if (!NRF24L01_CHECK_BIT(tmp, bit)) {
		return 0;
	}
	return 1;
}

uint8_t TM_NRF24L01_ReadRegister(uint8_t reg) {
  uint8_t value;
	NRF24L01_CSN_LOW;
	#ifdef CDC_LOG
	if(HAL_SPI_Transmit(&NRF24L01_SPI ,(uint8_t *)NRF24L01_READ_REGISTER_MASK(reg) ,1 ,100) != HAL_OK)
		CDC_Transmit_FS((uint8_t *)"Read command ERRORrd\n" ,21);
	if(HAL_SPI_Receive(&NRF24L01_SPI, &value, 1 ,100)!= HAL_OK)
		CDC_Transmit_FS((uint8_t *)"Read register ERRORwr0\n" ,23);
	#else
	HAL_SPI_Transmit(&NRF24L01_SPI ,(uint8_t *)NRF24L01_READ_REGISTER_MASK(reg) ,1 ,100);
	HAL_Delay(SPI_DLY);
	HAL_SPI_Receive(&NRF24L01_SPI, &value, 1 ,100);
	HAL_Delay(SPI_DLY);
	#endif
	NRF24L01_CSN_HIGH;
	return value;
}

void TM_NRF24L01_ReadRegisterMulti(uint8_t reg, uint8_t* data, uint8_t count) {
	NRF24L01_CSN_LOW;
	#ifdef CDC_LOG
	//TM_SPI_Send(NRF24L01_SPI, NRF24L01_READ_REGISTER_MASK(reg));
	if(HAL_SPI_Transmit(&NRF24L01_SPI ,(uint8_t *)NRF24L01_READ_REGISTER_MASK(reg) ,1 ,100) != HAL_OK)
		CDC_Transmit_FS((uint8_t *)"Read command ERRORrd\n" ,21);
	//TM_SPI_ReadMulti(NRF24L01_SPI, data, NRF24L01_NOP_MASK, count);
	if(HAL_SPI_Receive(&NRF24L01_SPI, data, count,100) != HAL_OK)
		CDC_Transmit_FS((uint8_t *)"Read command ERRORrd\n" ,21);
	#else
	HAL_SPI_Transmit(&NRF24L01_SPI ,(uint8_t *)NRF24L01_READ_REGISTER_MASK(reg) ,1 ,100);
	HAL_Delay(SPI_DLY);
	HAL_SPI_Receive(&NRF24L01_SPI, data, count,100);
	HAL_Delay(SPI_DLY);
	#endif
	NRF24L01_CSN_HIGH;
}

void TM_NRF24L01_WriteRegister(uint8_t reg, uint8_t value) {
	NRF24L01_CSN_LOW;
	#ifdef CDC_LOG
  //TM_SPI_Send(NRF24L01_SPI, NRF24L01_WRITE_REGISTER_MASK(reg));
	if(HAL_SPI_Transmit(&NRF24L01_SPI ,(uint8_t *)NRF24L01_WRITE_REGISTER_MASK(reg) ,1 ,100) != HAL_OK)
		CDC_Transmit_FS((uint8_t *)"Write command ERRORwr1\n" ,23);
	//TM_SPI_Send(NRF24L01_SPI, value);
	if(HAL_SPI_Transmit(&NRF24L01_SPI ,&value ,1 ,100) != HAL_OK)
		CDC_Transmit_FS((uint8_t *)"Write command ERRORwr2\n" ,23);
	#else
	HAL_SPI_Transmit(&NRF24L01_SPI ,(uint8_t *)NRF24L01_WRITE_REGISTER_MASK(reg) ,1 ,100);
	HAL_Delay(SPI_DLY);
	HAL_SPI_Transmit(&NRF24L01_SPI ,&value ,1 ,100);
	HAL_Delay(SPI_DLY);
	#endif
	
	NRF24L01_CSN_HIGH;
}

void TM_NRF24L01_WriteRegisterMulti(uint8_t reg, uint8_t *data, uint8_t count) {
	NRF24L01_CSN_LOW;
	#ifdef CDC_LOG
	//TM_SPI_Send(NRF24L01_SPI, NRF24L01_WRITE_REGISTER_MASK(reg));
	if(HAL_SPI_Transmit(&NRF24L01_SPI ,(uint8_t *)NRF24L01_WRITE_REGISTER_MASK(reg) ,1 ,100) != HAL_OK)
		CDC_Transmit_FS((uint8_t *)"Read command ERRORrd\n" ,21);
	//TM_SPI_WriteMulti(NRF24L01_SPI, data, count);
	if(HAL_SPI_Transmit(&NRF24L01_SPI ,data ,count ,100) != HAL_OK)
		CDC_Transmit_FS((uint8_t *)"Read command ERRORrd\n" ,21);
	
	#else
	HAL_SPI_Transmit(&NRF24L01_SPI ,(uint8_t *)NRF24L01_WRITE_REGISTER_MASK(reg) ,1 ,100);
	HAL_Delay(10);
	HAL_SPI_Transmit(&NRF24L01_SPI ,data ,count ,100);
	HAL_Delay(10);
	#endif
	NRF24L01_CSN_HIGH;
}

void TM_NRF24L01_PowerUpTx(void) {
	TM_NRF24L01_Clear_Interrupts();
	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG | (0 << NRF24L01_PRIM_RX) | (1 << NRF24L01_PWR_UP));
}

void TM_NRF24L01_PowerUpRx(void) {
	/* Disable RX/TX mode */
	NRF24L01_CE_LOW;
	/* Clear RX buffer */
	NRF24L01_FLUSH_RX;
	/* Clear interrupts */
	TM_NRF24L01_Clear_Interrupts();
	/* Setup RX mode */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, NRF24L01_CONFIG | 1 << NRF24L01_PWR_UP | 1 << NRF24L01_PRIM_RX);
	/* Start listening */
	NRF24L01_CE_HIGH;
}

void TM_NRF24L01_PowerDown(void) {
	NRF24L01_CE_LOW;
	TM_NRF24L01_WriteBit(NRF24L01_REG_CONFIG, NRF24L01_PWR_UP, 0);
}

void TM_NRF24L01_Transmit(uint8_t *data) {
	uint8_t count = TM_NRF24L01_Struct.PayloadSize;

	/* Chip enable put to low, disable it */
	NRF24L01_CE_LOW;
	
	/* Go to power up tx mode */
	TM_NRF24L01_PowerUpTx();
	
	/* Clear TX FIFO from NRF24L01+ */
	NRF24L01_FLUSH_TX;
	
	/* Send payload to nRF24L01+ */
	NRF24L01_CSN_LOW;
	/* Send write payload command */
	//TM_SPI_Send(NRF24L01_SPI, NRF24L01_W_TX_PAYLOAD_MASK);
	HAL_SPI_Transmit(&NRF24L01_SPI ,(uint8_t *)NRF24L01_W_TX_PAYLOAD_MASK ,1 ,100);	
	/* Fill payload with data*/
	//TM_SPI_WriteMulti(NRF24L01_SPI, data, count);
	HAL_SPI_Transmit(&NRF24L01_SPI ,data ,count ,100);	
	/* Disable SPI */
	NRF24L01_CSN_HIGH;
	
	/* Send data! */
	NRF24L01_CE_HIGH;
	
	HAL_Delay(100);
	/* Leave the send pin	*/
	NRF24L01_CE_LOW;
}

void TM_NRF24L01_GetData(uint8_t* data) {
	/* Pull down chip select */
	NRF24L01_CSN_LOW;
	/* Send read payload command*/
	//TM_SPI_Send(NRF24L01_SPI, NRF24L01_R_RX_PAYLOAD_MASK);
	HAL_SPI_Transmit(&NRF24L01_SPI ,(uint8_t *)NRF24L01_R_RX_PAYLOAD_MASK ,1 ,100);	
	/* Read payload */
	//TM_SPI_SendMulti(NRF24L01_SPI, data, data, TM_NRF24L01_Struct.PayloadSize);
	HAL_SPI_TransmitReceive(&NRF24L01_SPI, data, data, TM_NRF24L01_Struct.PayloadSize ,100);
	/* Pull up chip select */
	NRF24L01_CSN_HIGH;
	
	/* Reset status register, clear RX_DR interrupt flag */
	TM_NRF24L01_WriteRegister(NRF24L01_REG_STATUS, (1 << NRF24L01_RX_DR));
}

uint8_t TM_NRF24L01_DataReady(void) {
	uint8_t status = TM_NRF24L01_GetStatus();
	
	if (NRF24L01_CHECK_BIT(status, NRF24L01_RX_DR)) {
		return 1;
	}
	return !TM_NRF24L01_RxFifoEmpty();
}

uint8_t TM_NRF24L01_RxFifoEmpty(void) {
	uint8_t reg = TM_NRF24L01_ReadRegister(NRF24L01_REG_FIFO_STATUS);
	return NRF24L01_CHECK_BIT(reg, NRF24L01_RX_EMPTY);
}

uint8_t TM_NRF24L01_GetStatus(void) {
	uint8_t status;
	
	NRF24L01_CSN_LOW;
	/* First received byte is always status register */
	//status = TM_SPI_Send(NRF24L01_SPI, NRF24L01_NOP_MASK);
	HAL_SPI_Receive(&NRF24L01_SPI ,&status ,1 ,100);	
	/* Pull up chip select */
	NRF24L01_CSN_HIGH;
	
	return status;
}

TM_NRF24L01_Transmit_Status_t TM_NRF24L01_GetTransmissionStatus(void) {
	uint8_t status = TM_NRF24L01_GetStatus();
	if (NRF24L01_CHECK_BIT(status, NRF24L01_TX_DS)) {
		/* Successfully sent */
		return TM_NRF24L01_Transmit_Status_Ok;
	} else if (NRF24L01_CHECK_BIT(status, NRF24L01_MAX_RT)) {
		/* Message lost */
		return TM_NRF24L01_Transmit_Status_Lost;
	}
	
	/* Still sending */
	return TM_NRF24L01_Transmit_Status_Sending;
}

void TM_NRF24L01_SoftwareReset(void) {
	uint8_t data[5];
	
	TM_NRF24L01_WriteRegister(NRF24L01_REG_CONFIG, 		NRF24L01_REG_DEFAULT_VAL_CONFIG);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_AA,		NRF24L01_REG_DEFAULT_VAL_EN_AA);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_EN_RXADDR, 	NRF24L01_REG_DEFAULT_VAL_EN_RXADDR);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_AW, 	NRF24L01_REG_DEFAULT_VAL_SETUP_AW);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_SETUP_RETR, 	NRF24L01_REG_DEFAULT_VAL_SETUP_RETR);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_CH, 		NRF24L01_REG_DEFAULT_VAL_RF_CH);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_SETUP, 	NRF24L01_REG_DEFAULT_VAL_RF_SETUP);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_STATUS, 		NRF24L01_REG_DEFAULT_VAL_STATUS);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_OBSERVE_TX, 	NRF24L01_REG_DEFAULT_VAL_OBSERVE_TX);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RPD, 		NRF24L01_REG_DEFAULT_VAL_RPD);
	
	//P0
	data[0] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P0_4;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P0, data, 5);
	
	//P1
	data[0] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P1_4;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_RX_ADDR_P1, data, 5);
	
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P2, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P2);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P3, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P3);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P4, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P4);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_ADDR_P5, 	NRF24L01_REG_DEFAULT_VAL_RX_ADDR_P5);
	
	//TX
	data[0] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_0;
	data[1] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_1;
	data[2] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_2;
	data[3] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_3;
	data[4] = NRF24L01_REG_DEFAULT_VAL_TX_ADDR_4;
	TM_NRF24L01_WriteRegisterMulti(NRF24L01_REG_TX_ADDR, data, 5);
	
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P0, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P0);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P1, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P1);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P2, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P2);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P3, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P3);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P4, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P4);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RX_PW_P5, 	NRF24L01_REG_DEFAULT_VAL_RX_PW_P5);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_FIFO_STATUS, NRF24L01_REG_DEFAULT_VAL_FIFO_STATUS);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_DYNPD, 		NRF24L01_REG_DEFAULT_VAL_DYNPD);
	TM_NRF24L01_WriteRegister(NRF24L01_REG_FEATURE, 	NRF24L01_REG_DEFAULT_VAL_FEATURE);
}

uint8_t TM_NRF24L01_GetRetransmissionsCount(void) {
	/* Low 4 bits */
	return TM_NRF24L01_ReadRegister(NRF24L01_REG_OBSERVE_TX) & 0x0F;
}

uint8_t TM_NRF24L01_SetChannel(uint8_t channel) {
	if (channel <= 125 && channel != TM_NRF24L01_Struct.Channel) {
		/* Store new channel setting */
		TM_NRF24L01_Struct.Channel = channel;
		/* Write channel */
		TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_CH, channel);
		return HAL_OK;
	}
	else
	{
		TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_CH, 1);
		return HAL_ERROR;
	}
}

void TM_NRF24L01_SetRF(TM_NRF24L01_DataRate_t DataRate, TM_NRF24L01_OutputPower_t OutPwr) {
	uint8_t tmp = 0;
	TM_NRF24L01_Struct.DataRate = DataRate;
	TM_NRF24L01_Struct.OutPwr = OutPwr;
	
	if (DataRate == TM_NRF24L01_DataRate_2M) {
		tmp |= 1 << NRF24L01_RF_DR_HIGH;
	} else if (DataRate == TM_NRF24L01_DataRate_250k) {
		tmp |= 1 << NRF24L01_RF_DR_LOW;
	}
	/* If 1Mbps, all bits set to 0 */
	
	if (OutPwr == TM_NRF24L01_OutputPower_0dBm) {
		tmp |= 3 << NRF24L01_RF_PWR;
	} else if (OutPwr == TM_NRF24L01_OutputPower_M6dBm) {
		tmp |= 2 << NRF24L01_RF_PWR;
	} else if (OutPwr == TM_NRF24L01_OutputPower_M12dBm) {
		tmp |= 1 << NRF24L01_RF_PWR;
	}
	
	TM_NRF24L01_WriteRegister(NRF24L01_REG_RF_SETUP, tmp);
}

uint8_t TM_NRF24L01_Read_Interrupts(TM_NRF24L01_IRQ_t* IRQ) {
	IRQ->Status = TM_NRF24L01_GetStatus();
	return IRQ->Status;
}

void TM_NRF24L01_Clear_Interrupts(void) {
	TM_NRF24L01_WriteRegister(0x07, 0x70);
}






/*my functions*/

/**
 * @brief  Sends single byte over SPI
 * @param  *SPIx: Pointer to SPIx peripheral you will use, where x is between 1 to 6
 * @param  data: 8-bit data size to send over SPI
 * @retval Received byte from slave device
 */

	




