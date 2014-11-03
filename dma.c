/*-
 * Copyright (c) 2012 Oleksandr Tymoshenko <gonzo@freebsd.org>
 * Copyright (c) 2012, 2013 The FreeBSD Foundation
 * All rights reserved.
 *
 * Portions of this software were developed by Oleksandr Rybalko
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bio.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/endian.h>
#include <sys/kernel.h>
#include <sys/kthread.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/queue.h>
#include <sys/resource.h>
#include <sys/rman.h>
#include <sys/time.h>
#include <sys/timetc.h>
#include <sys/fbio.h>
#include <sys/consio.h>
#include <sys/eventhandler.h>

#include <machine/bus.h>
#include <machine/cpu.h>
#include <machine/cpufunc.h>
#include <machine/fdt.h>
#include <machine/resource.h>
#include <machine/frame.h>
#include <machine/intr.h>

#include <dev/fdt/fdt_common.h>
#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

struct ipu3sc_softc {
	device_t	dev;

	int		dma_size;
	bus_dma_tag_t	dma_tag;
	bus_dmamap_t	dma_map;
	uint32_t	*buf_base;
};

static int ipuv3_fb_probe(device_t);
static int ipuv3_fb_attach(device_t);
static int ipuv3_fb_attach(device_t);

static int
ipuv3_fb_probe(device_t dev)
{
	if (!ofw_bus_status_okay(dev))
		return (ENXIO);

	if (!ofw_bus_is_compatible(dev, "fsl,ipu3") &&
		!ofw_bus_is_compatible(dev, "fsl,imx6q-ipu"))
		return (ENXIO);

	device_set_desc(dev, "i.MX5x Image Processing Unit v3 (FB)");

	return (BUS_PROBE_DEFAULT);
}

static int
ipuv3_fb_attach(device_t dev)
{
	struct ipu3sc_softc *sc = device_get_softc(dev);
	int error;

	sc->dev = dev;
	sc->dma_size = 131072;

	error = bus_dma_tag_create(
		bus_get_dma_tag(sc->dev),
		0, 0,				/* alignment, boundary */
		BUS_SPACE_MAXADDR_32BIT,	/* lowaddr */
		BUS_SPACE_MAXADDR,		/* highaddr */
		NULL, NULL,			/* filter, filterarg */
		sc->dma_size, 1,		/* maxsize, nsegments */
		sc->dma_size, 0,		/* maxsegsize, flags */
		NULL, NULL,			/* lockfunc, lockarg */
		&sc->dma_tag);

	if (error) {
		device_printf(sc->dev, "cannot create dma tag\n");
		return (ENXIO);
	}

	uprintf("created DMA tag\n");

	error = bus_dmamem_alloc(sc->dma_tag, (void **)&sc->buf_base,
		BUS_DMA_NOWAIT | BUS_DMA_COHERENT, &sc->dma_map);


	if (error) {
		device_printf(sc->dev, "cannot allocate framebuffer\n");
		return (ENXIO);
	}

	uprintf("allocated fb %p\n", sc->buf_base);

	return (0);
}

static int
ipu3_fb_detach(device_t dev)
{
	return (0);
}

static device_method_t ipu3_fb_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		ipuv3_fb_probe),
	DEVMETHOD(device_attach,	ipuv3_fb_attach),
	DEVMETHOD(device_detach,	ipu3_fb_detach),
	{ 0, 0 }
};

static devclass_t ipu3_fb_devclass;

static driver_t ipu3_fb_driver = {
	"fb",
	ipu3_fb_methods,
	sizeof(struct ipu3sc_softc),
};

DRIVER_MODULE(fb, simplebus, ipu3_fb_driver, ipu3_fb_devclass, 0, 0);
