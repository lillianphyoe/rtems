/*
 * Copyright (c) 2013 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <info@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.org/license/LICENSE.
 */

#include <bsp.h>
#include <bsp/bootcard.h>
#include <bsp/irq-generic.h>
#include <bsp/fdt.h>
#include <bsp/linker-symbols.h>
#include <bsp/i2c.h>
#include <rtems/sysinit.h>
#include "bsp-soc-detect.h"
#include <arm/ti/ti_pinmux.h>
#include <ofw/ofw.h>

#include "bspdebug.h"

void bsp_start(void)
{
  const char *type;

  bsp_soc_detect();

  switch (ti_chip()) {
    case CHIP_AM335X:
      type = "am335x-based";
      break;
    case CHIP_OMAP_3:
      type = "dm3730-based";
      break;
    default:
      type = "Unknown SOC";
      break;
  }

  bsp_interrupt_initialize();
  printk("\nRTEMS Beagleboard: %s\n", type);
  printk("        ARM Debug: 0x%08x\n", (intptr_t) bbb_arm_debug_registers());
  rtems_cache_coherent_add_area(
      bsp_section_nocacheheap_begin,
      (uintptr_t) bsp_section_nocacheheap_size
  );
}

uint32_t bsp_fdt_map_intr(const uint32_t *intr, size_t icells)
{
  return intr[0];
}

static void traverse_fdt_nodes( phandle_t node )
{

  for (node = rtems_ofw_child(node); node != 0; node = rtems_ofw_peer(node)) {
    traverse_fdt_nodes(node);

    if (!rtems_ofw_node_status(node))
      continue;

    /*
     * Put all driver initialization functions here
     */
    beagle_pinmux_init(node);
  }
}

static void bbb_drivers_initialize(void)
{
  phandle_t node = rtems_ofw_peer(0);

  traverse_fdt_nodes(node);
}

static void bbb_i2c_0_initialize(void)
{
  int err;

  err = am335x_i2c_bus_register(BBB_I2C_0_BUS_PATH,
                                AM335X_I2C0_BASE,
                                I2C_BUS_CLOCK_DEFAULT,
                                BBB_I2C0_IRQ);
  if (err != 0) {
    printk("rtems i2c-0: Device could not be registered (%d)", err);
  }
}

RTEMS_SYSINIT_ITEM(
  bbb_i2c_0_initialize,
  RTEMS_SYSINIT_LAST,
  RTEMS_SYSINIT_ORDER_LAST_BUT_5
);

RTEMS_SYSINIT_ITEM(
	bbb_drivers_initialize,
	RTEMS_SYSINIT_BSP_PRE_DRIVERS,
	RTEMS_SYSINIT_ORDER_LAST
);
