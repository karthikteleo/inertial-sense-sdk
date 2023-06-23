/*
MIT LICENSE

Copyright (c) 2014-2023 Inertial Sense, Inc. - http://inertialsense.com

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "ISComm.h"

typedef union 
{
	uint16_t ck;
	struct
	{
		uint8_t a;	// Lower 8 bits
		uint8_t b;	// Upper 8 bits
	};	
} checksum16_u;

const unsigned int g_validBaudRates[IS_BAUDRATE_COUNT] = {
	// Actual on uINS:
	IS_BAUDRATE_18750000,   // 18750000 (uINS ser1 only)
	IS_BAUDRATE_9375000,    // 9375000
	IS_BAUDRATE_3125000,    // 3125000
	IS_BAUDRATE_921600,     // 937734 (default)
	IS_BAUDRATE_460800,     // 468600
	IS_BAUDRATE_230400,     // 232700
	IS_BAUDRATE_115200,
	IS_BAUDRATE_57600,
	IS_BAUDRATE_38400,
	IS_BAUDRATE_19200,
	IS_BAUDRATE_9600 
};


/**
* Calculate 24 bit crc used in formats like RTCM3 - note that no bounds checking is done on buffer
* @param buffer the buffer to calculate the CRC for
* @param len the number of bytes to calculate the CRC for
* @return the CRC value
*/
unsigned int calculate24BitCRCQ(unsigned char* buffer, unsigned int len)
{
	static const unsigned int TABLE_CRC24Q[] =
	{
		0x000000,0x864CFB,0x8AD50D,0x0C99F6,0x93E6E1,0x15AA1A,0x1933EC,0x9F7F17,
		0xA18139,0x27CDC2,0x2B5434,0xAD18CF,0x3267D8,0xB42B23,0xB8B2D5,0x3EFE2E,
		0xC54E89,0x430272,0x4F9B84,0xC9D77F,0x56A868,0xD0E493,0xDC7D65,0x5A319E,
		0x64CFB0,0xE2834B,0xEE1ABD,0x685646,0xF72951,0x7165AA,0x7DFC5C,0xFBB0A7,
		0x0CD1E9,0x8A9D12,0x8604E4,0x00481F,0x9F3708,0x197BF3,0x15E205,0x93AEFE,
		0xAD50D0,0x2B1C2B,0x2785DD,0xA1C926,0x3EB631,0xB8FACA,0xB4633C,0x322FC7,
		0xC99F60,0x4FD39B,0x434A6D,0xC50696,0x5A7981,0xDC357A,0xD0AC8C,0x56E077,
		0x681E59,0xEE52A2,0xE2CB54,0x6487AF,0xFBF8B8,0x7DB443,0x712DB5,0xF7614E,
		0x19A3D2,0x9FEF29,0x9376DF,0x153A24,0x8A4533,0x0C09C8,0x00903E,0x86DCC5,
		0xB822EB,0x3E6E10,0x32F7E6,0xB4BB1D,0x2BC40A,0xAD88F1,0xA11107,0x275DFC,
		0xDCED5B,0x5AA1A0,0x563856,0xD074AD,0x4F0BBA,0xC94741,0xC5DEB7,0x43924C,
		0x7D6C62,0xFB2099,0xF7B96F,0x71F594,0xEE8A83,0x68C678,0x645F8E,0xE21375,
		0x15723B,0x933EC0,0x9FA736,0x19EBCD,0x8694DA,0x00D821,0x0C41D7,0x8A0D2C,
		0xB4F302,0x32BFF9,0x3E260F,0xB86AF4,0x2715E3,0xA15918,0xADC0EE,0x2B8C15,
		0xD03CB2,0x567049,0x5AE9BF,0xDCA544,0x43DA53,0xC596A8,0xC90F5E,0x4F43A5,
		0x71BD8B,0xF7F170,0xFB6886,0x7D247D,0xE25B6A,0x641791,0x688E67,0xEEC29C,
		0x3347A4,0xB50B5F,0xB992A9,0x3FDE52,0xA0A145,0x26EDBE,0x2A7448,0xAC38B3,
		0x92C69D,0x148A66,0x181390,0x9E5F6B,0x01207C,0x876C87,0x8BF571,0x0DB98A,
		0xF6092D,0x7045D6,0x7CDC20,0xFA90DB,0x65EFCC,0xE3A337,0xEF3AC1,0x69763A,
		0x578814,0xD1C4EF,0xDD5D19,0x5B11E2,0xC46EF5,0x42220E,0x4EBBF8,0xC8F703,
		0x3F964D,0xB9DAB6,0xB54340,0x330FBB,0xAC70AC,0x2A3C57,0x26A5A1,0xA0E95A,
		0x9E1774,0x185B8F,0x14C279,0x928E82,0x0DF195,0x8BBD6E,0x872498,0x016863,
		0xFAD8C4,0x7C943F,0x700DC9,0xF64132,0x693E25,0xEF72DE,0xE3EB28,0x65A7D3,
		0x5B59FD,0xDD1506,0xD18CF0,0x57C00B,0xC8BF1C,0x4EF3E7,0x426A11,0xC426EA,
		0x2AE476,0xACA88D,0xA0317B,0x267D80,0xB90297,0x3F4E6C,0x33D79A,0xB59B61,
		0x8B654F,0x0D29B4,0x01B042,0x87FCB9,0x1883AE,0x9ECF55,0x9256A3,0x141A58,
		0xEFAAFF,0x69E604,0x657FF2,0xE33309,0x7C4C1E,0xFA00E5,0xF69913,0x70D5E8,
		0x4E2BC6,0xC8673D,0xC4FECB,0x42B230,0xDDCD27,0x5B81DC,0x57182A,0xD154D1,
		0x26359F,0xA07964,0xACE092,0x2AAC69,0xB5D37E,0x339F85,0x3F0673,0xB94A88,
		0x87B4A6,0x01F85D,0x0D61AB,0x8B2D50,0x145247,0x921EBC,0x9E874A,0x18CBB1,
		0xE37B16,0x6537ED,0x69AE1B,0xEFE2E0,0x709DF7,0xF6D10C,0xFA48FA,0x7C0401,
		0x42FA2F,0xC4B6D4,0xC82F22,0x4E63D9,0xD11CCE,0x575035,0x5BC9C3,0xDD8538
	};

	unsigned int crc = 0;
	for (uint32_t i = 0; i != len; i++)
	{
		crc = ((crc << 8) & 0xFFFFFF) ^ TABLE_CRC24Q[(crc >> 16) ^ buffer[i]];
	}
	return crc;
}

uint16_t is_comm_fletcher16(uint16_t cksum_init, const void* data, uint32_t size)
{
	checksum16_u cksum = { cksum_init };
	for (int i=0; i<size; i++)
	{
		cksum.a += ((uint8_t*)data)[i];
		cksum.b += cksum.a;
	}	
	return cksum.ck;
}

uint16_t is_comm_xor16(uint16_t cksum_init, const void* data, uint32_t size)
{	
	checksum16_u cksum = { cksum_init };
	for (int i=0; i<size; i++)
	{
		cksum.a ^= ((uint8_t*)data)[i];
		cksum.b ^= cksum.a;
	}
	return cksum.ck;
}

/**
* Retrieve the 32 bit unsigned integer value of the specified bits - note that no bounds checking is done on buffer
* @param buffer the buffer containing the bits
* @param pos the start bit position in buffer to read at
* @param len the number of bits to read
* @return the 32 bit unsigned integer value
*/
unsigned int getBitsAsUInt32(const unsigned char* buffer, unsigned int pos, unsigned int len)
{
	unsigned int bits = 0;
	for (unsigned int i = pos; i < pos + len; i++)
	{
		bits = (bits << 1) + ((buffer[i / 8] >> (7 - i % 8)) & 1u);
	}
	return bits;
}

int validateBaudRate(unsigned int baudRate)
{
	// Valid baudrates for InertialSense hardware
	for (size_t i = 0; i < _ARRAY_ELEMENT_COUNT(g_validBaudRates); i++)
	{
		if (g_validBaudRates[i] == baudRate)
		{
			return 0;
		}
	}
	return -1;
}

static int dataIdShouldSwap(uint32_t dataId)
{
	switch (dataId)
	{
	case DID_GPS1_VERSION:
	case DID_GPS2_VERSION:
		return 0;
	}
	return 1;
}

static void swapPacket(packet_t* pkt)
{
	uint8_t ptype = pkt->hdr.flags & PKT_TYPE_MASK;
	if (pkt->payload.size < sizeof(p_data_hdr_t) || (ptype != PKT_TYPE_DATA && ptype != PKT_TYPE_SET_DATA))
	{	// swap entire packet, not a data packet
		flipEndianess32(pkt->payload.ptr, pkt->payload.size);
	}
	else
	{	// swap header
		flipEndianess32(pkt->payload.ptr, sizeof(p_data_hdr_t));

		// get header
		p_data_hdr_t* dataHdr = (p_data_hdr_t*)pkt->payload.ptr;

		// if dev_info_t, swap only the uint32 fields, this data structure is handled special as it contains char[] arrays and uint32_t in the same struct
		if (dataHdr->id == DID_DEV_INFO && pkt->payload.size == sizeof(p_data_hdr_t) + sizeof(dev_info_t))
		{
			// swap only the pieces that need swapping
			dev_info_t* devInfo = (dev_info_t*)(pkt->payload.ptr + sizeof(p_data_hdr_t));
			devInfo->buildNumber = SWAP32(devInfo->buildNumber);
			devInfo->repoRevision = SWAP32(devInfo->repoRevision);
			devInfo->serialNumber = SWAP32(devInfo->serialNumber);
		}
		else if (dataIdShouldSwap(dataHdr->id))
		{
			// swap entire packet body
			flipEndianess32(pkt->payload.ptr + sizeof(p_data_hdr_t), pkt->payload.size - sizeof(p_data_hdr_t));

			// flip doubles
			uint16_t* offsets;
			uint16_t offsetsLength;
			uint8_t* dataBuf = pkt->payload.ptr + sizeof(p_data_hdr_t);

			// flip doubles back if needed
			if ((offsets = getDoubleOffsets(dataHdr->id, &offsetsLength)))
			{
				flipDoubles(dataBuf, dataHdr->size, dataHdr->offset, offsets, offsetsLength);
			}

			// flip strings back if needed
			if ((offsets = getStringOffsetsLengths(dataHdr->id, &offsetsLength)))
			{
				flipStrings(dataBuf, dataHdr->size, dataHdr->offset, offsets, offsetsLength);
			}
		}
	}
}

void is_comm_init(is_comm_instance_t* instance, uint8_t *buffer, int bufferSize)
{
	// Clear buffer and initialize buffer pointers
	memset(buffer, 0, bufferSize);
	instance->buf.size = bufferSize;
	instance->buf.start = buffer;
	instance->buf.end = buffer + bufferSize;
	instance->buf.head = instance->buf.tail = instance->buf.scan = buffer;
	
	// Set parse enable flags
	instance->config.enabledMask = 
		ENABLE_PROTOCOL_ISB |
		ENABLE_PROTOCOL_NMEA |
		ENABLE_PROTOCOL_UBLOX |
		ENABLE_PROTOCOL_RTCM3 |
		ENABLE_PROTOCOL_SPARTN |
		ENABLE_PROTOCOL_SONY;
	
	instance->txPktCount = 0;
	instance->rxErrorCount = 0;
	instance->hasStartByte = 0;
	memset(&instance->dataHdr, 0, sizeof(p_data_hdr_t));
	instance->dataPtr = instance->buf.start;
    instance->ackNeeded = 0;
	memset(&instance->pkt, 0, sizeof(packet_t));
	instance->pkt.payload.ptr = instance->buf.start;
	instance->pkt.payload.size = 0;
	instance->altDecodeBuf = NULL;
}

static inline void reset_parser(is_comm_instance_t *instance)
{
	instance->hasStartByte = 0;
	instance->buf.head = instance->buf.scan;
}

static protocol_type_t processIsbPkt(is_comm_instance_t* instance, int numBytes)
{
	// Packet to decode into
	// uint8_t* head;

	if (numBytes < MIN_PACKET_SIZE)
	{	// Wait for packet header
		return _PTYPE_NONE;
	}

	packet_buf_t *pkt = (packet_buf_t*)(instance->buf.head);	
	int payload_size = pkt->hdr.payloadSize;
	int pkt_size = sizeof(packet_hdr_t) + payload_size + 2;		// Header + payload + footer (checksum)
	if (numBytes < pkt_size)
	{	// Wait for entire packet
		return _PTYPE_NONE;
	}

	uint8_t *payload = ((uint8_t*)pkt)+sizeof(packet_hdr_t);
	checksum16_u *psum = (checksum16_u*)(payload + payload_size);
	int bytes_cksum = pkt_size - 2;	
	if (psum->ck != is_comm_isb_checksum16(0, pkt, bytes_cksum))
	{	// Invalid checksum
		return _PTYPE_PARSE_ERROR;
	}

	// Valid packet
	instance->pktPtr = instance->buf.head;
	reset_parser(instance);

	instance->ackNeeded = 0;
	if (pkt->hdr.flags & ISB_FLAGS_PAYLOAD_W_OFFSET)
	{	// Offset is first two bytes in payload  
		instance->dataHdr.size = _MAX(payload_size-2, 0);
		instance->dataHdr.offset = *((uint16_t*)payload);
		// Data starts after offset if data size is non-zero
		instance->dataPtr = (instance->dataHdr.size ? payload+2 : NULL);
	}
	else
	{	// No offset
		instance->dataHdr.size = payload_size;
		instance->dataHdr.offset = 0;
		instance->dataPtr = (payload_size ? payload : NULL);
	}

	uint8_t ptype = pkt->hdr.flags & PKT_TYPE_MASK;
	switch (ptype)
	{
	case PKT_TYPE_SET_DATA:
	case PKT_TYPE_DATA:
		instance->dataHdr.id = pkt->hdr.did;

		// ensure offset and size are in bounds - check the size independent of offset because the size could be a
		//  negative number in case of corrupt data
		if (instance->dataHdr.id > DID_NULL &&
// 				instance->dataHdr.id < DID_COUNT &&		// Commented out to allow support for Luna EVB data sets
			instance->dataHdr.size <= MAX_DATASET_SIZE //&&
// 					instance->dataHdr.offset <= MAX_DATASET_SIZE &&
// 					instance->dataHdr.offset + instance->dataHdr.size <= MAX_DATASET_SIZE
			)
		{
			if(ptype==PKT_TYPE_SET_DATA)
			{	// acknowledge valid data received
				instance->ackNeeded = PKT_TYPE_ACK;
			}
				
			return _PTYPE_INERTIAL_SENSE_DATA;
		}
		else
		{	// negative acknowledge data received
			instance->ackNeeded = PKT_TYPE_NACK;
		}
		break;
			
	case PKT_TYPE_GET_DATA:
		if (instance->dataHdr.id > DID_NULL &&
			instance->dataHdr.size <= MAX_DATASET_SIZE )
		{
			// Update data pointer
			// instance->dataPtr = pkt->body.ptr + sizeof(p_data_hdr_t);
			return _PTYPE_INERTIAL_SENSE_CMD;
		}
		break;

	case PKT_TYPE_STOP_BROADCASTS_ALL_PORTS:
	case PKT_TYPE_STOP_DID_BROADCAST:
	case PKT_TYPE_STOP_BROADCASTS_CURRENT_PORT:
		return _PTYPE_INERTIAL_SENSE_CMD;
	case PKT_TYPE_ACK:
	case PKT_TYPE_NACK:
		return _PTYPE_INERTIAL_SENSE_ACK;
	}                    

	// Invalid data or checksum failure.
	instance->rxErrorCount++;
	return _PTYPE_PARSE_ERROR;
}

static protocol_type_t processAsciiPkt(is_comm_instance_t* instance)
{
	uint8_t* head = instance->buf.head;
	reset_parser(instance);

	// calculate checksum, if pass return special data id
	if (instance->buf.scan - head > 7)
	{
		// parse out checksum, put in temp null terminator
		uint8_t tmp = *(instance->buf.scan - 2);
		*(instance->buf.scan - 2) = 0;
		int actualCheckSum = (int)strtol((const char*)instance->buf.scan - 4, 0, 16);
		*(instance->buf.scan - 2) = tmp;
		int dataCheckSum = 0;
		for (uint8_t* ptr = head + 1, *ptrEnd = instance->buf.scan - 5; ptr < ptrEnd; ptr++)
		{
			dataCheckSum ^= (int)*ptr;
		}
		if (actualCheckSum == dataCheckSum)
		{	// valid NMEA Data
			// Update data pointer and info
			instance->dataPtr = instance->pktPtr = head;
			instance->dataHdr.id = 0;
			instance->dataHdr.size = (uint32_t)(instance->buf.scan - head);
			instance->dataHdr.offset = 0;
			return _PTYPE_NMEA;
		}
	}

	// Invalid data or checksum failure.
	instance->rxErrorCount++;
	return _PTYPE_PARSE_ERROR;
}

static protocol_type_t processUbloxByte(is_comm_instance_t* instance)
{
	switch (instance->parseState)
	{
	case 0: // Preamble 1 & 2
		instance->parseState = 2;
		break;
	case 2: // class id
		// fall through
	case 3: // message id
		// fall through
	case 4: // length byte 1
		// fall through
		instance->parseState++;
		break;

	case 5: // length byte 2
		{
			uint32_t len = BE_SWAP16(*((uint16_t*)(void*)(instance->buf.scan - 2)));
	
			// if length is greater than available buffer, we cannot parse this ublox packet - ublox header is 6 bytes
			if (len > instance->buf.size - 6)
			{
				instance->rxErrorCount++;
				reset_parser(instance);
				return _PTYPE_PARSE_ERROR;
			}
			
			instance->parseState = -((int32_t)len + 2);
		} 
		break;

	default:
		if (++instance->parseState == 0)
		{
			// end of ublox packet, if checksum passes, send the external id
			instance->hasStartByte = 0;
			uint16_t actualChecksum = *((uint16_t*)(instance->buf.scan - 2));
			uint8_t* cksum_start = instance->buf.head + 2;
			uint8_t* cksum_end   = instance->buf.scan - 2;
			uint32_t cksum_size  = cksum_end - cksum_start; 
			checksum16_u cksum;
			cksum.ck = is_comm_fletcher16(0, cksum_start, cksum_size);
			if (actualChecksum == cksum.ck)
			{	// Checksum passed - Valid ublox packet
				// Update data pointer and info
				instance->dataPtr = instance->buf.head;
				instance->dataHdr.id = 0;
				instance->dataHdr.size = (uint32_t)(instance->buf.scan - instance->buf.head);
				instance->dataHdr.offset = 0;
				instance->pktPtr = instance->buf.head;
				reset_parser(instance);
				return _PTYPE_UBLOX;
			}
			else
			{	// Checksum failure
				instance->rxErrorCount++;
				reset_parser(instance);
				return _PTYPE_PARSE_ERROR;
			}
		}
	}

	return _PTYPE_NONE;
}

static protocol_type_t processRtcm3Byte(is_comm_instance_t* instance)
{
	switch (instance->parseState)
	{
	case 0:
	case 1:
		instance->parseState++;
		break;

	case 2:
	{
        uint32_t msgLength = getBitsAsUInt32(instance->buf.head, 14, 10);

		// if message is too small or too big for rtcm3 or too big for buffer, fail
		if (msgLength > 1023 || msgLength > instance->buf.size - 6)
		{
			// corrupt data
			instance->rxErrorCount++;
			reset_parser(instance);
			return _PTYPE_PARSE_ERROR;
		}
		
		// parse the message plus 3 crc24 bytes
        instance->parseState = -((int32_t)msgLength + 3);

	} break;

	default:
		if (++instance->parseState == 0)
		{
			// get len without 3 crc bytes
            int lenWithoutCrc = (int)((instance->buf.scan - instance->buf.head) - 3);
			uint32_t actualCRC = calculate24BitCRCQ(instance->buf.head, lenWithoutCrc);
			uint32_t correctCRC = getBitsAsUInt32(instance->buf.head + lenWithoutCrc, 0, 24);

			if (actualCRC == correctCRC)
			{	// Checksum passed - Valid RTCM3 packet
				// Update data pointer and info
				instance->dataPtr = instance->buf.head;
				instance->dataHdr.id = 0;
				instance->dataHdr.size = (uint32_t)(instance->buf.scan - instance->buf.head);
				instance->dataHdr.offset = 0;
				instance->pktPtr = instance->buf.head;
				reset_parser(instance);
				return _PTYPE_RTCM3;
			}
			else
			{	// Checksum failure
				instance->rxErrorCount++;
				reset_parser(instance);
				return _PTYPE_PARSE_ERROR;
			}
		}
	}

	return _PTYPE_NONE;
}

static const uint8_t u8CRC_4_TABLE[] = {
    0x00U, 0x0BU, 0x05U, 0x0EU, 0x0AU, 0x01U, 0x0FU, 0x04U,
    0x07U, 0x0CU, 0x02U, 0x09U, 0x0DU, 0x06U, 0x08U, 0x03U,
    0x0EU, 0x05U, 0x0BU, 0x00U, 0x04U, 0x0FU, 0x01U, 0x0AU,
    0x09U, 0x02U, 0x0CU, 0x07U, 0x03U, 0x08U, 0x06U, 0x0DU,
    0x0FU, 0x04U, 0x0AU, 0x01U, 0x05U, 0x0EU, 0x00U, 0x0BU,
    0x08U, 0x03U, 0x0DU, 0x06U, 0x02U, 0x09U, 0x07U, 0x0CU,
    0x01U, 0x0AU, 0x04U, 0x0FU, 0x0BU, 0x00U, 0x0EU, 0x05U,
    0x06U, 0x0DU, 0x03U, 0x08U, 0x0CU, 0x07U, 0x09U, 0x02U,
    0x0DU, 0x06U, 0x08U, 0x03U, 0x07U, 0x0CU, 0x02U, 0x09U,
    0x0AU, 0x01U, 0x0FU, 0x04U, 0x00U, 0x0BU, 0x05U, 0x0EU,
    0x03U, 0x08U, 0x06U, 0x0DU, 0x09U, 0x02U, 0x0CU, 0x07U,
    0x04U, 0x0FU, 0x01U, 0x0AU, 0x0EU, 0x05U, 0x0BU, 0x00U,
    0x02U, 0x09U, 0x07U, 0x0CU, 0x08U, 0x03U, 0x0DU, 0x06U,
    0x05U, 0x0EU, 0x00U, 0x0BU, 0x0FU, 0x04U, 0x0AU, 0x01U,
    0x0CU, 0x07U, 0x09U, 0x02U, 0x06U, 0x0DU, 0x03U, 0x08U,
    0x0BU, 0x00U, 0x0EU, 0x05U, 0x01U, 0x0AU, 0x04U, 0x0FU,
    0x09U, 0x02U, 0x0CU, 0x07U, 0x03U, 0x08U, 0x06U, 0x0DU,
    0x0EU, 0x05U, 0x0BU, 0x00U, 0x04U, 0x0FU, 0x01U, 0x0AU,
    0x07U, 0x0CU, 0x02U, 0x09U, 0x0DU, 0x06U, 0x08U, 0x03U,
    0x00U, 0x0BU, 0x05U, 0x0EU, 0x0AU, 0x01U, 0x0FU, 0x04U,
    0x06U, 0x0DU, 0x03U, 0x08U, 0x0CU, 0x07U, 0x09U, 0x02U,
    0x01U, 0x0AU, 0x04U, 0x0FU, 0x0BU, 0x00U, 0x0EU, 0x05U,
    0x08U, 0x03U, 0x0DU, 0x06U, 0x02U, 0x09U, 0x07U, 0x0CU,
    0x0FU, 0x04U, 0x0AU, 0x01U, 0x05U, 0x0EU, 0x00U, 0x0BU,
    0x04U, 0x0FU, 0x01U, 0x0AU, 0x0EU, 0x05U, 0x0BU, 0x00U,
    0x03U, 0x08U, 0x06U, 0x0DU, 0x09U, 0x02U, 0x0CU, 0x07U,
    0x0AU, 0x01U, 0x0FU, 0x04U, 0x00U, 0x0BU, 0x05U, 0x0EU,
    0x0DU, 0x06U, 0x08U, 0x03U, 0x07U, 0x0CU, 0x02U, 0x09U,
    0x0BU, 0x00U, 0x0EU, 0x05U, 0x01U, 0x0AU, 0x04U, 0x0FU,
    0x0CU, 0x07U, 0x09U, 0x02U, 0x06U, 0x0DU, 0x03U, 0x08U,
    0x05U, 0x0EU, 0x00U, 0x0BU, 0x0FU, 0x04U, 0x0AU, 0x01U,
    0x02U, 0x09U, 0x07U, 0x0CU, 0x08U, 0x03U, 0x0DU, 0x06U
};

static uint8_t computeCrc4Ccitt(const uint8_t *buf, const uint32_t numBytes)
{
    // Initialize local variables
    uint8_t tableRemainder;
    uint8_t remainder = 0U; // Initial remainder

    // Compute the CRC value
    // Divide each byte of the message by the corresponding polynomial
    for (uint32_t ctr = 0U; ctr < numBytes; ctr++)
    {
        tableRemainder = buf[ctr] ^ remainder;
        remainder = u8CRC_4_TABLE[tableRemainder];
    }

    return remainder & 0x0FU;
}

static protocol_type_t processSonyByte(is_comm_instance_t* instance)
{
	switch (instance->parseState)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		instance->parseState++;
		break;

	case 4:
	{
        uint16_t msgLength = instance->buf.head[1] | (instance->buf.head[2] << 8);

    	uint8_t checksum = 0x00;
		for (size_t i = 0; i < 4; i++)
		{
			checksum += instance->buf.head[i];
		}

		if(msgLength > 4090 || msgLength > instance->buf.size || checksum != instance->buf.head[4])
		{
			// corrupt data
			instance->rxErrorCount++;
			reset_parser(instance);
			return _PTYPE_PARSE_ERROR;
		}

		// parse the message plus 1 check byte
        instance->parseState = -((int32_t)msgLength + 1);
	} break;

	default:
		if (++instance->parseState == 0)
		{
			uint16_t msgLength = instance->buf.head[1] | (instance->buf.head[2] << 8);

			uint8_t checksum = 0x00;
			for (size_t i = 0; i < msgLength; i++)
			{
				checksum += instance->buf.head[i + 5];
			}

			if(checksum != instance->buf.scan[-1])
			{
				// corrupt data
				instance->rxErrorCount++;
				reset_parser(instance);
				return _PTYPE_PARSE_ERROR;
			}
			else
			{	// Checksum passed - Valid packet
				// Update data pointer and info
				instance->dataPtr = instance->buf.head;
				instance->dataHdr.id = 0;
				instance->dataHdr.size = (uint32_t)(instance->buf.scan - instance->buf.head);
				instance->dataHdr.offset = 0;
				instance->pktPtr = instance->buf.head;
				reset_parser(instance);
				return _PTYPE_SONY;
			}
		}
	}

	return _PTYPE_NONE;
}

static protocol_type_t processSpartnByte(is_comm_instance_t* instance)
{
	switch (instance->parseState)
	{
	case 0:
	case 1:
	case 2:
	// case 3 is below this to catch bad CRCs before any more is parsed. Can be adapted to filter messages later.
	case 4:
	case 5:
	case 6:
		instance->parseState++;
		break;

	case 3: {
		// Check length and header CRC4
		const uint8_t dbuf[3] = { instance->buf.head[1], instance->buf.head[2], instance->buf.head[3] & 0xF0 };
        uint8_t calc = computeCrc4Ccitt(dbuf, 3);
        if((instance->buf.head[3] & 0x0F) != calc)
        {
        	// corrupt data
			instance->rxErrorCount++;
			reset_parser(instance);
			return _PTYPE_PARSE_ERROR;
        }

        instance->parseState++;
	} break;

	case 7:			// byte 7 (8th byte) is minimum header, but depending on what bits are set...
	case 8:
	case 9:
	case 10:
	case 11: {		// we may need to parse up to byte 11 (12th byte) to get the timestamp and encryption length
		uint16_t payloadLen = ((((uint16_t)(instance->buf.head[1]) & 0x01) << 9) |
						(((uint16_t)(instance->buf.head[2])) << 1) |
						((instance->buf.head[3] & 0x80) >> 7)) & 0x3FF;

		// Variable length CRC {0x0, 0x1, 0x2, 0x3} = {1, 2, 3, 4}bytes - appears at end of message
		payloadLen += (((instance->buf.head[3] >> 4) & 0x03) + 1);

		uint8_t extendedTs = instance->buf.head[4] & 0x08;
		uint8_t encrypt = instance->buf.head[3] & 0x40;
		uint8_t *encryptPtr = NULL;

		if(extendedTs)
		{
			// Timestamp is 32 bit

			if(!encrypt && instance->parseState == 9)
			{
				// Encryption is disabled, we are ready to go to payload bytes
				instance->parseState = -((int32_t)payloadLen);
				break;
			}
			else if(encrypt && instance->parseState == 11)
			{
				// Encryption is ENABLED, and we have all the bytes we need to compute the length of payload
				encryptPtr = &instance->buf.head[10];
				// Don't break yet; continue to calculate encryption
			}
			else
			{
				// Not ready yet
				instance->parseState++;
				break;
			}
		}
		else
		{
			// Timestamp is 16 bit

			if(!encrypt && instance->parseState == 7)
			{
				// Encryption is disabled, we are ready to go to payload bytes
				instance->parseState = -((int32_t)payloadLen);
				break;
			}
			else if(encrypt && instance->parseState == 9)
			{
				// Encryption is ENABLED, and we have all the bytes we need to compute the length of payload
				encryptPtr = &instance->buf.head[8];
				// Don't break yet; continue to calculate encryption
			}
			else
			{
				// Not ready yet
				instance->parseState++;
				break;
			}
		}

		// Add encryption authentication bytes
		if(encryptPtr)
		{
			// If the message contains an embedded authentication sequence, add the length
			if(((encryptPtr[1] >> 3) & 0x07) > 1)
			{
				switch(encryptPtr[1] & 0x07)
				{
				case 0:
					payloadLen += 8;
					break;
				case 1:
					payloadLen += 12;
					break;
				case 2:
					payloadLen += 16;
					break;
				case 3:
					payloadLen += 32;
					break;
				case 4:
					payloadLen += 64;
					break;
				default:
					break;
				}
			}
		}
		else
		{
			// corrupt data
			instance->rxErrorCount++;
			reset_parser(instance);
			return _PTYPE_PARSE_ERROR;
		}

		instance->parseState = -((int32_t)payloadLen);

	} break;


	default:
		instance->parseState++;

		if (instance->parseState == 0)
		{
			instance->dataPtr = instance->buf.head;
			instance->dataHdr.id = 0;
			instance->dataHdr.size = (uint32_t)(instance->buf.scan - instance->buf.head);
			instance->dataHdr.offset = 0;
			instance->pktPtr = instance->buf.head;
			reset_parser(instance);

			return _PTYPE_SPARTN;
		}
		else if(instance->parseState > 0)
		{
			// corrupt data or bad state
			instance->rxErrorCount++;
			reset_parser(instance);
			return _PTYPE_PARSE_ERROR;
		}

		break;
	}

	return _PTYPE_NONE;
}

int is_comm_free(is_comm_instance_t* instance)
{
// 	if (instance == 0 || instance->buf.start == 0)
// 	{
// 		return -1;
// 	}

	is_comm_buffer_t *buf = &(instance->buf);

	int bytesFree = (int)(buf->end - buf->tail);

	// if we are out of free space, we need to either move bytes over or start over
	if (bytesFree == 0)
	{
		if ((int)(buf->head - buf->start) < (int)(buf->size / 3))	// if ring buffer start index is less than this and no space is left, clear the entire ring buffer
		{	// we will be hung unless we flush the ring buffer, we have to drop bytes in this case and the caller
			//  will need to resend the data
			buf->head = buf->start;
			buf->tail = buf->start;
			buf->scan = buf->start;
		}
		else
		{	// shift over the remaining data in the hopes that we will get a valid packet by appending the next read call
			memmove(buf->start, buf->head, buf->tail - buf->head);
			int shift = (int)(buf->head - buf->start);
			buf->head -= shift;
			buf->tail -= shift;
			buf->scan -= shift;
		}

		// re-calculate free byte count
		bytesFree = (int)(buf->end - buf->tail);
	}

	return bytesFree;
}

protocol_type_t is_comm_parse_byte(is_comm_instance_t* instance, uint8_t byte)
{
	// Reset buffer if needed
	is_comm_free(instance);
	
	// Add byte to buffer
	*(instance->buf.tail) = byte;
	instance->buf.tail++;
	
	return is_comm_parse(instance);
}

#define FOUND_START_BYTE(init)		if(init){ instance->hasStartByte = byte; instance->buf.head = instance->buf.scan-1; }
#define START_BYTE_SEARCH_ERROR()	

protocol_type_t is_comm_parse(is_comm_instance_t* instance)
{
	is_comm_buffer_t *buf = &(instance->buf);
	protocol_type_t ptype;

	// Search for packet
	while (buf->scan < buf->tail)
	{
		uint8_t byte = *(buf->scan++);

		// Check for start byte if we haven't found it yet
		if (instance->hasStartByte == 0)
		{
			switch(byte)
			{
#define START_BYTE1_CHECK(mask)         { \
	if (instance->config.enabledMask & mask) { \
		instance->hasStartByte = byte; \
		instance->buf.head = instance->buf.scan-1; \
		instance->parseState = 0; \
	} \
}
#define START_BYTE2_CHECK(mask, byte1)  { \
	if (instance->parseState==byte1 && instance->config.enabledMask & mask) { \
		instance->hasStartByte = byte1; \
		instance->buf.head = instance->buf.scan-2; \
		instance->parseState = 0; \
	} \
}
			case PSC_ISB_PREAMBLE_BYTE1: // Save start byte 
			case UBLOX_START_BYTE1:      instance->parseState = byte; 	continue; 
			case PSC_ISB_PREAMBLE_BYTE2: START_BYTE2_CHECK( ENABLE_PROTOCOL_ISB, PSC_ISB_PREAMBLE_BYTE1);	break;
			case UBLOX_START_BYTE2:      START_BYTE2_CHECK( ENABLE_PROTOCOL_UBLOX, UBLOX_START_BYTE1);      break;
			case PSC_NMEA_START_BYTE:    START_BYTE1_CHECK( ENABLE_PROTOCOL_NMEA );                         break;
			case RTCM3_START_BYTE:       START_BYTE1_CHECK( ENABLE_PROTOCOL_RTCM3 );                        break;
			case SPARTN_START_BYTE:      START_BYTE1_CHECK( ENABLE_PROTOCOL_SPARTN );                       break;
			case SONY_START_BYTE:        START_BYTE1_CHECK( ENABLE_PROTOCOL_SONY );                         break;
			}

			if (instance->hasStartByte == 0)
			{	// Searching for start byte
				if (instance->parseState != -1)
				{
					instance->parseState = -1;
					instance->rxErrorCount++;
					return _PTYPE_PARSE_ERROR;	// Return to notify of error
				}
				continue;						// Continue to scan for data
			}
		}

		int numBytes = (int)(buf->scan - buf->head);
		if (numBytes > PKT_BUF_SIZE)
		{	// Packet too large!  Clear state
			instance->hasStartByte = 0;
			instance->parseState = -1;
			instance->rxErrorCount++;
			return _PTYPE_PARSE_ERROR;	// Return to notify of error
		}

		// If we have a start byte, process the data type
		switch (instance->hasStartByte)
		{
		case PSC_ISB_PREAMBLE_BYTE1:
			ptype = processIsbPkt(instance, numBytes);
			if (ptype != _PTYPE_NONE)
			{
				return ptype;
			}
			break;
		case PSC_NMEA_START_BYTE:
			if (byte == PSC_ASCII_END_BYTE)
			{
				return processAsciiPkt(instance);
			}
			// Check for invalid bytes in NMEA string and exit if found.
			if (byte == PSC_ISB_PREAMBLE || byte == 0)
			{
				instance->hasStartByte = 0;
				instance->parseState = -1;
				instance->rxErrorCount++;
				return _PTYPE_PARSE_ERROR;	// Return to notify of error
			}
			break;
		case UBLOX_START_BYTE1:
			ptype = processUbloxByte(instance);
			if (ptype != _PTYPE_NONE)
			{
				return ptype;
			}
			break;
		case RTCM3_START_BYTE:
			ptype = processRtcm3Byte(instance);
			if (ptype != _PTYPE_NONE)
			{
				return ptype;
			}
			break;
		case SPARTN_START_BYTE:
			ptype = processSpartnByte(instance);
			if(ptype == _PTYPE_PARSE_ERROR)
			{
				//time_delay_usec(500);	// Temporary test code
			}
			else if (ptype != _PTYPE_NONE)
			{
				return ptype;
			}
			break;
		case SONY_START_BYTE:
			ptype = processSonyByte(instance);
			if (ptype != _PTYPE_NONE)
			{
				return ptype;
			}
			break;
		default:
			break;
		}
	}

	// No valid data yet...
	return _PTYPE_NONE;
}

int is_comm_get_data(is_comm_instance_t* instance, uint32_t dataId, uint32_t offset, uint32_t size, uint32_t periodMultiple)
{
	p_data_get_t request;

	request.id = dataId;
	request.offset = offset;
	request.size = size;
	request.bc_period_multiple = periodMultiple;

	packet_hdr_t hdr;
	hdr.flags = PKT_TYPE_GET_DATA;
	(uint8_t)instance->txPktCount++;

	return is_comm_encode_isb_packet(&request, sizeof(request), &hdr, 0, instance->buf.start, instance->buf.size);
}

int is_comm_get_data_rmc(is_comm_instance_t* instance, uint64_t rmcBits)
{
	return is_comm_set_data(instance, DID_RMC, offsetof(rmc_t,bits), sizeof(uint64_t), (void*)&rmcBits);
}

void is_comm_encode_isb_packet_ptr(is_comm_instance_t* instance, uint8_t flags, uint16_t did, uint16_t data_size, uint16_t offset, void* data)
{
	packet_t *pkt = &(instance->pkt);

	// Header
	pkt->hdr.preamble = PSC_ISB_PREAMBLE;
	pkt->hdr.flags = flags;
	pkt->hdr.did = did;
	pkt->hdr.payloadSize = data_size;

	// Payload
	if (offset)
	{	// Offset in payload
		pkt->hdr.flags |= ISB_FLAGS_PAYLOAD_W_OFFSET;
		pkt->hdr.payloadSize += 2;
		pkt->offset = offset;
	}
	pkt->payload.ptr = data;
	pkt->payload.size = data_size;
	pkt->pktSize = pkt->hdr.payloadSize + sizeof(packet_hdr_t) + 2;	// Pkt header + payload + checksum

	// Footer
	pkt->checksum = is_comm_isb_checksum16(0, &pkt->hdr, sizeof(pkt->hdr));
	if (offset)
	{
		pkt->checksum = is_comm_isb_checksum16(pkt->checksum, &pkt->offset, sizeof(pkt->offset));
	}
	pkt->checksum = is_comm_isb_checksum16(pkt->checksum, data, data_size);

	instance->txPktCount++;
}

int is_comm_encode_isb_packet_buf(uint8_t* buf, int buf_size, uint8_t flags, uint16_t did, uint16_t data_size, uint16_t offset, void* data)
{
	int pktSize = data_size + sizeof(packet_hdr_t) + (offset ? 4 : 2);		// Pkt header + payload + checksum (+2 if offset)
	if (pktSize > buf_size)
	{	// Packet size large than buffer size
		return -1;
	}

	packet_buf_t *pkt = (packet_buf_t*)buf;

	// Header
	pkt->hdr.preamble = PSC_ISB_PREAMBLE;
	pkt->hdr.flags = flags;
	pkt->hdr.did = did;
	pkt->hdr.payloadSize = data_size;

	// Payload
	uint8_t *payload = &(pkt->payload.data);
	if (offset)
	{	// Offset in payload
		pkt->hdr.flags |= ISB_FLAGS_PAYLOAD_W_OFFSET;
		pkt->hdr.payloadSize += 2;
		pkt->payload.offset = offset;
		payload += 2;
	}
	memcpy(payload, data, data_size);

	// Footer
	int cksum_offset = pktSize-2;
	uint16_t *cksum = (uint16_t*)(buf + cksum_offset);
	*cksum = is_comm_isb_checksum16(0, buf, cksum_offset);

	return pktSize;
}

int is_comm_copy_isb_packet_ptr_to_buf(is_comm_instance_t* instance, void* buf, int buf_size)
{
	if (instance->pkt.pktSize > buf_size)
	{	// Packet size large than buffer size
		return -1;
	}

	uint8_t *ptr = buf;

#define MEMCPY_INC(dst, src, size)    memcpy((dst), (src), (size)); (dst) += (size);

	// Header
	MEMCPY_INC(ptr, &(instance->pkt.hdr), sizeof(instance->pkt.hdr));

	// Offset (optional)
	if (instance->pkt.offset)
	{
		MEMCPY_INC(ptr, &(instance->pkt.offset), sizeof(instance->pkt.offset));
	}

	// Payload
	MEMCPY_INC(ptr, instance->pkt.payload.ptr, instance->pkt.payload.size);

	// Checksum
	MEMCPY_INC(ptr, &(instance->pkt.checksum), sizeof(instance->pkt.checksum));

	return instance->pkt.pktSize;
}

int is_comm_set_data(is_comm_instance_t* instance, uint16_t did, uint16_t offset, uint16_t size, void* data)
{
    return is_comm_encode_isb_packet_buf(instance->buf.start, instance->buf.size, PKT_TYPE_SET_DATA, did, size, offset, data);
}    

int is_comm_data(is_comm_instance_t* instance, uint16_t did, uint16_t offset, uint16_t size, void* data)
{
    return is_comm_encode_isb_packet_buf(instance->buf.start, instance->buf.size, PKT_TYPE_DATA, did, size, offset, data);    
}    

int is_comm_stop_broadcasts_all_ports(is_comm_instance_t* instance)
{
    packet_hdr_t hdr;
    hdr.flags = PKT_TYPE_STOP_BROADCASTS_ALL_PORTS;
    instance->txPktCount++;

    return is_comm_encode_isb_packet(0, 0, &hdr, 0, instance->buf.start, instance->buf.size);
}

int is_comm_stop_broadcasts_current_port(is_comm_instance_t* instance)
{
    packet_hdr_t hdr;
    hdr.flags = PKT_TYPE_STOP_BROADCASTS_CURRENT_PORT;
    instance->txPktCount++;

    return is_comm_encode_isb_packet(0, 0, &hdr, 0, instance->buf.start, instance->buf.size);
}

int is_comm_encode_isb_packet(void* srcBuffer, unsigned int srcBufferLength, packet_hdr_t* hdr, uint8_t additionalPktFlags, void* encodedPacket, int encodedPacketLength)
{
	// Ensure data size is small enough, assuming packet size could double after encoding.
	if (srcBufferLength > MAX_PKT_BODY_SIZE)
	{
		return -1;
	}

	// Update Packet Counter
	uint8_t* ptrSrc;
	uint8_t* ptrSrcEnd;
	uint8_t* ptrDest = (uint8_t*)encodedPacket;
	uint8_t* ptrDestEnd = ptrDest + encodedPacketLength;
	uint32_t shifter = 0;
	uint32_t checkSumValue = CHECKSUM_SEED;
	uint32_t val;

	if (ptrDest >= ptrDestEnd)
	{
		return -1;
	}

	// // Packet header -------------------------------------------------------------------------------------------
	// *ptrDest++ = PSC_ISB_PREAMBLE;

	// // PID
	// if (ptrDest >= ptrDestEnd)
	// {
	// 	return -1;
	// }
	// val = hdr->ptype;
	// checkSumValue ^= val;

	// // Counter
	// if (ptrDest >= ptrDestEnd)
	// {
	// 	return -1;
	// }
	// val = hdr->counter;
	// checkSumValue ^= (val << 8);

	// // Flags
	// if (ptrDest >= ptrDestEnd)
	// {
	// 	return -1;
	// }
	// val = hdr->flags | additionalPktFlags | CPU_IS_LITTLE_ENDIAN | CM_PKT_FLAGS_CHECKSUM_24_BIT;
	// checkSumValue ^= (val << 16);

	// // Packet body ----------------------------------------------------------------------------------------------
	// if (srcBufferLength > 0)
	// {
	// 	ptrSrc = (uint8_t*)srcBuffer;
	// 	ptrSrcEnd = ptrSrc + srcBufferLength;

	// 	// copy body bytes, doing encoding and checksum
	// 	while (ptrSrc != ptrSrcEnd && ptrDest < ptrDestEnd)
	// 	{
	// 		val = *ptrSrc++;
	// 		checkSumValue ^= (val << shifter);

	// 		// increment shifter
	// 		shifter += 8;

	// 		// reset if shifter equals 24
	// 		shifter *= (shifter != 24);
	// 	}
	// }

	// // footer ----------------------------------------------------------------------------------------------------

	// // checksum byte 3
	// if (ptrDest >= ptrDestEnd)
	// {
	// 	return -1;
	// }
	// val = (uint8_t)((checkSumValue >> 16) & 0xFF);

	// // checksum byte 2
	// if (ptrDest >= ptrDestEnd)
	// {
	// 	return -1;
	// }
	// val = (uint8_t)(checkSumValue >> 8) & 0xFF;

	// // checksum byte 1
	// if (ptrDest >= ptrDestEnd)
	// {
	// 	return -1;
	// }
	// val = (uint8_t)(checkSumValue & 0xFF);

	// // packet end byte
	// if (ptrDest >= ptrDestEnd)
	// {
	// 	return -1;
	// }
	// *ptrDest++ = PSC_END_BYTE;
    // return (int)(ptrDest - (uint8_t*)encodedPacket);
	return 0;
}

char copyStructPToDataP(p_data_t *data, const void *sptr, const unsigned int maxsize)
{
    if ((data->hdr.size + data->hdr.offset) <= maxsize)
    {
        memcpy((uint8_t*)(data->buf), (uint8_t*)sptr + data->hdr.offset, data->hdr.size);
        return 0;
    }
    else
    {
        return -1;
    }
}

char copyDataPToStructP(void *sptr, const p_data_t *data, const unsigned int maxsize)
{
    if ((data->hdr.size + data->hdr.offset) <= maxsize)
    {
        memcpy((uint8_t*)sptr + data->hdr.offset, data->buf, data->hdr.size);
        return 0;
    }
    else
    {
        return -1;
    }
}

/** Copies packet data into a data structure.  Returns 0 on success, -1 on failure. */
char copyDataPToStructP2(void *sptr, const p_data_hdr_t *dataHdr, const uint8_t *dataBuf, const unsigned int maxsize)
{
    if ((dataHdr->size + dataHdr->offset) <= maxsize)
    {
        memcpy((uint8_t*)sptr + dataHdr->offset, dataBuf, dataHdr->size);
        return 0;
    }
    else
    {
        return -1;
    }
}

/** Copies packet data into a data structure.  Returns 0 on success, -1 on failure. */
char is_comm_copy_to_struct(void *sptr, const is_comm_instance_t *instance, const unsigned int maxsize)
{    
    if ((instance->dataHdr.size + instance->dataHdr.offset) <= maxsize)
    {
        memcpy((uint8_t*)sptr + instance->dataHdr.offset, instance->dataPtr, instance->dataHdr.size);
        return 0;
    }
    else
    {
        return -1;
    }
}
