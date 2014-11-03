KMOD=	dma

SRCS=   device_if.h bus_if.h ofw_bus_if.h
SRCS+=	dma.c

.include <bsd.kmod.mk>
