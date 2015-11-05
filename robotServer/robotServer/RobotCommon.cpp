#include "stdafx.h"
#include "RobotCommon.h"
string JsonDocToString(const rapidjson::Document& jsDoc)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	jsDoc.Accept(writer);
	string str = buffer.GetString();
	return str;
}
string JsonValueToString(const rapidjson::Value& jsValue)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	jsValue.Accept(writer);
	string str = buffer.GetString();
	return str;
}

