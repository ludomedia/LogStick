/* LogStick
 * Based on project: USB PCB Business Card
 * License: GNU GPL v2
 */


// Example of AVRDUDE ARGS WITH ATMEL STUDIO
// Programm
// -pattiny85 -C"C:\$(Device)Program Files (x86)\Arduino\hardware\tools\avr\etc/avrdude.conf" -v -v -carduino -P\\.\COM4 -b19200 -Uflash:w:"$(ProjectDir)Debug\$(ItemFileName).hex":i
// Fuse
//-pattiny85 -P\\.\COM4 -b19200 -carduino -U lfuse:w:0xE1:m -U hfuse:w:0xdf:m -C"C:\$(Device)Program Files (x86)\Arduino\hardware\tools\avr\etc/avrdude.conf" -v -v

#define F_CPU 16500000
//#define DIGISPARK 1

#if defined(DIGISPARK)
#	define LED_PIN	PB1
#	define SWITCH_PIN PB0
#	define SENSOR_PIN PB2
#else
#	define LED_PIN	PB4
#	define SWITCH_PIN PB1
#	define SENSOR_PIN PB3
#endif


 // please see http://www.frank-zhao.com/card/
// note, for ATtiny MCUs, fuses -U lfuse:w:0xE1:m -U hfuse:w:0xDF:m -U efuse:w:0xFF:m -U lock:w:0xFF:m
// note, write to ATtiny at a low ISP frequency

// required avr-libc modules, see http://www.nongnu.org/avr-libc/user-manual/modules.html
#include <avr/io.h> // allows access to AVR hardware registers
#include <avr/interrupt.h> // allows enabling/disabling and declaring interrupts
#include <util/delay.h> // includes delay functions
#include <avr/wdt.h> // allows enabling/disabling watchdog timer
#include <avr/pgmspace.h> // descriptor must be stored in flash memory
#include <avr/eeprom.h> // text file and calibration data is stored in EEPROM
#include <stdio.h> // allows streaming strings

// configure settings for V-USB then include the V-USB driver so V-USB uses your settings
#include "usbconfig.h"
#include "usbdrv.h"

// compilation settings check
#if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)
#if F_CPU != 16500000
#error "ATtiny MCU used but not at 16.5 KHz"
#endif
#else
#if F_CPU != 12000000
#error "Clock speed is not 12 KHz"
#endif
#endif

// USB HID report descriptor for boot protocol keyboard
// see HID1_11.pdf appendix B section 1
// USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH is defined in usbconfig
PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)(224)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs) ; Modifier byte
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs) ; Reserved byte
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs) ; LED report
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs) ; LED report padding
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))(0)
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)(101)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};

// data structure for boot protocol keyboard report
// see HID1_11.pdf appendix B section 1
typedef struct {
	uint8_t modifier;
	uint8_t reserved;
	uint8_t keycode[6];
} keyboard_report_t;

// global variables

static keyboard_report_t keyboard_report;
#define keyboard_report_reset() keyboard_report.modifier=0;keyboard_report.reserved=0;keyboard_report.keycode[0]=0;keyboard_report.keycode[1]=0;keyboard_report.keycode[2]=0;keyboard_report.keycode[3]=0;keyboard_report.keycode[4]=0;keyboard_report.keycode[5]=0;
static uint8_t idle_rate = 500 / 4; // see HID1_11.pdf sect 7.2.4
static uint8_t protocol_version = 0; // see HID1_11.pdf sect 7.2.6
static uint8_t LED_state = 0; // see HID1_11.pdf appendix B section 1
static uint8_t blink_count = 0; // keep track of how many times caps lock have toggled

// see http://vusb.wikidot.com/driver-api
// constants are found in usbdrv.h
usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
	// see HID1_11.pdf sect 7.2 and http://vusb.wikidot.com/driver-api
	usbRequest_t *rq = (void *)data;

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS)
		return 0; // ignore request if it's not a class specific request

	// see HID1_11.pdf sect 7.2
	switch (rq->bRequest)
	{
		case USBRQ_HID_GET_IDLE:
			usbMsgPtr = &idle_rate; // send data starting from this byte
			return 1; // send 1 byte
		case USBRQ_HID_SET_IDLE:
			idle_rate = rq->wValue.bytes[1]; // read in idle rate
			return 0; // send nothing
		case USBRQ_HID_GET_PROTOCOL:
			usbMsgPtr = &protocol_version; // send data starting from this byte
			return 1; // send 1 byte
		case USBRQ_HID_SET_PROTOCOL:
			protocol_version = rq->wValue.bytes[1];
			return 0; // send nothing
		case USBRQ_HID_GET_REPORT:
			usbMsgPtr = &keyboard_report; // send the report data
			return sizeof(keyboard_report);
		case USBRQ_HID_SET_REPORT:
			if (rq->wLength.word == 1) // check data is available
			{
				// 1 byte, we don't check report type (it can only be output or feature)
				// we never implemented "feature" reports so it can't be feature
				// so assume "output" reports
				// this means set LED status
				// since it's the only one in the descriptor
				return USB_NO_MSG; // send nothing but call usbFunctionWrite
			}
			else // no data or do not understand data, ignore
			{
				return 0; // send nothing
			}
		default: // do not understand data, ignore
			return 0; // send nothing
	}
}

// see http://vusb.wikidot.com/driver-api
usbMsgLen_t usbFunctionWrite(uint8_t * data, uchar len)
{
	if (data[0] != LED_state)
	{
		// increment count when LED has toggled
		blink_count = blink_count < 10 ? blink_count + 1 : blink_count;
	}
	
	LED_state = data[0];
	
	#if !defined(__AVR_ATtiny85__) && !defined(__AVR_ATtiny45__) && !defined(__AVR_ATtiny25__)
	 // LED lights for debug
	if (bit_is_set(LED_state, 1))
	{
		DDRD |= _BV(0);
		PORTD |= _BV(0);
	}
	else
	{
		DDRD |= _BV(0);
		PORTD &= ~_BV(0);
	}
	#endif
	
	return 1; // 1 byte read
}

#if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)
/* ------------------------------------------------------------------------- */
/* ------------------------ Oscillator Calibration ------------------------- */
/* ------------------------------------------------------------------------- */
// section copied from EasyLogger
/* Calibrate the RC oscillator to 8.25 MHz. The core clock of 16.5 MHz is
 * derived from the 66 MHz peripheral clock by dividing. Our timing reference
 * is the Start Of Frame signal (a single SE0 bit) available immediately after
 * a USB RESET. We first do a binary search for the OSCCAL value and then
 * optimize this value with a neighboorhod search.
 * This algorithm may also be used to calibrate the RC oscillator directly to
 * 12 MHz (no PLL involved, can therefore be used on almost ALL AVRs), but this
 * is wide outside the spec for the OSCCAL value and the required precision for
 * the 12 MHz clock! Use the RC oscillator calibrated to 12 MHz for
 * experimental purposes only!
 */
static void calibrateOscillator(void)
{
	uchar       step = 128;
	uchar       trialValue = 0, optimumValue;
	int         x, optimumDev, targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

    /* do a binary search: */
    do{
        OSCCAL = trialValue + step;
        x = usbMeasureFrameLength();    /* proportional to current real frequency */
        if(x < targetValue)             /* frequency still too low */
            trialValue += step;
        step >>= 1;
    }while(step > 0);
    /* We have a precision of +/- 1 for optimum OSCCAL here */
    /* now do a neighborhood search for optimum value */
    optimumValue = trialValue;
    optimumDev = x; /* this is certainly far away from optimum */
    for(OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++){
        x = usbMeasureFrameLength() - targetValue;
        if(x < 0)
            x = -x;
        if(x < optimumDev){
            optimumDev = x;
            optimumValue = OSCCAL;
        }
    }
    OSCCAL = optimumValue;
}
/*
Note: This calibration algorithm may try OSCCAL values of up to 192 even if
the optimum value is far below 192. It may therefore exceed the allowed clock
frequency of the CPU in low voltage designs!
You may replace this search algorithm with any other algorithm you like if
you have additional constraints such as a maximum CPU clock.
For version 5.x RC oscillators (those with a split range of 2x128 steps, e.g.
ATTiny25, ATTiny45, ATTiny85), it may be useful to search for the optimum in
both regions.
*/

void usbEventResetReady(void)
{
    calibrateOscillator();
    eeprom_update_byte(0, OSCCAL);   /* store the calibrated value in EEPROM */
}
#endif

// translates ASCII to appropriate keyboard report, taking into consideration the status of caps lock
void ASCII_to_keycode(uint8_t ascii)
{
	keyboard_report.keycode[0] = 0x00;
	keyboard_report.modifier = 0x00;
	
	// see scancode.doc appendix C
	
	if (ascii >= 'A' && ascii <= 'Z')
	{
		keyboard_report.keycode[0] = 4 + ascii - 'A'; // set letter
		if (bit_is_set(LED_state, 1)) // if caps is on
		{
			keyboard_report.modifier = 0x00; // no shift
		}
		else
		{
			keyboard_report.modifier = _BV(1); // hold shift // hold shift
		}
	}
	else if (ascii >= 'a' && ascii <= 'z')
	{
		keyboard_report.keycode[0] = 4 + ascii - 'a'; // set letter
		if (bit_is_set(LED_state, 1)) // if caps is on
		{
			keyboard_report.modifier = _BV(1); // hold shift // hold shift
		}
		else
		{
			keyboard_report.modifier = 0x00; // no shift
		}
	}
	else if (ascii >= '0' && ascii <= '9')
	{
		keyboard_report.modifier = 0x00;
		if (ascii == '0')
		{
			keyboard_report.keycode[0] = 0x27;
		}
		else
		{
			keyboard_report.keycode[0] = 30 + ascii - '1'; 
		}
	}
	else
	{
		switch (ascii) // convert ascii to keycode according to documentation
		{
			case ';':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case ',':
				keyboard_report.keycode[0] = 0x36;
				break;
			case ':':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '.':
				keyboard_report.keycode[0] = 0x37;
				break;
			case '_':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '-':
				keyboard_report.keycode[0] = 0x38; // 0x2D;
				break;
			case ' ':
				keyboard_report.keycode[0] = 0x2C;
				break;
			case '\t':
				keyboard_report.keycode[0] = 0x2B;
				break;
			case '\n':
				keyboard_report.keycode[0] = 0x28;
				break;

			case '!':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 1;
				break;
			case '@':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 2;
				break;
			case '#':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 3;
				break;
			case '$':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 4;
				break;
			case '%':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 5;
				break;
			case '^':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 6;
				break;
			case '&':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 7;
				break;
			case '*':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 8;
				break;
			case '(':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 29 + 9;
				break;
			case ')':
				keyboard_report.modifier = _BV(1); // hold shift
				keyboard_report.keycode[0] = 0x27;
				break;
			case '~':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '`':
				keyboard_report.keycode[0] = 0x35;
				break;
			case '+':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '=':
				keyboard_report.keycode[0] = 0x2E;
				break;
			case '{':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '[':
				keyboard_report.keycode[0] = 0x2F;
				break;
			case '}':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case ']':
				keyboard_report.keycode[0] = 0x30;
				break;
			case '|':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '\\':
				keyboard_report.keycode[0] = 0x31;
				break;
			case '"':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '\'':
				keyboard_report.keycode[0] = 0x34;
				break;
			case '?':
				keyboard_report.modifier = _BV(1); // hold shift
				// fall through
			case '/':
				keyboard_report.keycode[0] = 0x38;
				break;
		}
	}
}

void send_report_once()
{
	// perform usb background tasks until the report can be sent, then send it
	while (1)
	{
		usbPoll(); // this needs to be called at least once every 10 ms
		if (usbInterruptIsReady())
		{
			usbSetInterrupt(&keyboard_report, sizeof(keyboard_report)); // send
			break;
			
			// see http://vusb.wikidot.com/driver-api
		}
	}
}

// stdio's stream will use this funct to type out characters in a string
void type_out_char(uint8_t ascii, FILE *stream)
{
	ASCII_to_keycode(ascii);
	send_report_once();
	keyboard_report_reset(); // release keys
	send_report_once();
}

static FILE mystdout = FDEV_SETUP_STREAM(type_out_char, NULL, _FDEV_SETUP_WRITE); // setup writing stream

#define SENSOR_HIGH (PINB & (1<<SENSOR_PIN))
#define SENSOR_LOW (!SENSOR_HIGH)

#define TIMEOUT 1000
// 116 loopCnt for a one, 42 loopCnt for a zero (mesured)
#define LIMIT 921

uchar sensor_bytes[5];

uchar sensorRead() {

	// INIT BUFFERVAR TO RECEIVE DATA
	uint8_t cnt = 7;
	uint8_t idx = 0;

	// EMPTY BUFFER
	//for (int i=0; i< 5; i++) bits[i] = 0;

	// REQUEST SAMPLE
	// set pin to low as output
	PORTB &= ~(1<<SENSOR_PIN);
	DDRB |= (1<<SENSOR_PIN);
	_delay_ms(2);
	// set pin as input with pullup
	DDRB &= ~(1<<SENSOR_PIN);
	PORTB |= (1<<SENSOR_PIN);
	_delay_us(10);

	unsigned int loopCnt;

	// Wait first falling edge
	loopCnt = TIMEOUT;
	while(SENSOR_HIGH) if(loopCnt-- == 0) return 0;

	// Wait rising edge
	loopCnt = TIMEOUT;
	while(SENSOR_LOW) if(loopCnt-- == 0) return 0;

	// Wait falling edge
	loopCnt = TIMEOUT;
	while(SENSOR_HIGH) if(loopCnt-- == 0) return 0;

	uchar index;
	uchar i;
	// READ THE OUTPUT - 40 BITS => 5 BYTES
	for (i=0; i<40; i++) {
		// wait rising edge
		loopCnt = TIMEOUT;
		while(SENSOR_LOW) if(loopCnt-- == 0) return 0;
		// wait falling edge, mesure high level length
		loopCnt = TIMEOUT;
		while(SENSOR_HIGH) if(loopCnt-- == 0) return 0;
		index = i>>3;
		sensor_bytes[index] <<= 1;
		if(loopCnt<LIMIT) sensor_bytes[index] |= 1;
	}

	for(i=0;i<4;i++) sensor_bytes[4] -= sensor_bytes[i];
	return (sensor_bytes[4]==0) ? 1 : 0;
}

//	eeprom_write_word(4, 20);
//	eeprom_write_word(6, 1345);
	
#define LED_ON (PORTB |= (1<<LED_PIN))
#define LED_OFF	(PORTB &= ~(1<<LED_PIN))
#define ACQ_MODE (PINB & (1<<SWITCH_PIN))
//#define LOG_INTERVAL (2)
#define LOG_INTERVAL (60*60)
#define NB_RECORDS 72
#define START_PTR 12
// Attention il faut 4 octet de libre depuis MAX_PTR (pour le marquage 0x7FFF)
// Nb max pour NB_RECORDS = ((EEPROM_SIZE - START_PTR) / 4) - 1 Soit pour AtTiny85: 124
#define MAX_PTR (START_PTR + (NB_RECORDS * 4))

int ptr; // pointeur d'�criture dans le tampon circulaire des mesures

void initPtr() {
	for(ptr=START_PTR;ptr<MAX_PTR;ptr+=4) {
		if(eeprom_read_word(ptr)==0x7FFF) return;
	}
	ptr = START_PTR;
}

void write_record(int humidity, int temperature) {
	eeprom_write_word(ptr + 4, 0x7FFF); // place la marque de la prochaine �criture	
	eeprom_write_word(ptr + 0, humidity);
	eeprom_write_word(ptr + 2, temperature);	
	ptr += 4;
}

int main()
{
	DDRB |= 1<<LED_PIN; // led pin as output
	PORTB |= 1<<SWITCH_PIN; // pullup for switch
	LED_OFF;
	_delay_ms(2);
	initPtr(); // cherche le point d'�criture dans le tampon
	
	if(ACQ_MODE) {
		int cnt = 0;
		int seconds = LOG_INTERVAL - 5; // first mesurement: 5 seconds after power on
		write_record(0x7F00, 0); // place a power on mark
		for(;;) {			
			if(ACQ_MODE) {				

				_delay_ms(20); // TODO work with timer interrupt

				if(cnt++>50) { // augmente le compteur de secondes
					seconds += 1;
					cnt = 0;
				}
								
				if(seconds>=LOG_INTERVAL) {
					if(sensorRead()) {
						write_record(sensor_bytes[0]<<8 | sensor_bytes[1], sensor_bytes[2]<<8 | sensor_bytes[3]);
						// clear le compteur de seconde uniquement en cas de succ�s de lecture (ceci permet de tol�rer des �chec de lecture du capteur)
						seconds = 0; 
					}
				}

				// Led clignotante pour signaler le mode aquisition
				if((cnt%25)>22) LED_ON; 
				else LED_OFF;

			}
			else {
				LED_OFF;
			}	
		}
	
	}		
	else {
		#if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)
		uint8_t calibrationValue = eeprom_read_byte(0); /* calibration value from last time */
		if (calibrationValue != 0xFF)
		{
			OSCCAL = calibrationValue;
		}
		#endif
	
		stdout = &mystdout; // set default stream
	
		// initialize report (I never assume it's initialized to 0 automatically)
		keyboard_report_reset();
	
		wdt_disable(); // disable watchdog, good habit if you don't use it
	
		// enforce USB re-enumeration by pretending to disconnect and reconnect
		usbDeviceDisconnect();
		_delay_ms(250);
		usbDeviceConnect();
	
		// initialize various modules
		usbInit();
	
		sei(); // enable interrupts

		while (1) // main loop, do forever
		{
			if (blink_count > 2) // activated by blinking lights
			{
				#if !defined(__AVR_ATtiny85__) && !defined(__AVR_ATtiny45__) && !defined(__AVR_ATtiny25__)
				DDRD |= _BV(1); // LED lights for debug
				PORTD |= _BV(1);
				#endif
				puts_P(PSTR("----------------------------------"));
				printf("LogStick 1.0 interval %d seconds\n", LOG_INTERVAL);
				if(ACQ_MODE) printf("Ptr %x\n", ptr);
				puts_P(PSTR("----------------------------------"));
				// PLACE TEXT HERE
				//puts_P(PSTR(" ")); // test size
				int humidity;
				int temperature;
				int p = ptr; // pointeur de la prochaine �criture en mode ACQ
				for(;;) {
					p -= 4;
					if(p<START_PTR) p = MAX_PTR-4;
					if(p==ptr) break;
					humidity = eeprom_read_word(p);
					temperature = eeprom_read_word(p+2);
					if(humidity==0x7F00) {
						if(ACQ_MODE) printf("%x:", p);
						puts_P(PSTR("---POWER ON---"));
					}						
					else if(humidity==0x7FFF) {
						puts_P(PSTR("---PTR MARK---")); // ne devrait pas se produire
					}						
					else {
						if(ACQ_MODE) printf("%x:%d.%d,%d.%d\n", p, humidity/10, humidity%10, temperature/10, temperature%10);
						else printf("%d.%d,%d.%d\n", humidity/10, humidity%10, temperature/10, temperature%10);
					}					
				}
				
				puts_P(PSTR("------------------------------END-"));
			
				blink_count = 0; // reset
			}
		
			// perform usb related background tasks
			usbPoll(); // this needs to be called at least once every 10 ms
			// this is also called in send_report_once
		}
	}	
	return 0;
}