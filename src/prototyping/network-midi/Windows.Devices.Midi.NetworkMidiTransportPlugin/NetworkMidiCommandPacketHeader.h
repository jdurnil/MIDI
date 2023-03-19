#pragma once

#include "pch.h"


#include "NetworkMidiCommandCode.h"

union NetworkMidiCommandPacketHeader
{
	struct
	{
		union
		{
			uint16_t AsUInt16;

			struct 
			{
				uint8_t Byte2; // can't use an array endianess is wrong
				uint8_t Byte1;
			} AsBytes;
		} CommandSpecificData;

		uint8_t CommandPayloadLength;
		NetworkMidiCommandCode CommandCode;
	} HeaderData;

	uint32_t HeaderWord;
};