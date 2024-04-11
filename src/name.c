// To give your project a unique name, this code must be
// placed into a .c file (its own tab).  It can not be in
// a .cpp file or your main sketch (the .ino file).

#include "usb_names.h"

// Edit these lines to create your own name.  The length must
// match the number of characters in your custom name.

#ifdef EFARMASTER


#define MIDI_NAME   {'E','k','d','a','h','l',' ','F','A','R',' ','M','a','s','t','e', 'r'}
#define MIDI_NAME_LEN  17

// Do not change this part.  This exact format is required by USB.

struct usb_string_descriptor_struct usb_string_product_name = {
        2 + MIDI_NAME_LEN * 2,
        3,
        MIDI_NAME
};


#define MANUFACTURER_NAME { 'K', 'N', 'A', 'S' }
#define MANUFACTURER_NAME_LEN 4
/*
#define PRODUCT_NAME { 'E', 'k', 'd', 'a', 'h', 'l', ' ', 'F', 'A', 'R', ' ', 'M', 'a', 's', 't', 'e', 'r'	}
#define PRODUCT_NAME_LEN 17
*/
#define SERIAL_NUMBER { 'E', 'F', 'A', 'R', 'M', 'A', 'S', 'T', 'E', 'R', '0' }
#define SERIAL_NUMBER_LEN 11

struct usb_string_descriptor_struct usb_string_manufacturer_name = {
	2 + MANUFACTURER_NAME_LEN * 2,
	3,
	MANUFACTURER_NAME};
/*
struct usb_string_descriptor_struct usb_string_product_name = {
	2 + PRODUCT_NAME_LEN * 2,
	3,
	PRODUCT_NAME};
*/
struct usb_string_descriptor_struct usb_string_serial_number = {
	2 + SERIAL_NUMBER_LEN * 2,
	3,
	SERIAL_NUMBER};

#else

#define MIDI_NAME   {'E','k','d','a','h','l',' ','F','A','R',' ','S','l','a','v','e'}
#define MIDI_NAME_LEN  16

// Do not change this part.  This exact format is required by USB.

struct usb_string_descriptor_struct usb_string_product_name = {
        2 + MIDI_NAME_LEN * 2,
        3,
        MIDI_NAME
};


#define MANUFACTURER_NAME { 'K', 'N', 'A', 'S' }
#define MANUFACTURER_NAME_LEN 4
/*
#define PRODUCT_NAME { 'E', 'k', 'd', 'a', 'h', 'l', ' ', 'F', 'A', 'R', ' ', 'S', 'l', 'a', 'v', 'e'	}
#define PRODUCT_NAME_LEN 16
*/
#define SERIAL_NUMBER { 'E', 'F', 'A', 'R', 'S', 'L', 'A', 'V', 'E', '0' }
#define SERIAL_NUMBER_LEN 10

struct usb_string_descriptor_struct usb_string_manufacturer_name = {
	2 + MANUFACTURER_NAME_LEN * 2,
	3,
	MANUFACTURER_NAME};
/*
struct usb_string_descriptor_struct usb_string_product_name = {
	2 + PRODUCT_NAME_LEN * 2,
	3,
	PRODUCT_NAME};
*/
struct usb_string_descriptor_struct usb_string_serial_number = {
	2 + SERIAL_NUMBER_LEN * 2,
	3,
	SERIAL_NUMBER};

#endif
