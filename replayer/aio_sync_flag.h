#ifndef LIBAIO_H_
#define LIBAIO_H_

#include <libaio.h>

#define IOCB_FLAG_SYNC	(1 << 1)

static inline void io_set_sync_flag(struct iocb *iocb)
{
	iocb->u.c.flags |=  IOCB_FLAG_SYNC;
}

#endif /* LIBAIO_H_ */
