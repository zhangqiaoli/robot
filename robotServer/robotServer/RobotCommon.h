#ifdef _MSC_VER
#pragma once
#endif
#ifndef _INC_ROBOTCOMMON_H
#define _INC_ROBOTCOMMON_H
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#define PROTOCOLFLAG_ROBOTCONNECTION 0x01332CD4
#define PROTOCOLFLAG_ROBOTAGENTCONNECTION 0x01332AD4
typedef struct _tag_Robot_MsgHeader
{
	UInt32 nProtocolFlag;
	UInt32 nMessageLength;
	UInt32 nInvokeID;
}SMsgHeader, *LPSMsgHeader;

string JsonDocToString(const rapidjson::Document& jsDoc);
string JsonValueToString(const rapidjson::Value& jsValue);
#endif