/*
 * (c)COPYRIGHT
 * ALL RIGHT RESERVED
 *
 * FileName : w5100.c
 * Revision History :
 * ----------	-------		------------------------------------------------
 * 	Date			version	  	Description
 * ----------	-------  	------------------------------------------------
 * 01/25/2007	1.1			Bug is Fixed in the Indirect Mode 
 *							: Memory mapping error	
 * ----------	-------		------------------------------------------------
 * 01/08/2008	1.2			Modification of Socket Command Part
 *							: Check if the appropriately performed after writing Sn_CR
 *
 *							Modification of SPI Part
 *							: SPI code changed by adding 'spi.h'.
 *							: Change control type for SPI port from byte to bit.
 * ----------	-------		------------------------------------------------
 * 01/15/2008	1.3			Bug is Fixed in the pppinit() fuction.
 *							: do not clear interrupt value, so fixed.
 *
 *		                   			Modification of ISR
 *                   				: Do not exit ISR, if there is interrupt.
 * ----------	-------		------------------------------------------------
 * 03/21/2008	1.4			Modification of SetMR() function
 *                   				: Use IINCHIP_WRITE() function in Direct or SPI mode.
 * ----------	-------		------------------------------------------------
 */
#include <stdio.h>
#include <string.h>

#include <avr/interrupt.h>
// #include <avr/io.h> 
   
#include "types.h"
#include "socket.h"
#include "w5100.h"



#ifdef __DEF_IINCHIP_PPP__
   #include "md5.h"
#endif


#if (__DEF_IINCHIP_BUS__ == __DEF_IINCHIP_SPI_MODE__)
#include "spi.h"		//+2007113[jhpark]
#endif

static uint8 I_STATUS[MAX_SOCK_NUM];

// Hard wired for a 2K buffer for every channel.


uint8 getISR(uint8 s)
{
	return I_STATUS[s];
}

void putISR(uint8 s, uint8 val)
{
   I_STATUS[s] = val;
}

uint16 getIINCHIP_RxBASE(uint8 s)
{
   return (uint16)(__DEF_IINCHIP_MAP_RXBUF__ | s<<11);
}
uint16 getIINCHIP_TxBASE(uint8 s)
{
   return (uint16)(__DEF_IINCHIP_MAP_TXBUF__ | s<<11);
}

 /**
@brief	This function writes the data into W5100 registers.
*/
void wiz_write_byte(uint16 addr,uint8 data)
{
    IINCHIP_ISR_DISABLE();
    IINCHIP_SpiInit();
    IINCHIP_CSoff();                             // CS=0, SPI start
    IINCHIP_SpiSendData(0xF0);
    IINCHIP_SpiSendData(addr>>8);
    IINCHIP_SpiSendData(addr);
    IINCHIP_SpiSendData(data);
    IINCHIP_CSon();    
    IINCHIP_ISR_ENABLE();   
}


/**
@brief	This function reads the value from W5100 registers.
*/
uint8 wiz_read_byte(uint16 addr)
{
	uint8 data;
	
    IINCHIP_ISR_DISABLE();
    IINCHIP_SpiInit();	
    IINCHIP_CSoff();                             // CS=0, SPI start
    IINCHIP_SpiSendData(0x0F);
    IINCHIP_SpiSendData(addr>>8);
    IINCHIP_SpiSendData(addr);
    IINCHIP_SpiSendData(0);
    data = IINCHIP_SpiRecvData();
    IINCHIP_CSon();                          	// SPI end
    IINCHIP_ISR_ENABLE();
    return data;
}

uint16 wiz_read_word(uint16 addr)
{
    return (wiz_read_byte(addr) << 8) + wiz_read_byte(addr+1);
}

void wiz_write_word(uint16 addr, uint16 data)
{
    wiz_write_byte(addr,data >> 8);
    wiz_write_byte(addr+1,data & 0xff);
}


/**
@brief	This function writes into W5100 memory(Buffer)
*/ 

void wiz_write_buf(uint16 addr,uint8* buf,uint16 len)
{
    IINCHIP_ISR_DISABLE();
    IINCHIP_SpiInit();
    while(len--)
    {
        IINCHIP_CSoff();                             // CS=0, SPI start 
        IINCHIP_SpiSendData(0xF0);
	    IINCHIP_SpiSendData(addr>>8);
	    IINCHIP_SpiSendData(addr++);
        IINCHIP_SpiSendData(*buf++);
        IINCHIP_CSon();                             // CS=0, SPI end 
    }
    IINCHIP_ISR_ENABLE();	   
}


/**
@brief	This function reads into W5100 memory(Buffer)
*/ 
void wiz_read_buf(uint16 addr, uint8* buf,uint16 len)
{
    IINCHIP_ISR_DISABLE();
    IINCHIP_SpiInit();
    while(len--)
    {
        IINCHIP_CSoff();                             // CS=0, SPI start 
        IINCHIP_SpiSendData(0x0F);
	    IINCHIP_SpiSendData(addr>>8);
	    IINCHIP_SpiSendData(addr++);
        IINCHIP_SpiSendData(0);
        *buf++ = IINCHIP_SpiRecvData();
        IINCHIP_CSon();                             // CS=0, SPI end 	   
    }
    IINCHIP_ISR_ENABLE();
}


/**
@brief	Socket interrupt routine
*/ 
#ifdef __DEF_IINCHIP_INT__
ISR(INT4_vect)
{
	uint8 int_val;
	IINCHIP_ISR_DISABLE();
	int_val = wiz_read_byte(IR);
	
	/* +200801[bj] process all of interupt */
   do {
   /*---*/
   
   	if (int_val & IR_CONFLICT)
   	{
   		printf("IP conflict : %.2x\r\n", int_val);
   	}
   	if (int_val & IR_UNREACH)
   	{
   		printf("INT Port Unreachable : %.2x\r\n", int_val);
   		printf("UIPR0 : %d.%d.%d.%d\r\n", wiz_read_byte(UIPR0), wiz_read_byte(UIPR0+1), wiz_read_byte(UIPR0+2), wiz_read_byte(UIPR0+3));
   		printf("UPORT0 : %.2x %.2x\r\n", wiz_read_byte(UPORT0), wiz_read_byte(UPORT0+1));
   	}
   
   	/* +200801[bj] interrupt clear */
   	wiz_write_byte(IR, 0xf0); 
      /*---*/
   
   	if (int_val & IR_SOCK(0))
   	{
   	/* +-200801[bj] save interrupt value*/
   		I_STATUS[0] |= wiz_read_byte(Sn_IR(0)); // can be come to over two times.
   		wiz_write_byte(Sn_IR(0), I_STATUS[0]);
      /*---*/
   	}
   	if (int_val & IR_SOCK(1))
   	{
   	/* +-200801[bj] save interrupt value*/
   		I_STATUS[1] |= wiz_read_byte(Sn_IR(1));
   		wiz_write_byte(Sn_IR(1), I_STATUS[1]);
      /*---*/
   	}
   	if (int_val & IR_SOCK(2))
   	{
   	/* +-200801[bj] save interrupt value*/
   		I_STATUS[2] |= wiz_read_byte(Sn_IR(2));
   		wiz_write_byte(Sn_IR(2), I_STATUS[2]);
      /*---*/
   	}
   	if (int_val & IR_SOCK(3))
   	{
   	/* +-200801[bj] save interrupt value*/
   		I_STATUS[3] |= wiz_read_byte(Sn_IR(3));
   		wiz_write_byte(Sn_IR(3), I_STATUS[3]);
      /*---*/
   	}
   
   	/* +-200801[bj] re-read interrupt value*/
   	int_val = wiz_read_byte(IR);

	/* +200801[bj] if exist, contiue to process */
   } while (int_val != 0x00);
   /*---*/

	IINCHIP_ISR_ENABLE();
}
#endif

/**
@brief	This function is for resetting of the iinchip. Initializes the iinchip to work in whether DIRECT or INDIRECT mode
*/ 
void iinchip_init(void)
{	
	setMR( MR_RST );
}

void sysinit(void)
{
    // 4 channels of 2K each in each direction
	wiz_write_byte(TMSR,0x55); /* Set Tx memory size for each channel */
	wiz_write_byte(RMSR,0x55);	 /* Set Rx memory size for each channel */
}



/**
@brief	get socket TX free buf size

This gives free buffer size of transmit buffer. This is the data size that user can transmit.
User shuold check this value first and control the size of transmitting data
*/
uint16 getSn_TX_FSR(SOCKET s)
{
    uint16 val=0,val1=0;
    do
    {
        val1 = wiz_read_word(Sn_TX_FSR0(s));
        if (val1 != 0)
        {
            val = wiz_read_word(Sn_TX_FSR0(s));
        }
    } while (val != val1);
    return val;
}


/**
@brief	 get socket RX recv buf size

This gives size of received data in receive buffer. 
*/
uint16 getSn_RX_RSR(SOCKET s)
{
    uint16 val=0,val1=0;
    do
    {
        val1 = wiz_read_word(Sn_RX_RSR0(s));
        if(val1 != 0)
        {
            val = wiz_read_word(Sn_RX_RSR0(s));
        }
    } while (val != val1);
    return val;
}


/**
@brief	 This function is being called by send() and sendto() function also. 

This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
register. User should read upper byte first and lower byte later to get proper value.
*/
void send_data_processing(SOCKET s, uint8 *data, uint16 len)
{
	uint16 ptr;
	ptr = wiz_read_word(Sn_TX_WR0(s));
	write_data(s, data, (uint8 *)(ptr), len);
	ptr += len;
	wiz_write_word(Sn_TX_WR0(s),ptr);
	wiz_write_byte(Sn_CR(s),Sn_CR_SEND);
	while( wiz_read_byte(Sn_CR(s)) );
}


/**
@brief	This function is being called by recv() also.

This function read the Rx read pointer register
and after copy the data from receive buffer update the Rx write pointer register.
User should read upper byte first and lower byte later to get proper value.
*/
void recv_data_processing(SOCKET s, uint8 *data, uint16 len)
{
	uint16 ptr;
	ptr = wiz_read_word(Sn_RX_RD0(s));
#ifdef __DEF_IINCHIP_DBG__
	printf("ISR_RX: rd_ptr : %.4x\r\n", ptr);
#endif
	read_data(s, (uint8 *)ptr, data, len); // read data
	ptr += len;
	wiz_write_word(Sn_RX_RD0(s),ptr);
	wiz_write_byte(Sn_CR(s),Sn_CR_RECV);
	while( wiz_read_byte(Sn_CR(s)) );
}


/**
@brief	for copy the data form application buffer to Transmite buffer of the chip.

This function is being used for copy the data form application buffer to Transmite
buffer of the chip. It calculate the actual physical address where one has to write
the data in transmite buffer. Here also take care of the condition while it exceed
the Tx memory uper-bound of socket.
*/
void write_data(SOCKET s, vuint8 * src, vuint8 * dst, uint16 len)
{
	uint16 size = len;
	uint16 dst_mask;
	uint8 * dst_ptr;

	dst_mask = (uint16)dst & IINCHIP_TxMASK;
	dst_ptr = (uint8 *)(getIINCHIP_TxBASE(s) + dst_mask);
	
	if (dst_mask + len > IINCHIP_TxMAX) 
	{
		size = IINCHIP_TxMAX - dst_mask;
		wiz_write_buf((uint16)dst_ptr, (uint8*)src, size);
		src += size;
		size = len - size;
		dst_ptr = (uint8 *)(getIINCHIP_TxBASE(s));
	} 
	wiz_write_buf((uint16)dst_ptr, (uint8*)src, size);
}


/**
@brief	This function is being used for copy the data form Receive buffer of the chip to application buffer.

It calculate the actual physical address where one has to read
the data from Receive buffer. Here also take care of the condition while it exceed
the Rx memory uper-bound of socket.
*/
void read_data(SOCKET s, vuint8 * src, vuint8 * dst, uint16 len)
{
	uint16 size = len;
	uint16 src_mask;
	uint8 * src_ptr;

	src_mask = (uint16)src & IINCHIP_RxMASK;
	src_ptr = (uint8 *)(getIINCHIP_RxBASE(s) + src_mask);
	
	if( (src_mask + len) > IINCHIP_RxMAX ) 
	{
		size = IINCHIP_RxMAX - src_mask;
		wiz_read_buf((uint16)src_ptr, (uint8*)dst,size);
		dst += size;
		size = len - size;
		src_ptr = (uint8 *)(getIINCHIP_RxBASE(s));
	} 
	wiz_read_buf((uint16)src_ptr, (uint8*) dst,size);
}


#ifdef __DEF_IINCHIP_PPP__
#define PPP_OPTION_BUF_LEN 64

uint8 pppinit_in(uint8 * id, uint8 idlen, uint8 * passwd, uint8 passwdlen);


/**
@brief	make PPPoE connection
@return	1 => success to connect, 2 => Auth fail, 3 => timeout, 4 => Auth type not support

*/
uint8 pppinit(uint8 * id, uint8 idlen, uint8 * passwd, uint8 passwdlen)
{
	uint8 ret;
	uint8 isr;
	
	// PHASE0. W5100 PPPoE(ADSL) setup
	// enable pppoe mode
	printf("-- PHASE 0. W5100 PPPoE(ADSL) setup process --\r\n");
	printf("\r\n");
	wiz_write_byte(MR,wiz_read_byte(MR) | MR_PPPOE);

	// open socket in pppoe mode
	isr = wiz_read_byte(Sn_IR(0));// first clear isr(0), W5100 at present time
	wiz_write_byte(Sn_IR(0),isr);
	
	wiz_write_byte(PTIMER,200); // 5sec timeout
	wiz_write_byte(PMAGIC,0x01); // magic number
	wiz_write_byte(Sn_MR(0),Sn_MR_PPPOE);
	wiz_write_byte(Sn_CR(0),Sn_CR_OPEN);
	
	/* +20071122[chungs]:wait to process the command... */
	while( wiz_read_byte(Sn_CR(0)) ) 
		;
	/* ------- */
	
	ret = pppinit_in(id, idlen, passwd, passwdlen);

	// close ppp connection socket
	/* +200801 (hwkim) */
	close(0);
	/* ------- */
	
	return ret;
}


uint8 pppinit_in(uint8 * id, uint8 idlen, uint8 * passwd, uint8 passwdlen)
{
	uint8 loop_idx = 0;
	uint8 isr = 0;
	uint8 buf[PPP_OPTION_BUF_LEN];
	uint16 len;
	uint8 str[PPP_OPTION_BUF_LEN];
	uint8 str_idx,dst_idx;

   // PHASE1. PPPoE Discovery
	// start to connect pppoe connection
	printf("-- PHASE 1. PPPoE Discovery process --");
	printf(" ok\r\n");
	printf("\r\n");
	wiz_write_byte(Sn_CR(0),Sn_CR_PCON);
	/* +20071122[chungs]:wait to process the command... */
	while( wiz_read_byte(Sn_CR(0)) ) 
		;
	/* ------- */
	
	wait_10ms(100);

	loop_idx = 0;
	//check whether PPPoE discovery end or not
	while (!(wiz_read_byte(Sn_IR(0)) & Sn_IR_PNEXT))
	{
		printf(".");
		if (loop_idx++ == 10) // timeout
		{
			printf("timeout before LCP\r\n"); 
			return 3;
		}
		wait_10ms(100);
	}

   /* +200801[bj] clear interrupt value*/
   wiz_write_byte(Sn_IR(0), 0xff);
   /*---*/

   // PHASE2. LCP process
	printf("-- PHASE 2. LCP process --");
		
	// send LCP Request
	{
		// Magic number option
		// option format (type value + length value + data)
	   // write magic number value
		buf[0] = 0x05; // type value
		buf[1] = 0x06; // length value
		buf[2] = 0x01; buf[3] = 0x01; buf[4] = 0x01; buf[5]= 0x01; // data
		// for MRU option, 1492 0x05d4  
		// buf[6] = 0x01; buf[7] = 0x04; buf[8] = 0x05; buf[9] = 0xD4;
	}
	send_data_processing(0, buf, 0x06);
	wiz_write_byte(Sn_CR(0),Sn_CR_PCR); // send request 
	/* +20071122[chungs]:wait to process the command... */
	while( wiz_read_byte(Sn_CR(0)) ) 
		;
	/* ------- */
		
	wait_10ms(100);

	while (!((isr = wiz_read_byte(Sn_IR(0))) & Sn_IR_PNEXT))
	{
		if (isr & Sn_IR_PRECV) // Not support option
		{
   /* +200801[bj] clear interrupt value*/
         wiz_write_byte(Sn_IR(0), Sn_IR_PRECV);
   /*---*/
			len = getSn_RX_RSR(0);
			if ( len > 0 )
			{
				recv_data_processing(0, str, len);
				wiz_write_byte(Sn_CR(0),Sn_CR_RECV);
				/* +20071122[chungs]:wait to process the command... */
				while( wiz_read_byte(Sn_CR(0)) ) 
					;
				/* ------- */
				
				// for debug
				//printf("LCP proc\r\n"); for (i = 0; i < len; i++) printf ("%02x ", str[i]); printf("\r\n");
				// get option length
				len = str[4]; len = ((len & 0x00ff) << 8) + str[5];
				len += 2;
				str_idx = 6; dst_idx = 0; // ppp header is 6 byte, so starts at 6.
				do 
				{
					if ((str[str_idx] == 0x01) || (str[str_idx] == 0x02) || (str[str_idx] == 0x03) || (str[str_idx] == 0x05))
					{
						// skip as length of support option. str_idx+1 is option's length.
						str_idx += str[str_idx+1];
					}
					else
					{
						// not support option , REJECT
						memcpy((uint8 *)(buf+dst_idx), (uint8 *)(str+str_idx), str[str_idx+1]);
						dst_idx += str[str_idx+1]; str_idx += str[str_idx+1];
					}
				} while (str_idx != len);
	   			// for debug
	   			// printf("LCP dst proc\r\n"); for (i = 0; i < dst_idx; i++) printf ("%02x ", dst[i]); printf("\r\n");
	   
	   			// send LCP REJECT packet
	   			send_data_processing(0, buf, dst_idx);
	   			wiz_write_byte(Sn_CR(0),Sn_CR_PCJ);
				/* +20071122[chungs]:wait to process the command... */
				while( wiz_read_byte(Sn_CR(0)) ) 
					;
				/* ------- */
  			}
		}
		printf(".");
		if (loop_idx++ == 10) // timeout
		{
			printf("timeout after LCP\r\n");
			return 3;
		}
		wait_10ms(100);
	}
	printf(" ok\r\n");
	printf("\r\n");

   /* +200801[bj] clear interrupt value*/
   wiz_write_byte(Sn_IR(0), 0xff);
   /*---*/

	printf("-- PHASE 3. PPPoE(ADSL) Authentication mode --\r\n");
	printf("Authentication protocol : %.2x %.2x, ", wiz_read_byte(PATR0), wiz_read_byte(PATR0+1));

	loop_idx = 0;
	if (wiz_read_word(PATR0) == 0xc023)
	{
		printf("PAP\r\n"); // in case of adsl normally supports PAP.
		// send authentication data
		// copy (idlen + id + passwdlen + passwd)
		buf[loop_idx] = idlen; loop_idx++;
		memcpy((uint8 *)(buf+loop_idx), (uint8 *)(id), idlen); loop_idx += idlen;
		buf[loop_idx] = passwdlen; loop_idx++;
		memcpy((uint8 *)(buf+loop_idx), (uint8 *)(passwd), passwdlen); loop_idx += passwdlen;
		send_data_processing(0, buf, loop_idx);
		wiz_write_byte(Sn_CR(0),Sn_CR_PCR);
		/* +20071122[chungs]:wait to process the command... */
		while( wiz_read_byte(Sn_CR(0)) ) 
			;
		/* ------- */
		wait_10ms(100);
	}	
	else if (wiz_read_word(PATR0) == 0xc223)
	{
		uint8 chal_len;
		md5_ctx context;
		uint8  digest[16];

		len = getSn_RX_RSR(0);
		if ( len > 0 )
		{
			recv_data_processing(0, str, len);
			wiz_write_byte(Sn_CR(0),Sn_CR_RECV);
			/* +20071122[chungs]:wait to process the command... */
			while( wiz_read_byte(Sn_CR(0)) ) 
				;
			/* ------- */
#ifdef __DEF_IINCHIP_DBG__
			printf("recv CHAP\r\n");
			{
				int16 i;
				
				for (i = 0; i < 32; i++) 
					printf ("%02x ", str[i]);
			}
			printf("\r\n");
#endif
// str is C2 23 xx CHAL_ID xx xx CHAP_LEN CHAP_DATA
// index  0  1  2  3       4  5  6        7 ...

			memset(buf,0x00,64);
			buf[loop_idx] = str[3]; loop_idx++; // chal_id
			memcpy((uint8 *)(buf+loop_idx), (uint8 *)(passwd), passwdlen); loop_idx += passwdlen; //passwd
			chal_len = str[6]; // chal_id
			memcpy((uint8 *)(buf+loop_idx), (uint8 *)(str+7), chal_len); loop_idx += chal_len; //challenge
			buf[loop_idx] = 0x80;
#ifdef __DEF_IINCHIP_DBG__
			printf("CHAP proc d1\r\n");
			{
				int16 i;
				for (i = 0; i < 64; i++) 
					printf ("%02x ", buf[i]);
			}
			printf("\r\n");
#endif

			md5_init(&context);
			md5_update(&context, buf, loop_idx);
			md5_final(digest, &context);

#ifdef __DEF_IINCHIP_DBG__
			printf("CHAP proc d1\r\n");
			{
				int16 i;				
				for (i = 0; i < 16; i++) 
					printf ("%02x", digest[i]);
			}
			printf("\r\n");
#endif
			loop_idx = 0;
			buf[loop_idx] = 16; loop_idx++; // hash_len
			memcpy((uint8 *)(buf+loop_idx), (uint8 *)(digest), 16); loop_idx += 16; // hashed value
			memcpy((uint8 *)(buf+loop_idx), (uint8 *)(id), idlen); loop_idx += idlen; // id
			send_data_processing(0, buf, loop_idx);
			wiz_write_byte(Sn_CR(0),Sn_CR_PCR);
			/* +20071122[chungs]:wait to process the command... */
			while( wiz_read_byte(Sn_CR(0)) ) 
				;
			/* ------- */
			wait_10ms(100);
		}
	}
	else
	{
		printf("Not support\r\n");
#ifdef __DEF_IINCHIP_DBG__
		printf("Not support PPP Auth type: %.2x%.2x\r\n",wiz_read_byte(PATR0), wiz_read_byte(PATR0+1));
#endif
		return 4;
	}
	printf("\r\n");

	printf("-- Waiting for PPPoE server's admission --");
	loop_idx = 0;
	while (!((isr = wiz_read_byte(Sn_IR(0))) & Sn_IR_PNEXT))
	{
		if (isr & Sn_IR_PFAIL)
		{
   /* +200801[bj] clear interrupt value*/
   wiz_write_byte(Sn_IR(0), 0xff);
   /*---*/
			printf("failed\r\nReinput id, password..\r\n");
			return 2;
		}
		printf(".");
		if (loop_idx++ == 10) // timeout
		{
   /* +200801[bj] clear interrupt value*/
   wiz_write_byte(Sn_IR(0), 0xff);
   /*---*/
			printf("timeout after PAP\r\n");
			return 3;
		}
		wait_10ms(100);
	}
   /* +200801[bj] clear interrupt value*/
   wiz_write_byte(Sn_IR(0), 0xff);
   /*---*/
	printf("ok\r\n");
	printf("\r\n");
	printf("-- PHASE 4. IPCP process --");
	// IP Address
	buf[0] = 0x03; buf[1] = 0x06; buf[2] = 0x00; buf[3] = 0x00; buf[4] = 0x00; buf[5] = 0x00;
	send_data_processing(0, buf, 6);
	wiz_write_byte(Sn_CR(0),Sn_CR_PCR);
	/* +20071122[chungs]:wait to process the command... */
	while( wiz_read_byte(Sn_CR(0)) ) 
		;
	/* ------- */
	wait_10ms(100);

	loop_idx = 0;
	while (1)
	{
		if (wiz_read_byte(Sn_IR(0)) & Sn_IR_PRECV)
		{
   /* +200801[bj] clear interrupt value*/
   wiz_write_byte(Sn_IR(0), 0xff);
   /*---*/
			len = getSn_RX_RSR(0);
			if ( len > 0 )
			{
				recv_data_processing(0, str, len);
				wiz_write_byte(Sn_CR(0),Sn_CR_RECV);
				/* +20071122[chungs]:wait to process the command... */
				while( wiz_read_byte(Sn_CR(0)) ) 
					;
				/* ------- */
	   			//for debug
	   			//printf("IPCP proc\r\n"); for (i = 0; i < len; i++) printf ("%02x ", str[i]); printf("\r\n");
	   			str_idx = 6; dst_idx = 0;
	   			if (str[2] == 0x03) // in case of NAK
	   			{
	   				do 
	   				{
	   					if (str[str_idx] == 0x03) // request only ip information
	   					{
	   						memcpy((uint8 *)(buf+dst_idx), (uint8 *)(str+str_idx), str[str_idx+1]);
	   						dst_idx += str[str_idx+1]; str_idx += str[str_idx+1];
	   					}
	   					else
	   					{
	   						// skip byte
	   						str_idx += str[str_idx+1];
	   					}
	   					// for debug
	   					//printf("s: %d, d: %d, l: %d", str_idx, dst_idx, len);
	   				} while (str_idx != len);
	   				send_data_processing(0, buf, dst_idx);
	   				wiz_write_byte(Sn_CR(0),Sn_CR_PCR); // send ipcp request
	   				/* +20071122[chungs]:wait to process the command... */
					while( wiz_read_byte(Sn_CR(0)) ) 
						;
					/* ------- */
	   				wait_10ms(100);
	   				break;
	   			}
			}
		}
		printf(".");
		if (loop_idx++ == 10) // timeout
		{
			printf("timeout after IPCP\r\n");
			return 3;
		}
		wait_10ms(100);
		send_data_processing(0, buf, 6);
		wiz_write_byte(Sn_CR(0),Sn_CR_PCR); //ipcp re-request
		/* +20071122[chungs]:wait to process the command... */
		while( wiz_read_byte(Sn_CR(0)) ) 
			;
		/* ------- */
	}

	loop_idx = 0;
	while (!(wiz_read_byte(Sn_IR(0)) & Sn_IR_PNEXT))
	{
		printf(".");
		if (loop_idx++ == 10) // timeout
		{
			printf("timeout after IPCP NAK\r\n");
			return 3;
		}
		wait_10ms(100);
		wiz_write_byte(Sn_CR(0),Sn_CR_PCR); // send ipcp request
		/* +20071122[chungs]:wait to process the command... */
		while( wiz_read_byte(Sn_CR(0)) ) 
			;
		/* ------- */
	}
   /* +200801[bj] clear interrupt value*/
   wiz_write_byte(Sn_IR(0), 0xff);
   /*---*/
	printf("ok\r\n");
	printf("\r\n");
	return 1;
	// after this function, User must save the pppoe server's mac address and pppoe session id in current connection
}


/**
@brief	terminate PPPoE connection
*/
uint8 pppterm(uint8 * mac, uint8 * sessionid)
{
	uint16 i;
	uint8 isr;
#ifdef __DEF_IINCHIP_DBG__
	printf("pppterm()\r\n");
#endif
	/* Set PPPoE bit in MR(Common Mode Register) : enable socket0 pppoe */
	wiz_write_byte(MR,wiz_read_byte(MR) | MR_PPPOE);
	
	// write pppoe server's mac address and session id 
	// must be setted these value.
	for (i = 0; i < 6; i++) wiz_write_byte((Sn_DHAR0(0)+i),mac[i]);
	for (i = 0; i < 2; i++) wiz_write_byte((Sn_DPORT0(0)+i),sessionid[i]);
	isr = wiz_read_byte(Sn_IR(0));
	wiz_write_byte(Sn_IR(0),isr);
	
	//open socket in pppoe mode
	wiz_write_byte(Sn_MR(0),Sn_MR_PPPOE);
	wiz_write_byte(Sn_CR(0),Sn_CR_OPEN);
	/* +20071122[chungs]:wait to process the command... */
	while( wiz_read_byte(Sn_CR(0)) ) 
		;
	/* ------- */
	wait_1us(1);
	// close pppoe connection
	wiz_write_byte(Sn_CR(0),Sn_CR_PDISCON);
	/* +20071122[chungs]:wait to process the command... */
	while( wiz_read_byte(Sn_CR(0)) ) 
		;
	/* ------- */
	wait_10ms(100);
	// close socket
	/* +200801 (hwkim) */
	close(0);
	/* ------- */
	

#ifdef __DEF_IINCHIP_DBG__
	printf("pppterm() end ..\r\n");
#endif

	return 1;
}
#endif
