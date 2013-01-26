#include <aery32/all.h>
#include "board.h"

using namespace aery;

#define LED AVR32_PIN_PC04

int configure_endpoints(void)
{
	/*
	 * It's recommended to reset every endpoint before using those upon
	 * hardware reset.
	 */
	usb_free_all_eps();
	usb_reset_all_eps();

	/* Endpoints have to be enabled before configure/allocation. */
	usb_enable_all_eps();

	usb_alloc_epn(0, USB_EPBANK_SINGLE, USB_EPSIZE_64,  USB_EPTYPE_CONTROL,     USB_EPDIR_OUT);
	usb_alloc_epn(1, USB_EPBANK_DOUBLE, USB_EPSIZE_64,  USB_EPTYPE_BULK,        USB_EPDIR_IN);
	usb_alloc_epn(2, USB_EPBANK_DOUBLE, USB_EPSIZE_64,  USB_EPTYPE_BULK,        USB_EPDIR_OUT);
	usb_alloc_epn(3, USB_EPBANK_SINGLE, USB_EPSIZE_64,  USB_EPTYPE_INTERRUPT,   USB_EPDIR_IN);
	usb_alloc_epn(4, USB_EPBANK_SINGLE, USB_EPSIZE_64,  USB_EPTYPE_INTERRUPT,   USB_EPDIR_OUT);
	usb_alloc_epn(5, USB_EPBANK_SINGLE, USB_EPSIZE_256, USB_EPTYPE_ISOCHRONOUS, USB_EPDIR_IN);
	usb_alloc_epn(6, USB_EPBANK_SINGLE, USB_EPSIZE_256, USB_EPTYPE_ISOCHRONOUS, USB_EPDIR_OUT);

	/* Test if the endpoint configurations are correct. */
	for (int i = 0; i < 7; i++) {
		if (!usb_epn_is_active(i))
			return -10 - i;
	}

	return 0;
}

void isrhandler_group17(void)
{
	if (AVR32_USBB.UDINT.ep0int) {
		if (AVR32_USBB.UESTA0.rxstpi) {
			//gpio_set_pin_low(LED);
			AVR32_USBB.UESTA0CLR.rxstpic = 1;
		}
		if (AVR32_USBB.UESTA0.txini) {
		}
		if (AVR32_USBB.UESTA0.rxouti) {
		}
		if (AVR32_USBB.UESTA0.nakouti) {
		}
		if (AVR32_USBB.UESTA0.nakini) {
		}
		if (AVR32_USBB.UESTA0.overfi) {
		}
		if (AVR32_USBB.UESTA0.stalledi) {
		}
		if (AVR32_USBB.UESTA0.shortpacket) {
		}
		return;
	}
	if (AVR32_USBB.UDINT.ep1int) {
		if (AVR32_USBB.UESTA1.txini) {
		}
		else if (AVR32_USBB.UESTA1.rxouti) {
		}
		else if (AVR32_USBB.UESTA1.rxstpi) {
		}
		else if (AVR32_USBB.UESTA1.nakouti) {
		}
		else if (AVR32_USBB.UESTA1.nakini) {
		}
		else if (AVR32_USBB.UESTA1.overfi) {
		}
		else if (AVR32_USBB.UESTA1.stalledi) {
		}
		else if (AVR32_USBB.UESTA1.shortpacket) {
		}
		return;
	}
	if (AVR32_USBB.UDINT.ep2int) {
		return;
	}
	if (AVR32_USBB.UDINT.ep3int) {
		return;
	}
	if (AVR32_USBB.UDINT.ep4int) {
		return;
	}
	if (AVR32_USBB.UDINT.ep5int) {
		return;
	}
	if (AVR32_USBB.UDINT.ep6int) {
		return;
	}
	if (AVR32_USBB.UDINT.susp) {
		gpio_set_pin_low(LED); // Seems like I'm in suspend mode
		return;
	}
}

int main(void)
{
	board::init();
	gpio_init_pin(LED, GPIO_OUTPUT);

	intc_register_isrhandler(&isrhandler_group17, 17, 0);
	intc_init();
	intc_enable_globally();

	// Init and enable generic clock for USB (48MHz)
	// Params: - Generic clock number which to init
	//         - Generic clock source, see datasheet p.79
	//         - Source clock divider. PLL1 is clocked to 48MHz by
	//           init_board() so we don't want to divide this anymore
	pm_init_gclk(GCLK_USBB, GCLK_SOURCE_PLL1, 0);
	pm_enable_gclk(GCLK_USBB);

	usb_init_device();
	usb_enable();

	// USBB has to be enabled before endpoints can be configured
	if (configure_endpoints() < 0)
		return -1;

	// Endpoints have to be configured before enabling USB Device
	// Interrupts in Endpoint Control Register, see datasheet p. 570
	AVR32_USBB.uecon0set = 0xffffffff;
	AVR32_USBB.uecon1set = 0xffffffff;
	AVR32_USBB.uecon2set = 0xffffffff;
	AVR32_USBB.uecon3set = 0xffffffff;
	AVR32_USBB.uecon4set = 0xffffffff;
	AVR32_USBB.uecon5set = 0xffffffff;
	AVR32_USBB.uecon6set = 0xffffffff;

	// Enable all USB General Interrupts
	AVR32_USBB.udinteset = 0xffffffff;

	gpio_set_pin_high(LED);
	usb_attach();

	for (;;) {
	}

	return 0;
}
