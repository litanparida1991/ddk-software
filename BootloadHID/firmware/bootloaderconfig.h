/* Name: bootloaderconfig.h
 * Project: AVR bootloader HID
 * Author: Christian Starkjohann
 * Creation Date: 2007-03-19
 * Tabsize: 4
 * Copyright: (c) 2007 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt)
 * This Revision: $Id: bootloaderconfig.h 699 2008-11-26 19:50:32Z cs $
 */

#ifndef __bootloaderconfig_h_included__
#define __bootloaderconfig_h_included__

/*
General Description:
This file (together with some settings in Makefile) configures the boot loader
according to the hardware.

This file contains (besides the hardware configuration normally found in
usbconfig.h) two functions or macros: bootLoaderInit() and
bootLoaderCondition(). Whether you implement them as macros or as static
inline functions is up to you, decide based on code size and convenience.

bootLoaderInit() is called as one of the first actions after reset. It should
be a minimum initialization of the hardware so that the boot loader condition
can be read. This will usually consist of activating a pull-up resistor for an
external jumper which selects boot loader mode. You may call leaveBootloader()
from this function if you know that the main code should run.

bootLoaderCondition() is called immediately after initialization and in each
main loop iteration. If it returns TRUE, the boot loader will be active. If it
returns FALSE, the boot loader jumps to address 0 (the loaded application)
immediately.

For compatibility with Thomas Fischl's avrusbboot, we also support the macro
names BOOTLOADER_INIT and BOOTLOADER_CONDITION for this functionality. If
these macros are defined, the boot loader usees them.
*/

/* ---------------------------- Hardware Config ---------------------------- */

#define USB_CFG_IOPORTNAME      D
/* This is the port where the USB bus is connected. When you configure it to
 * "B", the registers PORTB, PINB and DDRB will be used.
 */
#define USB_CFG_DMINUS_BIT      3
/* This is the bit number in USB_CFG_IOPORT where the USB D- line is connected.
 * This may be any bit in the port.
 */
#define USB_CFG_DPLUS_BIT       2
/* This is the bit number in USB_CFG_IOPORT where the USB D+ line is connected.
 * This may be any bit in the port. Please note that D+ must also be connected
 * to interrupt pin INT0! [You can also use other interrupts, see section
 * "Optional MCU Description" below, or you can connect D- to the interrupt, as
 * it is required if you use the USB_COUNT_SOF feature. If you use D- for the
 * interrupt, the USB interrupt will also be triggered at Start-Of-Frame
 * markers every millisecond.]
 */
#define USB_CFG_CLOCK_KHZ       (F_CPU/1000)
/* Clock rate of the AVR in KHz. Legal values are 12000, 12800, 15000, 16000,
 * 16500 and 20000. The 12.8 MHz and 16.5 MHz versions of the code require no
 * crystal, they tolerate +/- 1% deviation from the nominal frequency. All
 * other rates require a precision of 2000 ppm and thus a crystal!
 * Default if not specified: 12 MHz
 */

/* ----------------------- Optional Hardware Config ------------------------ */

/* #define USB_CFG_PULLUP_IOPORTNAME   D */
/* If you connect the 1.5k pullup resistor from D- to a port pin instead of
 * V+, you can connect and disconnect the device from firmware by calling
 * the macros usbDeviceConnect() and usbDeviceDisconnect() (see usbdrv.h).
 * This constant defines the port on which the pullup resistor is connected.
 */
/* #define USB_CFG_PULLUP_BIT          4 */
/* This constant defines the bit number in USB_CFG_PULLUP_IOPORT (defined
 * above) where the 1.5k pullup resistor is connected. See description
 * above for details.
 */

/* --------------------------- Functional Range ---------------------------- */

#define BOOTLOADER_CAN_EXIT     1
/* If this macro is defined to 1, the boot loader command line utility can
 * initiate a reboot after uploading the FLASH when the "-r" command line
 * option is given. If you define it to 0 or leave it undefined, the "-r"
 * option won't work and you save a couple of bytes in the boot loader. This
 * may be of advantage if you compile with gcc 4 instead of gcc 3 because it
 * generates slightly larger code.
 * If you need to save even more memory, consider using your own vector table.
 * Since only the reset vector and INT0 (the first two vectors) are used,
 * this saves quite a bit of flash. See Alexander Neumann's boot loader for
 * an example: http://git.lochraster.org:2080/?p=fd0/usbload;a=tree
 */

/* ------------------------------------------------------------------------- */

/* Example configuration: Port D bit 3 is connected to a jumper which ties
 * this pin to GND if the boot loader is requested. Initialization allows
 * several clock cycles for the input voltage to stabilize before
 * bootLoaderCondition() samples the value.
 * We use a function for bootLoaderInit() for convenience and a macro for
 * bootLoaderCondition() for efficiency.
 */

#ifndef __ASSEMBLER__   /* assembler cannot parse function definitions */
#include <util/delay.h>

/*
We assume that an active high LED is connected to port B bit 7. If you connect
it with different polarity change it in the usbFunctionSetup below. If you
connect it to a different port or bit, change the macros below:
*/
#define LED_PORT_DDR        DDRB
#define LED_PORT_OUTPUT     PORTB
#define LED_BIT             7
#define DL_PORT_DDR         DDRB
#define DL_PORT_PULLUP      PORTB
#define DL_PORT_INPUT       PINB
#define DL_BIT              4

static inline void  bootLoaderInit(void)
{
	DL_PORT_PULLUP |= _BV(DL_BIT);	/* Activate pull-up for key */
    DL_PORT_DDR &= ~_BV(DL_BIT);	/* Make the switch bit an input */
    _delay_us(10);					/* Wait for levels to stabilize */
    LED_PORT_OUTPUT |= _BV(LED_BIT);/* Switch on LED to start with */
    LED_PORT_DDR |= _BV(LED_BIT);	/* Make the LED bit an output */
}
static inline void  bootLoaderShut(void)
{
	DL_PORT_PULLUP &= ~_BV(DL_BIT);	/* Reset / De-activate pull-up for key */
    LED_PORT_OUTPUT &= ~_BV(LED_BIT);/* Switch off LED before stopping */
    LED_PORT_DDR &= ~_BV(LED_BIT);	/* Reset the LED bit to default input state */
}

#define bootLoaderCondition() ((DL_PORT_INPUT & _BV(DL_BIT)) == 0) /* True if switch is pressed */
#define toggleLED() (LED_PORT_OUTPUT ^= _BV(LED_BIT))

#endif

/* ------------------------------------------------------------------------- */

#endif /* __bootloader_h_included__ */
