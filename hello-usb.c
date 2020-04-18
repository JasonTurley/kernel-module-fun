/*
 * hello-usb.c - prints a string when a usb device is connected and
 * disconnected.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

/*
 * The Linux USB subsystem does not support non-GLP kernel drivers
 */
MODULE_LICENSE("GPL");

/*
 * In this example I am using a USB Mouse with Vendor ID 413c and
 * Product ID 301a
 */

#define VENDOR_ID 0x413c
#define PRODUCT_ID 0x301a

static int hello_usb_probe(struct usb_interface *interface,
					const struct usb_device_id *id)
{
	printk(KERN_INFO "USB drive (%04X:%04X) plugged\n", id->idVendor,
                                id->idProduct);
	return 0;
}

static void hello_usb_disconnect(struct usb_interface *interface)
{
    printk(KERN_INFO "USB drive removed\n");
}

/*
 *
 */
static struct usb_device_id hello_usb_table[] =
{
	{ USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
	{} /* Terminating entry */
};

/* Register the usb in the device table */
MODULE_DEVICE_TABLE(usb, hello_usb_table);

static struct usb_driver hello_usb_driver =
{
	.name = "hello_usb_driver",
	.id_table = hello_usb_table,
	.probe = hello_usb_probe,
	.disconnect = hello_usb_disconnect,
};

static int __init hello_usb_init(void)
{
	int error;

	/* register this driver with the USB subsystem */
	error = usb_register_driver(&hello_usb_driver, THIS_MODULE, "hello_usb");
	if (error)
		printk(KERN_ALERT "usb_register_driver failed: error num %d", error);
	return error;
}

static void __exit hello_usb_exit(void)
{
	/* deregister this driver from the USB subsystem */
	usb_deregister(&hello_usb_driver);
}

module_init(hello_usb_init);
module_exit(hello_usb_exit);
