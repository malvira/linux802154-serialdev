#include "linux.h"
#define start_command() ((sb[0] == START_BYTE1) && (sb[1] == START_BYTE2))

#include <mc1322x.h>
#include <board.h>
#include <stdio.h> /* For printf() */

void give_to_linux(volatile packet_t *p) {
	uint8_t i;
        /* linux doesn't need these null to frame, but the python tests do */
	uart1_putc(0);
	uart1_putc(0);
	/* send data_recv_block */
	printf("zb");
	uart1_putc(DATA_RECV_BLOCK);
	uart1_putc(0);
	uart1_putc(p->length); 

	for(i=0; i < p->length ; i++) {
		uart1_putc(p->data[ i + p->offset]);
	}

}

/* Baud rate */
#define MOD 9999
/*  230400 bps, INC=767, MOD=9999, 24Mhz 16x samp */
/*  115200 bps, INC=767, MOD=9999, 24Mhz 8x samp */
#define INC 767  
/*  921600 bps, MOD=9999, 24Mhz 16x samp */
//#define INC 3071 
#define SAMP UCON_SAMP_8X
//#define SAMP UCON_SAMP_16X

/* How long to wait for command bytes */
#define COMMAND_TIMEOUT  4096
/* linux uses channels 1-16 the maca driver expects 0-15 */
/* and 802.15.4 is 11-26 (for 2.4GHz) */
#define PHY_CHANNEL_OFFSET 1 

void main(void) {	
	volatile uint8_t sb[NUM_START_BYTES];
	volatile uint32_t i, timeout;
	volatile uint8_t cmd, parm1;
	static volatile uint8_t state = IDLE_MODE;
	volatile packet_t *p = 0;

	uart1_init(INC, MOD, SAMP);
	vreg_init();
	maca_init();
	radio_off();

	while(1) {
		
		/* clear out sb */
		for(i = 0; i < NUM_START_BYTES; i++) {
			sb[i] = 0;
		}

		/* recieve bytes until we see the first start byte */
		/* this syncs up to the commands */
		while(sb[0] != START_BYTE1) {
			while(!uart1_can_get() && 
			      ((p = rx_packet()) == 0)) { continue; }
			if(p) { 
				give_to_linux(p);
				free_packet(p);
				continue;
			}
			if(uart1_can_get()) { sb[0] = uart1_getc(); }
		}

		/* receive start bytes */
		for(i=1; i<NUM_START_BYTES; i++) {
			timeout=0;
			while(!uart1_can_get() &&
			      (timeout < COMMAND_TIMEOUT)) { timeout++; }
			sb[i] = uart1_getc();
		}
		
		if(start_command()) {
			/* do a command */
			cmd = 0;

			timeout = 0;
			while(!uart1_can_get() &&
			      (timeout < COMMAND_TIMEOUT)) { timeout++; };
			cmd = uart1_getc();
			
			switch(cmd)
			{
			case CMD_OPEN:
				radio_on();
				set_power(0x12); /* 4.5dbm */
				set_channel(0); /* channel 11 */
				printf("zb");
				uart1_putc(RESP_OPEN);
				uart1_putc(STATUS_SUCCESS);
				break;
			case CMD_CLOSE:
				radio_off();
				free_all_packets();
				printf("zb");
				uart1_putc(RESP_CLOSE);
				uart1_putc(STATUS_SUCCESS);
				break;
			case CMD_SET_CHANNEL:
				radio_off();				
				parm1 = uart1_getc();
				set_channel(parm1-PHY_CHANNEL_OFFSET);
				radio_on();
				printf("zb");
				uart1_putc(RESP_SET_CHANNEL);
				uart1_putc(STATUS_SUCCESS);
				break;
			case CMD_ED:
				printf("zb");
				uart1_putc(RESP_ED);
				uart1_putc(STATUS_ERR);
				uart1_putc(0);
				break;
			case CMD_CCA:
				printf("zb");
				uart1_putc(RESP_CCA);
				uart1_putc(STATUS_SUCCESS);
				break;
			case CMD_SET_STATE:
				state = uart1_getc();
				printf("zb");
				uart1_putc(RESP_SET_STATE);
				uart1_putc(STATUS_SUCCESS);
				break;
			case DATA_XMIT_BLOCK:

				/* send packet here */
				if( ( p = get_free_packet() ) ) {

					p->length = uart1_getc();
					/* linux gives us a checksum, but the radio is going */
					/* to add it automatically */
					
					for(i=0; i < p->length; i++) {
						p->data[ i + p->offset] = uart1_getc();
					}
				      					
					tx_packet(p);
					
					printf("zb");
					uart1_putc(RESP_XMIT_BLOCK);
					uart1_putc(STATUS_SUCCESS);

				} else {
					printf("zb");
					uart1_putc(RESP_XMIT_BLOCK);
					uart1_putc(STATUS_BUSY);
				}
				break;
			default:
				break;
			}
		}
	}
}
