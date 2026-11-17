#include "usb.h"
#include "power.h"
#include "util.h"
#include "nvic.h"
#include "string.h"
#include "ring.h"
#include <stdint.h>

#define USB_RING_SIZE       (1UL<<11)

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

// Control-related
usb_control_machine_t control_machine;
static char usb_control_buffer[64] = { 0 };
static char serial_confbuf[7] = {0};

// Bulk IN
static ring_buff_t usb_ring;
static uint8_t usb_ring_buf[USB_RING_SIZE] = {0};
/// A packet is being sent via Bulk IN endpoint
static uint32_t op_pending = 0;
static uint32_t sending_length = 0;

/// Is there any data waiting for DMA moves
static int dma_busy = 0;

// Misc
static volatile int dtr_latch = 0;

void usb_print_hex8(uint8_t val) {
    char buf[2];
    hex8(buf, val);
    usb_write(buf, sizeof buf);
}

void usb_print_hex32(uint32_t val) {
    char buf[10];
    hex32(buf, val);
    usb_write(buf, 10);
}

void usb_print_dec(unsigned long v) {
    char buf[10];
    unsigned len;
    char *p = utoa_back(v, buf + sizeof buf, &len);
    usb_write(p, len);
}

void usb_hex_dump(const uint8_t *buf, unsigned len){
    char formatted[256] = {0};
    unsigned int written = 0;
    unsigned int taken = 0;
    unsigned int total = 0;

    while(1) {
        taken = hexdump(&buf[total], len-total, formatted, 256, &written, total);
        if(!taken) {
            break;
        }
        total += taken;
        usb_write(formatted, written);
        if(total == len){
            break;
        }
    }
}

void usb_print(const char *str){
    usb_write(str, strlen(str));
}

void usb_println(const char *str){
    usb_print(str);
    usb_print("\r\n");
}


static void usb_send_control_data() {
    USBD_EPIN_PTR_N(0) = (unsigned long)usb_control_buffer;
    int sending = MIN(control_machine.remaining, 0x40);
    USBD_EPIN_MAXCNT_N(0) = sending;
    
    // For sending data, stage it in a RAM buffer so easyDMA can access it.
    for (uint32_t i = 0; i < sending; i ++) {
        usb_control_buffer[i] = *control_machine.ptr++;
    }
    control_machine.remaining-=sending;

    control_machine.ep0_busy = 1;
    USBD_TASKS_STARTEPIN_N(0) = 1;
}

static void usb_read_control_data(){
    USBD_EPOUT_PTR_N(0) = (unsigned long)control_machine.ptr;
    USBD_EPOUT_MAXCNT_N(0) = MIN(control_machine.remaining, 0x40);
    control_machine.out_armed = 1;
    USBD_TASKS_EP0RCVOUT = 1;
}

void usb_write(const uint8_t *buf, int length){
    if(dtr_latch){
        ring_write(&usb_ring, buf, length);
        // Trigger an USB interrupt
        NVIC_ISPR1 = (1UL<<7);
    }
}

/// Peeks the usb ring buffer and starts trasnmitting in EPIN[1]
/// returns 1 if it started a transmission
int usb_tx_start() {
    uint32_t length = 0;
    void *data_ptr = 0;
    if(dtr_latch && !op_pending) {
        // Peek the ring
        ring_peek(&usb_ring, &data_ptr, &length);
        if(length){
            USBD_EPIN_PTR_N(1) = (unsigned long)data_ptr;
            sending_length = MIN(length, 0x40);
            USBD_EPIN_MAXCNT_N(1) = sending_length;

            USBD_TASKS_STARTEPIN_N(1) = 1;
            op_pending = 1;
            return 1;
        }
    }
    return 0;
}

void dma_schedule(){
    if(dma_busy){
        return;
    }
    if(control_machine.state == CONTROL_DATA_IN && control_machine.remaining > 0 && !control_machine.ep0_busy){
        usb_send_control_data();
        dma_busy = 1;
    } else if(control_machine.state == CONTROL_DATA_OUT){
        if(control_machine.out_ready){
            control_machine.out_ready = 0;
            dma_busy = 1;
            USBD_TASKS_STARTEPOUT_N(0) = 1;
        }
        else if(!control_machine.out_armed){
            usb_read_control_data();
        }
    } else if(usb_tx_start()){
        dma_busy = 1;
    }
}

void USBD_IRQHandler(void){
    if(USBD_EVENTS_USBRESET){
        USBD_EVENTS_USBRESET = 0;
        dma_busy = 0;
        op_pending = 0;
        control_machine.ep0_busy = 0;
        control_machine.out_armed = 0;
        control_machine.out_ready = 0;
        dtr_latch = 0;
    }
    
    if(USBD_EVENTS_EP0SETUP == 1) {
        USBD_EVENTS_EP0SETUP = 0;
        // A SETUP should abort whatever is going on
        control_machine.state = CONTROL_IDLE;
        control_machine.ep0_busy = 0;
        control_machine.out_armed = 0;
        control_machine.out_ready = 0;
    
        uint8_t bmRequestType = 0;
        uint8_t bRequest = 0;
        uint32_t wLength = 0;
        uint32_t wValue = 0;

        bmRequestType = USBD_BMREQUESTTYPE;
        bRequest = USBD_BREQUEST;
        wLength = USBD_WLENGTHL | (USBD_WLENGTHH << 8);
        wValue = USBD_WVALUEL | (USBD_WVALUEH << 8);

        switch(bRequest){
            case USBD_STD_GET_DESCRIPTOR:
                switch (bmRequestType) {
                    case 0x80:
                        if(wValue == 0x100) {
                            control_machine.remaining = MIN(wLength, sizeof(devDescriptor));
                            control_machine.ptr = devDescriptor;
                            control_machine.state = CONTROL_DATA_IN;
                        }
                        else if(wValue == 0x200) {
                            control_machine.remaining = MIN(wLength, sizeof(cfgDescriptor));
                            control_machine.ptr = cfgDescriptor;
                            control_machine.state = CONTROL_DATA_IN;
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
                    control_machine.remaining = MIN(wLength, 7);
                    control_machine.ptr = serial_confbuf;
                    control_machine.state = CONTROL_DATA_OUT;
                }else{
                    USBD_TASKS_EP0STALL = 1;
                }
                break;
            // GET_LINE_CODING
            case 0x21:
                if(bmRequestType == 0xa1){
                    control_machine.remaining = MIN(wLength, sizeof(serial_confbuf));
                    control_machine.ptr = serial_confbuf;
                    control_machine.state = CONTROL_DATA_IN;
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
    if(USBD_EVENTS_EP0DATADONE){
        USBD_EVENTS_EP0DATADONE = 0;
        if(control_machine.state == CONTROL_DATA_IN){
            control_machine.ep0_busy = 0;
            if(control_machine.remaining == 0){
                USBD_TASKS_EP0STATUS = 1;
                control_machine.state = CONTROL_IDLE;
            }
        } else if(control_machine.state == CONTROL_DATA_OUT){
            control_machine.out_ready = 1;
        }
    }
    if(USBD_EVENTS_ENDEPIN_N(0)){
        USBD_EVENTS_ENDEPIN_N(0) = 0;
        dma_busy = 0;
    }
    if(USBD_EVENTS_ENDEPOUT_N(0)){
        USBD_EVENTS_ENDEPOUT_N(0) = 0;
        dma_busy = 0;
        control_machine.out_armed = 0;
        uint32_t rcv = USBD_EPOUT_AMOUNT_N(0);
        control_machine.ptr += rcv;
        control_machine.remaining -= rcv;
        if(control_machine.remaining == 0){
            USBD_TASKS_EP0STATUS = 1;
            control_machine.state = CONTROL_IDLE;
        }
    }
    // Data has been sent, consume in the ring
    if(USBD_EVENTS_ENDEPIN_N(1)){
        USBD_EVENTS_ENDEPIN_N(1) = 0;
        ring_consume(&usb_ring, sending_length);
        dma_busy = 0;
    }
    // Data has been ACK's, we can send again
    if(USBD_EVENTS_EPDATA){
        USBD_EVENTS_EPDATA = 0;
        if(((USBD_EPDATASTATUS & (1<<1)) != 0)){
            // W1C register
            USBD_EPDATASTATUS = (1<<1);
            op_pending = 0;
        }
    }
    dma_schedule();
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
    USBD_INTENSET = USBD_INT_ENDEPIN_1_MSK;
    USBD_INTENSET = USBD_INT_ENDEPOUT_1_MSK;
    USBD_INTENSET = USBD_INT_ENDEPIN_0_MSK;
    USBD_INTENSET = USBD_INT_ENDEPOUT_0_MSK;
    USBD_INTENSET = USBD_INT_EPDATA_MSK;
    USBD_INTENSET = USBD_INT_USBRESET_MSK;
    NVIC_ISER1 |= (1UL<<7);
    // Set lower pripority than RADIO
    NVIC_IPR_N(39) = (1UL<<5);

    // Init the ring buffer
    ring_init(&usb_ring, usb_ring_buf, USB_RING_SIZE);
}