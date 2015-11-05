#include "stdafx.h"
#include "JoystickCommon.h"
CString JsonDocToString(const rapidjson::Document& jsDoc)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	jsDoc.Accept(writer);
	CString str = buffer.GetString();
	return str;
}
CString JsonValueToString(const rapidjson::Value& jsValue)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	jsValue.Accept(writer);
	CString str = buffer.GetString();
	return str;
}

