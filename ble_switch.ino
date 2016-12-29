/* 
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <kyet@me.com> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.         Isaac K. Ko
 * ----------------------------------------------------------------------------
 */

#include <stdarg.h>
#include <SoftwareSerial.h>
#include <PacketSerial.h>

////////////////////
// EDIT THESE...

// device type
#define SWITCH
//#define DIMMING

// undefine for release
#define __DEBUG__

////////////////////

#define BLE_DATA_MAX 20

#define TYPE_RAW     0x01
#define TYPE_DIMMING 0x02

#define TYPE_UNDEF3  0x40
#define TYPE_UNDEF2  0x80
#define TYPE_UNDEF1  0xFF

SoftwareSerial bleSerial(10, 11); // Rx, Tx (Connect reverse order)
PacketSerial   pktSerial;

#if defined(SWITCH)
const int outlet[] = {2, 4, 7, 8, 12, 13};
#elif defined(DIMMING)
const int outlet[] = {3, 5, 6, 9};
#else
#error "MODE NOT DEFINED"
#endif

const int nOutlet = sizeof(outlet) / sizeof(outlet[0]);

void setup() 
{
	Serial.begin(9600);

	// wait for serial port to connect
	// Needed for native USB port only
	while (!Serial) { ; }

#if defined(SWITCH)
	for (int i=0; i<nOutlet; i++)
		pinMode(outlet[i], OUTPUT);
#endif

	bleSerial.begin(9600);

	pktSerial.setPacketHandler(&bleParser);
	pktSerial.begin(&bleSerial);
}

/* event handler */
typedef struct _portValue {
	uint8_t port;
	uint8_t value;
} portValue;

void bleRaw(uint8_t *data, uint8_t sz)
{
	portValue *pv = NULL;

	/* sanity check */
	if (sz < 2)             { return; }
	if ((sz % 2) != 0)      { return; }
	if ((sz / 2) > nOutlet) { return; }

	dumpPkt(data, sz);

	for (int i=0; i<sz; i+=2)
	{
		pv = (portValue *)(data + i);

		if (pv->port == 0) { continue; }
		pv->port--;

#if defined(SWITCH)
		if (pv->value >= 0x80)
		{
			digitalWrite(outlet[pv->port], HIGH);
			syslog("LED %d(%dpin) ON(0x%02X)", 
					pv->port, outlet[pv->port], pv->value);
		}
		else
		{
			digitalWrite(outlet[pv->port], LOW);
			syslog("LED %d(%dpin) OFF(0x%02X)",
					pv->port, outlet[pv->port], pv->value);
		}
#else
		AnalogWrite(outlet[pv->port], pv->value);
		syslog("LED %d(%dpin) Value(0x%02X)",
				pv->port, outlet[pv->port], pv->value);
#endif
	}
}

void bleDimming(uint8_t *data, uint8_t sz)
{

}

void bleParser(const uint8_t* buffer, size_t size)
{
	uint8_t datagram[BLE_DATA_MAX] = {0};
	uint8_t type, sz;
	char buf[128];

	if (buffer == NULL)
		return; 

	if (size > BLE_DATA_MAX)
		return; 

	dumpPkt(buffer, size);
	memcpy(datagram, buffer, size); 

	// parse header
	type = datagram[0];
	sz   = datagram[1];

	if (sz < 2 || sz > BLE_DATA_MAX)
		return;

	switch(type)
	{
		case TYPE_RAW:
			bleRaw(datagram + 2, sz - 2);
			break;
		case TYPE_DIMMING:
#if DIMMING
			bleDimming(datagram + 2, sz - 2);
#endif
			break;
		default:
			break;
	}
}

void loop() 
{
	pktSerial.update();
}

void dumpPkt(const uint8_t* packet, size_t size)
{
#if defined(__DEBUG__)
	char buf[10] = "";

	Serial.print("DUMP ");

	for(int i=0; i<size; i++)
	{
		sprintf(buf, "[%02X] ", packet[i]);
		Serial.print(buf);
	}

	Serial.println();
#endif
}

void syslog(char *fmt, ... )
{
#if defined(__DEBUG__)
	char buf[128];
	va_list args;
	
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), (const char *)fmt, args);
	va_end(args);

	Serial.println(buf);
#endif
}
