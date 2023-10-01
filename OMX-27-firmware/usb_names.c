#include "usb_names.h"

#define PRODUCT_NAME      {'o','m','x','-','2','7'}
#define PRODUCT_NAME_LEN   6
#define MANUFACTURER_NAME  {'d','e','n','k','i','-','o','t','o'}
#define MANUFACTURER_NAME_LEN 9

struct usb_string_descriptor_struct usb_string_product_name = {
	2 + PRODUCT_NAME_LEN * 2,
	3,
	PRODUCT_NAME
};

struct usb_string_descriptor_struct usb_string_manufacturer_name = {
	2 + MANUFACTURER_NAME_LEN * 2,
	3,
	MANUFACTURER_NAME
};
