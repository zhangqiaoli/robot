#pragma once

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

CString JsonDocToString(const rapidjson::Document& jsDoc);
CString JsonValueToString(const rapidjson::Value& jsValue);
