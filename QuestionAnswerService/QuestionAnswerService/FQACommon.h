#ifdef _MSC_VER
#pragma once
#endif
#ifndef _INC_FQACOMMON_H
#define _INC_FQACOMMON_H
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
string JsonDocToString(const rapidjson::Document& jsDoc);
string JsonValueToString(const rapidjson::Value& jsValue);
#endif