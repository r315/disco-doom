
#ifndef _fifo_h_
#define _fifo_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define FIFO_SIZE	512

	typedef struct {
		uint32_t head;
		uint32_t tail;
		uint32_t size;
		uint8_t	buf[FIFO_SIZE];
	} fifo_t;


	void fifo_init(fifo_t *fifo);
	void fifo_flush(fifo_t *fifo);
	uint8_t fifo_put(fifo_t *fifo, uint8_t c);
	uint8_t fifo_get(fifo_t *fifo, uint8_t *pc);
	int  fifo_avail(fifo_t *fifo);
	int	 fifo_free(fifo_t *fifo);

#ifdef __cplusplus
}
#endif

#endif