/*
*
@file		socket.c
@brief	setting chip register for socket
		last update : 2008. Jan
*
*/

#include "types.h"
#include "w5100.h"
#include "socket.h"

static uint16 local_port;


/**
@brief	This Socket function initialize the channel in perticular mode, and set the port and wait for W5100 done it.
@return	1 for sucess else 0.
*/	 
uint8 socket(
	SOCKET s,		/**< for socket number */
	uint8 protocol,	/**< for socket protocol */
	uint16 port,		/**< the source port for the socket */
	uint8 flag		/**< the option for the socket */
	)
{
	uint8 ret;
#ifdef __DEF_IINCHIP_DBG__
	printf("socket()\r\n");
#endif
	if ((protocol == Sn_MR_TCP) || (protocol == Sn_MR_UDP) || (protocol == Sn_MR_IPRAW) || (protocol == Sn_MR_MACRAW) || (protocol == Sn_MR_PPPOE))
	{
		close(s);
		if (!port) port = local_port++;
		wiz_write_byte(Sn_MR(s),protocol | flag);
		// if don't set the source port, set local_port number.
		wiz_write_word(Sn_PORT0(s),port);
		wiz_write_byte(Sn_CR(s),Sn_CR_OPEN); // run sockinit Sn_CR

		/* +20071122[chungs]:wait to process the command... */
		while( wiz_read_byte(Sn_CR(s)) ) 
			;
		/* ------- */
		ret = 1;
	}
	else
	{
		ret = 0;
	}
#ifdef __DEF_IINCHIP_DBG__
	printf("Sn_SR = %.2x , Protocol = %.2x\r\n", wiz_read_byte(Sn_SR(s)), wiz_read_byte(Sn_MR(s)));
#endif
	return ret;
}


/**
@brief	This function close the socket and parameter is "s" which represent the socket number
*/ 
void close(SOCKET s)
{
#ifdef __DEF_IINCHIP_DBG__
	printf("close()\r\n");
#endif
	
	wiz_write_byte(Sn_CR(s),Sn_CR_CLOSE);

	/* +20071122[chungs]:wait to process the command... */
	while( wiz_read_byte(Sn_CR(s)) ) 
		;
	/* ------- */

	/* +2008.01 [hwkim]: clear interrupt */	
	#ifdef __DEF_IINCHIP_INT__
		/* m2008.01 [bj] : all clear */
		putISR(s, 0x00);
	#else
		/* m2008.01 [bj] : all clear */
		wiz_write_byte(Sn_IR(s), 0xFF);
	#endif
}


/**
@brief	This function established	the connection for the channel in passive (server) mode. This function waits for the request from the peer.
@return	1 for success else 0.
*/ 
uint8 listen(
	SOCKET s /**< the socket number */
	)
{
	uint8 ret;
#ifdef __DEF_IINCHIP_DBG__
	printf("listen()\r\n");
#endif
	if (wiz_read_byte(Sn_SR(s)) == SOCK_INIT)
	{
		wiz_write_byte(Sn_CR(s),Sn_CR_LISTEN);
		/* +20071122[chungs]:wait to process the command... */
		while( wiz_read_byte(Sn_CR(s)) ) 
			;
		/* ------- */
		ret = 1;
	}
	else
	{
		ret = 0;
#ifdef __DEF_IINCHIP_DBG__
	printf("Fail[invalid ip,port]\r\n");
#endif
	}
	return ret;
}


/**
@brief	This function established	the connection for the channel in Active (client) mode. 
		This function waits for the untill the connection is established.
		
@return	1 for success else 0.
*/ 
uint8 connect(SOCKET s, uint8 * addr, uint16 port)
{
	uint8 ret;
#ifdef __DEF_IINCHIP_DBG__
	printf("connect()\r\n");
#endif

	if 
		(
			((addr[0] == 0xFF) && (addr[1] == 0xFF) && (addr[2] == 0xFF) && (addr[3] == 0xFF)) ||
			((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
			(port == 0x00) 
		) 
	{
		ret = 0;
#ifdef __DEF_IINCHIP_DBG__
	printf("Fail[invalid ip,port]\r\n");
#endif
	}
	else
	{
		ret = 1;
		// set destination IP
		wiz_write_buf(Sn_DIPR0(s), addr,4);
		wiz_write_word(Sn_DPORT0(s),port);
		wiz_write_byte(Sn_CR(s),Sn_CR_CONNECT);
		/* m2008.01 [bj] :  wait for completion */
		while ( wiz_read_byte(Sn_CR(s)) ) ;
	}

	return ret;
}



/**
@brief	This function used for disconnect the socket and parameter is "s" which represent the socket number
@return	1 for success else 0.
*/ 
void disconnect(SOCKET s)
{
#ifdef __DEF_IINCHIP_DBG__
	printf("disconnect()\r\n");
#endif
	wiz_write_byte(Sn_CR(s),Sn_CR_DISCON);

	/* +20071122[chungs]:wait to process the command... */
	while( wiz_read_byte(Sn_CR(s)) ) 
		;
	/* ------- */
}


/**
@brief	This function used to send the data in TCP mode
@return	1 for success else 0.
*/ 
uint16 send(
	SOCKET s,		/**< the socket index */
	const uint8 * buf,	/**< a pointer to data */
	uint16 len		/**< the data size to be send */
	)
{
	uint8 status=0;
	uint16 ret=0;
	uint16 freesize=0;
#ifdef __DEF_IINCHIP_DBG__
	printf("send()\r\n");
#endif

	if (len > IINCHIP_TxMAX) ret = IINCHIP_TxMAX; // check size not to exceed MAX size.
	else ret = len;

	// if freebuf is available, start.
	do 
	{
		freesize = getSn_TX_FSR(s);
		status = wiz_read_byte(Sn_SR(s));
		if ((status != SOCK_ESTABLISHED) && (status != SOCK_CLOSE_WAIT))
		{
			ret = 0; 
			break;
		}
#ifdef __DEF_IINCHIP_DBG__
		printf("socket %d freesize(%d) empty or error\r\n", s, freesize);
#endif
	} while (freesize < ret);

		// copy data
	send_data_processing(s, (uint8 *)buf, ret);

	/* ------- */

/* +2008.01 bj */ 
#ifdef __DEF_IINCHIP_INT__
	while ( (getISR(s) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK ) 
#else
	while ( (wiz_read_byte(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK ) 
#endif
	{
		/* m2008.01 [bj] : reduce code */
		if ( wiz_read_byte(Sn_SR(s)) == SOCK_CLOSED )
		{
#ifdef __DEF_IINCHIP_DBG__
			printf("SOCK_CLOSED.\r\n");
#endif
			close(s);
			return 0;
		}
	}
/* +2008.01 bj */ 
#ifdef __DEF_IINCHIP_INT__
	putISR(s, getISR(s) & (~Sn_IR_SEND_OK));
#else
	wiz_write_byte(Sn_IR(s), Sn_IR_SEND_OK);
#endif
	return ret;
}


/**
@brief	This function is an application I/F function which is used to receive the data in TCP mode.
		It continues to wait for data as much as the application wants to receive.
		
@return	received data size for success else -1.
*/ 
uint16 recv(
	SOCKET s,	/**< socket index */
	uint8 * buf,	/**< a pointer to copy the data to be received */
	uint16 len	/**< the data size to be read */
	)
{
#ifdef __DEF_IINCHIP_DBG__
	printf("recv()\r\n");
#endif
	if ( len > 0 )
	{
		recv_data_processing(s, buf, len);
	}
	return len;
}


/**
@brief	This function is an application I/F function which is used to send the data for other then TCP mode. 
		Unlike TCP transmission, The peer's destination address and the port is needed.
		
@return	This function return send data size for success else -1.
*/ 
uint16 sendto(
	SOCKET s,		/**< socket index */
	const uint8 * buf,	/**< a pointer to the data */
	uint16 len,			/**< the data size to send */
	uint8 * addr,		/**< the peer's Destination IP address */
	uint16 port		/**< the peer's destination port number */
	)
{
	uint16 ret=0;
	
	if (len > IINCHIP_TxMAX) ret = IINCHIP_TxMAX; // check size not to exceed MAX size.
	else ret = len;

	if
		(
			((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
			((port == 0x00)) ||(ret == 0)
		) 
	{
		/* +2008.01 [bj] : added return value */
		ret = 0;
	}
	else
	{
		wiz_write_buf(Sn_DIPR0(s),addr,4);
		wiz_write_word(Sn_DPORT0(s),port);
		send_data_processing(s, (uint8 *)buf, ret);
		
/* +2008.01 bj */ 
#ifdef __DEF_IINCHIP_INT__
		while ( (getISR(s) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK ) 
#else
		while ( (wiz_read_byte(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK ) 
#endif
		{
#ifdef __DEF_IINCHIP_INT__
			if (getISR(s) & Sn_IR_TIMEOUT)
#else
			if (wiz_read_byte(Sn_IR(s)) & Sn_IR_TIMEOUT)
#endif
			{
#ifdef __DEF_IINCHIP_DBG__
				printf("send fail.\r\n");
#endif
/* +2008.01 [bj]: clear interrupt */
#ifdef __DEF_IINCHIP_INT__
				putISR(s, getISR(s) & ~(Sn_IR_SEND_OK | Sn_IR_TIMEOUT));	 /* clear SEND_OK & TIMEOUT */
#else
				wiz_write_byte(Sn_IR(s), (Sn_IR_SEND_OK | Sn_IR_TIMEOUT)); /* clear SEND_OK & TIMEOUT */
#endif
			return 0;
			}
		}

/* +2008.01 bj */ 
#ifdef __DEF_IINCHIP_INT__
		putISR(s, getISR(s) & (~Sn_IR_SEND_OK));
#else
		wiz_write_byte(Sn_IR(s), Sn_IR_SEND_OK);
#endif

	}
	return ret;
}


/**
@brief	This function is an application I/F function which is used to receive the data in other then
	TCP mode. This function is used to receive UDP, IP_RAW and MAC_RAW mode, and handle the header as well. 
	
@return	This function return received data size for success else -1.
*/ 
uint16 recvfrom(
	SOCKET s,	/**< the socket number */
	uint8 * buf,	/**< a pointer to copy the data to be received */
	uint16 len,		/**< the data size to read */
	uint8 * addr,	/**< a pointer to store the peer's IP address */
	uint16 *port	/**< a pointer to store the peer's port number. */
	)
{
	uint8 head[8];
	uint16 data_len=0;
	uint16 ptr=0;

	if ( len > 0 )
	{
		ptr = wiz_read_word(Sn_RX_RD0(s));
		switch (wiz_read_byte(Sn_MR(s)) & 0x07)
		{
		case Sn_MR_UDP :
			read_data(s, (uint8 *)ptr, head, 0x08);
			ptr += 8;
			// read peer's IP address, port number.
			addr[0] = head[0];
			addr[1] = head[1];
			addr[2] = head[2];
			addr[3] = head[3];
			*port = (head[4] << 8) + head[5];
			data_len = (head[6] << 8) + head[7];
			break;
	
		case Sn_MR_IPRAW :
			read_data(s, (uint8 *)ptr, head, 0x06);
			ptr += 6;
			addr[0] = head[0];
			addr[1] = head[1];
			addr[2] = head[2];
			addr[3] = head[3];
			data_len = (head[4] << 8) + head[5];
			break;

		case Sn_MR_MACRAW :
			read_data(s, (uint8 *)ptr, head, 2);
			ptr += 2;
			data_len = (head[0]<<8) + head[1] - 2;
			break;

		default :
			break;
		}
		read_data(s,(uint8*) ptr,buf,data_len); // Does nothing if data_len = 0
		ptr += data_len;
		wiz_write_word(Sn_RX_RD0(s),ptr);
		wiz_write_byte(Sn_CR(s),Sn_CR_RECV);
		while( wiz_read_byte(Sn_CR(s)) ) 
			;
	}
	return data_len;
}

uint16 igmpsend(SOCKET s, const uint8 * buf, uint16 len)
{
	uint8 status=0;
// uint8 isr=0;
	uint16 ret=0;
	
#ifdef __DEF_IINCHIP_DBG__
	printf("igmpsend()\r\n");
#endif
	if (len > IINCHIP_TxMAX) ret = IINCHIP_TxMAX; // check size not to exceed MAX size.
	else ret = len;

	if (ret == 0) 
	{
		;
#ifdef __DEF_IINCHIP_DBG__
	printf("%d Fail[%d]\r\n",len);
#endif
	}
	else
	{
		// copy data
		send_data_processing(s, (uint8 *)buf, ret);
		
/* +2008.01 bj */ 
#ifdef __DEF_IINCHIP_INT__
		while ( (getISR(s) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK ) 
#else
		while ( (wiz_read_byte(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK ) 
#endif
		{
			status = wiz_read_byte(Sn_SR(s));
#ifdef __DEF_IINCHIP_INT__
			if (getISR(s) & Sn_IR_TIMEOUT)
#else
			if (wiz_read_byte(Sn_IR(s)) & Sn_IR_TIMEOUT)
#endif
			{
#ifdef __DEF_IINCHIP_DBG__
				printf("igmpsend fail.\r\n");
#endif
				/* in case of igmp, if send fails, then socket closed */
				/* if you want change, remove this code. */
				close(s);
				/* ----- */
				
				return 0;
			}
		}

/* +2008.01 bj */ 
#ifdef __DEF_IINCHIP_INT__
		putISR(s, getISR(s) & (~Sn_IR_SEND_OK));
#else
		wiz_write_byte(Sn_IR(s), Sn_IR_SEND_OK);
#endif
	}
	return ret;
}

