/*
 * usb hub driver head file
 *
 * Copyright (C) 1999 Linus Torvalds
 * Copyright (C) 1999 Johannes Erdfelt
 * Copyright (C) 1999 Gregory P. Smith
 * Copyright (C) 2001 Brad Hards (bhards@bigpond.net.au)
 * Copyright (C) 2012 Intel Corp (tianyu.lan@intel.com)
 *
 *  move struct usb_hub to this file.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
#ifndef __HUB_H
#define __HUB_H

#include <linux/usb.h>
#include <linux/usb/ch11.h>
#include <linux/usb/hcd.h>
#include "xhci_ctler_verify.h"

/*
 * Firmware specific cookie identifying a port's location. '0' == no location
 * data available
 */
typedef u32 usb_port_location_t;

struct usb_hub {
	struct device		*intfdev;	/* the "interface" device */
	struct usb_device	*hdev;
	struct kref		kref;
	struct urb		*urb;		/* for interrupt polling pipe */

	/* buffer for urb ... with extra space in case of babble */
	u8			(*buffer)[8];
	union {
		struct usb_hub_status	hub;
		struct usb_port_status	port;
	}			*status;	/* buffer for status reports */
	struct mutex		status_mutex;	/* for the status buffer */

	int			error;		/* last reported error */
	int			nerrors;	/* track consecutive errors */

	unsigned long		event_bits[1];	/* status change bitmask */
	unsigned long		change_bits[1];	/* ports with logical connect
							status change */
	unsigned long		removed_bits[1]; /* ports with a "removed"
							device present */
	unsigned long		wakeup_bits[1];	/* ports that have signaled
							remote wakeup */
	unsigned long		power_bits[1]; /* ports that are powered */
	unsigned long		child_usage_bits[1]; /* ports powered on for
							children */
	unsigned long		warm_reset_bits[1]; /* ports requesting warm
							reset recovery */
#if USB_MAXCHILDREN > 31 /* 8*sizeof(unsigned long) - 1 */
#error event_bits[] is too short!
#endif

	struct usb_hub_descriptor *descriptor;	/* class descriptor */
	struct usb_tt		tt;		/* Transaction Translator */

	unsigned		mA_per_port;	/* current for each child */
#ifdef	CONFIG_PM
	unsigned		wakeup_enabled_descendants;
#endif

	unsigned		limited_power:1;
	unsigned		quiescing:1;
	unsigned		disconnected:1;
	unsigned		in_reset:1;

	unsigned		quirk_check_port_auto_suspend:1;

	unsigned		has_indicators:1;
	u8			indicator[USB_MAXCHILDREN];
	struct delayed_work	leds;
	struct delayed_work	init_work;
	struct work_struct      events;
	struct usb_port		**ports;
};

/**
 * struct usb port - kernel's representation of a usb port
 * @child: usb device attached to the port
 * @dev: generic device interface
 * @port_owner: port's owner
 * @peer: related usb2 and usb3 ports (share the same connector)
 * @req: default pm qos request for hubs without port power control
 * @connect_type: port's connect type
 * @location: opaque representation of platform connector location
 * @status_lock: synchronize port_event() vs usb_port_{suspend|resume}
 * @portnum: port index num based one
 * @is_superspeed cache super-speed status
 */
struct usb_port {
	struct usb_device *child;
	struct device dev;
	struct usb_dev_state *port_owner;
	struct usb_port *peer;
	struct dev_pm_qos_request *req;
	enum usb_port_connect_type connect_type;
	usb_port_location_t location;
	struct mutex status_lock;
	u8 portnum;
	unsigned int is_superspeed:1;
};

#define to_usb_port(_dev) \
	container_of(_dev, struct usb_port, dev)

#endif