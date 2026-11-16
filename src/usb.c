#include "usb.h"
#include "power.h"
#include "util.h"
#include "nvic.h"
#include "string.h"
#include <stdint.h>

#define STR_LANGUAGE_CODES 0x00
#define STR_MANUFACTURER   0x00
#define STR_PRODUCT        0x00

static const char devDescriptor[] = {
	0x12,      // bLength
	0x01,      // bDescriptorType
	0x00,0x02, // Complies with USB Spec. Release (0200h = release 2.0)
	0x02,      // bDeviceClass:    (Communication Device Class)
	0x00,      // bDeviceSubclass: (unused at this time)
	0x00,      // bDeviceProtocol: (unused at this time)
	0x40,      // bMaxPacketSize0
	0x09,0x12, // Vendor ID
	0x01,0x00, // Product ID
	0x01,0x00, // Device release number (0001)
	STR_MANUFACTURER,  // iManufacturer
	STR_PRODUCT,       // iProduct
	0x00,      // iSerialNumber
	0x01       // bNumConfigs
};

static const char cfgDescriptor[] = {
	/* ============== CONFIGURATION 1 =========== */
	/* Configuration 1 descriptor */
	0x09,   // CbLength
	0x02,   // CbDescriptorType
	0x43,   // CwTotalLength 2 EP + Control
	0x00,
	0x02,   // CbNumInterfaces
	0x01,   // CbConfigurationValue
	0x00,   // CiConfiguration
	0x80,   // CbmAttributes (Bus Powered)
	0x4B,   // CMaxPower (150mA max current drawn from bus)

	/* Interface 0 Descriptor: Communication Class Interface */
	0x09, // bLength
	0x04, // bDescriptorType
	0x00, // bInterfaceNumber
	0x00, // bAlternateSetting
	0x01, // bNumEndpoints
	0x02, // bInterfaceClass:       Communication Interface Class
	0x02, // bInterfaceSubclass:    Abstract Control Model
	0x01, // bInterfaceProtocol:    Common AT Commands, V.25ter
	0x00, // iInterface

	/* Header Functional Descriptor */
	0x05, // bFunction Length
	0x24, // bDescriptor type:      CS_INTERFACE
	0x00, // bDescriptor subtype:   Header Functional Descriptor
	0x10, // bcdCDC:1.1
	0x01,

	/* ACM Functional Descriptor */
	0x04, // bFunctionLength
	0x24, // bDescriptor Type:      CS_INTERFACE
	0x02, // bDescriptor Subtype:   Abstract Control Management Functional Descriptor
	0x02, // bmCapabilities:        D1: Device supports the request combination of Set_Line_Coding, Set_Control_Line_State, Get_Line_Coding, and the notification Serial_State

	/* Union Functional Descriptor */
	0x05, // bFunctionLength
	0x24, // bDescriptorType:       CS_INTERFACE
	0x06, // bDescriptor Subtype:   Union Functional Descriptor
	0x00, // bMasterInterface:      Communication Class Interface
	0x01, // bSlaveInterface0:      Data Class Interface

	/* Call Management Functional Descriptor */
	0x05, // bFunctionLength
	0x24, // bDescriptor Type:      CS_INTERFACE
	0x01, // bDescriptor Subtype:   Call Management Functional Descriptor
	0x00, // bmCapabilities:        Device sends/receives call management information only over the Communication Class interface. Device does not handle call management itself
	0x01, // bDataInterface:        Data Class Interface 1

	/* Endpoint 1 descriptor */
	0x07,   // bLength
	0x05,   // bDescriptorType
	0x82,   // bEndpointAddress:    Endpoint 02 - IN
	0x03,   // bmAttributes:        INT
	0x08,   // wMaxPacketSize:      8
	0x00,
	0xFF,   // bInterval

	/* Interface 1 Descriptor: Data Class Interface */
	0x09, // bLength
	0x04, // bDescriptorType
	0x01, // bInterfaceNumber
	0x00, // bAlternateSetting
	0x02, // bNumEndpoints
	0x0A, // bInterfaceClass:       Data Interface Class
	0x00, // bInterfaceSubclass:    not used
	0x00, // bInterfaceProtocol:    No class specific protocol required)
	0x00, // iInterface

	/* Endpoint 1 descriptor */
	0x07,   // bLength
	0x05,   // bDescriptorType
	0x01,   // bEndpointAddress:    Endpoint 01 - OUT
	0x02,   // bmAttributes:        BULK
	0x40, // wMaxPacketSize
	0x00,
	0x00,   // bInterval

	/* Endpoint 2 descriptor */
	0x07,   // bLength
	0x05,   // bDescriptorType
	0x81,   // bEndpointAddress:    Endpoint 01 - IN
	0x02,   // bmAttributes:        BULK
	0x40,   // wMaxPacketSize
	0x00,
	0x00    // bInterval
};

static const char lineCoding[] = {
    // Baudrate, little endian, 115200 baud (0x1c200)
    0x00,
    0xc2,
    0x01,
    0x00,
    // Stop bit (1)
    0x00,
    // Parity type (None)
    0x00,
    // Data bits (8)
    0x08,
};

static char serial_confbuf[7] = {0};
static char usb_buffer[64] = { 0 };
static volatile int dtr_latch = 0;

static void usb_send_control_data(const char *data, int length) {
    uint32_t tosend = 0;
    USBD_EPIN_PTR_N(0) = (unsigned long)usb_buffer;
    
    do {
        tosend = MIN(length, 0x40);
        length -= tosend;
        USBD_EPIN_MAXCNT_N(0) = tosend;

        for (uint32_t i = 0; i < tosend; i ++) {
            usb_buffer[i] = *data++;
        }

        USBD_TASKS_STARTEPIN_N(0) = 1;
        while(USBD_EVENTS_EP0DATADONE == 0) { }
        USBD_EVENTS_EP0DATADONE = 0;
    } while(length);
    USBD_TASKS_EP0STATUS = 1;
}

static uint32_t usb_read_control_data(char *outbuf, uint32_t buflen, uint32_t wLength){
    uint32_t received_total = 0;
    uint32_t rcv = 0;
    USBD_EPOUT_PTR_N(0) = (unsigned long)usb_buffer;

    // Enable useful shorts
    USBD_SHORTS |= (1<<1 | 1<<4); // EP0DATADONE_STARTEPOUT0 | ENDEPOUT0_EP0RCVOUT
    
    USBD_TASKS_EP0RCVOUT = 1;
    while(1) {
        USBD_EPOUT_MAXCNT_N(0) = MIN(MIN(sizeof(usb_buffer), buflen-received_total), wLength-received_total);
        while(USBD_EVENTS_EP0DATADONE == 0) { }
        USBD_EVENTS_EP0DATADONE = 0;
        while(USBD_EVENTS_ENDEPOUT_N(0) == 0) { }
        USBD_EVENTS_ENDEPOUT_N(0) = 0;
        rcv = USBD_EPOUT_AMOUNT_N(0);
        for (uint32_t i = 0; i < rcv; i ++) {
            *outbuf++ = usb_buffer[i];
        }
        received_total += rcv;
        if(received_total == wLength || rcv < 0x40) {
            break;
        }
    }
    USBD_TASKS_EP0STATUS = 1;

    // Disable the shorts
    USBD_SHORTS &= ~(1<<1 | 1<<4); // EP0DATADONE_STARTEPOUT0 | ENDEPOUT0_EP0RCVOUT

    return received_total;
}

void usb_write(const char *buf, int length){
    if(!dtr_latch){
        return;
    }
    uint32_t tosend = 0;
    USBD_EPIN_PTR_N(1) = (unsigned long)usb_buffer;
    
    do {
        tosend = MIN(length, 0x40);
        length -= tosend;
        USBD_EPIN_MAXCNT_N(1) = tosend;

        for (uint32_t i = 0; i < tosend; i ++) {
            usb_buffer[i] = *buf++;
        }

        USBD_TASKS_STARTEPIN_N(1) = 1;
        while(dtr_latch){
            // If the device is disconnected while we wait, we will never receive the event and be stuck forever... :/
            if(((USBD_EPDATASTATUS & (1<<1)) != 0)){
                // W1C register
                USBD_EPDATASTATUS = (1<<1);
                break;
            }
        }
    } while(length && dtr_latch);
}

void USBD_IRQHandler(void){
    uint8_t bmRequestType = 0;
    uint8_t bRequest = 0;
    uint32_t wLength = 0;
    uint32_t wValue = 0;

    if(USBD_EVENTS_EP0SETUP == 1) {
        USBD_EVENTS_EP0SETUP = 0;
        bmRequestType = USBD_BMREQUESTTYPE;
        bRequest = USBD_BREQUEST;
        wLength = USBD_WLENGTHL | (USBD_WLENGTHH << 8);
        wValue = USBD_WVALUEL | (USBD_WVALUEH << 8);
        switch(bRequest){
            case USBD_STD_GET_DESCRIPTOR:
                switch (bmRequestType) {
                    case 0x80:
                        if(wValue == 0x100) {
                            usb_send_control_data(devDescriptor, MIN(wLength, sizeof(devDescriptor)));                            
                        }
                        else if(wValue == 0x200) {
                            usb_send_control_data(cfgDescriptor, MIN(wLength, sizeof(cfgDescriptor)));                            
                        }
                        else{
                            USBD_TASKS_EP0STALL = 1;
                        }
                        break;
                    default:
                        USBD_TASKS_EP0STALL = 1;
                        break;
                }
                break;
            // SET_LINE_CODING
            case 0x20:
                if(bmRequestType == 0x21){
                    usb_read_control_data(serial_confbuf, 7, wLength);
                }else{
                    USBD_TASKS_EP0STALL = 1;
                }
                break;
            // GET_LINE_CODING
            case 0x21:
                if(bmRequestType == 0xa1){
                    // usb_send_control_data(lineCoding, MIN(wLength, sizeof(lineCoding)));
                    usb_send_control_data(serial_confbuf, MIN(wLength, sizeof(serial_confbuf)));
                }else{
                    USBD_TASKS_EP0STALL = 1;
                }
                break;
            // SET_CONTROL_LINE_STATE
            case 0x22:
                if(bmRequestType == 0x21){
                    dtr_latch = wValue&1;
                    USBD_TASKS_EP0STATUS = 1;
                }else{
                    USBD_TASKS_EP0STALL = 1;
                }
                break;
            case USBD_STD_SET_ADDRESS:
                // Handled by hardware, do nothing
                break;
            case USBD_STD_SET_CONFIGURATION:
                USBD_EPINEN |= (1<<1) | (1<<2); // Enable EPIN[1] & EPIN[2]
                USBD_EPOUTEN |= 1<<1; // Enable EPOUT[1]
                USBD_TASKS_EP0STATUS = 1;
                break;
            default:
                USBD_TASKS_EP0STALL = 1;
        }
    }
}

void usb_init(){
    // USB boot sequence is:
    // 1. Power USB detected
    // 2. Enable USBD (with fix)
    // 3. Wait for USB event with event cause "READY"
    // 4. Wait for Power's USB power ready
    // 5. Enable Puul-up 
    while(POWER_EVENTS_USBDETECTED == 0){ }
    POWER_EVENTS_USBDETECTED = 0;

    // https://docs.nordicsemi.com/r/bundle/errata_nrf52840_engc/page/err/nrf52840/engineeringc/latest/anomaly_840_171.html
    if(*(volatile uint32_t *)0x4006EC00 == 0x00000000) {
        *(volatile uint32_t *)0x4006EC00 = 0x00009375;
    }
    *(volatile uint32_t *)0x4006EC14 = 0x000000C0; 
    *(volatile uint32_t *)0x4006EC00 = 0x00009375;

    USBD_ENABLE = 1;

    while(1) {
        if(USBD_EVENTS_USBEVENT != 0){
            USBD_EVENTS_USBEVENT = 0;
            if((USBD_EVENTCAUSE & USBD_EVENTCAUSE_READY) != 0){
                break;
            }
        }
    }

    // https://docs.nordicsemi.com/r/bundle/errata_nrf52840_engc/page/err/nrf52840/engineeringc/latest/anomaly_840_171.html
    if(*(volatile uint32_t *)0x4006EC00 == 0x00000000) {
        *(volatile uint32_t *)0x4006EC00 = 0x00009375;
    }
    *(volatile uint32_t *)0x4006EC14 = 0x00000000; 
    *(volatile uint32_t *)0x4006EC00 = 0x00009375;

    while(POWER_EVENTS_USBPWRRDY == 0) { }
    POWER_EVENTS_USBPWRRDY = 0;

    USBD_USBPULLUP = 1;

    // Enable USB interupts
    USBD_INTENSET = USBD_INT_EP0SETUP_MSK;
    USBD_INTENSET = USBD_INT_EP0DATADONE_MSK;
    NVIC_ISER1 |= (1UL<<7);
    // Set lower pripority than RADIO
    NVIC_IPR_N(39) = (1UL<<5);
}