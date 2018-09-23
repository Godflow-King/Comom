
#include "Utils.h"
#include "reader.h"
#include <fstream>
#include <sstream>

Json::Value CUtils::LoadJsonFromString(char* text)
{
	Json::Reader jsonReader;
	Json::Value jsonValue;

	jsonReader.parse(text, jsonValue);

	return jsonValue;
}

Json::Value CUtils::LoadJsonFromString(std::string str)
{
	Json::Reader jsonReader;
	Json::Value jsonValue;
	jsonReader.parse(str, jsonValue);
	return jsonValue;
}


Json::Value CUtils::LoadJsonFromFile(std::string filePath)
{
	Json::Reader jsonReader;
	Json::Value jsonValue;
	std::ifstream m_stream(filePath.c_str());
	if (m_stream.is_open())
	{
		jsonReader.parse(m_stream, jsonValue);
	}
	m_stream.close();
	return jsonValue;
}


bool CUtils::JsonToValue(Json::Value* jsonValue, int& value)
{
	value = 0;
	if (jsonValue == NULL || jsonValue->isNull())
	{
		return false;
	}
	if (jsonValue->isInt() || jsonValue->isUInt())
	{
		value = jsonValue->asInt();
		return true;
	}
	else
	{
		return false;
	}
}

bool CUtils::JsonToValue(Json::Value* jsonValue, unsigned char& value)
{
	int iValue = 0;
	bool result = JsonToValue(jsonValue, iValue);
	value = iValue;
	return result;
}


int CUtils::JsonArraySize(Json::Value* value)
{
	if (value == NULL || value->isNull())
		return 0;
	if (value->isArray())
		return value->size();
	else
		return 0;
}

bool CUtils::JsonToValue(Json::Value* jsonValue, std::string& value)
{
	value = "";
	if (jsonValue == NULL || jsonValue->isNull())
	{
		return false;
	}
	if (jsonValue->isString())
	{
		value = jsonValue->asString();
		return true;
	}
	else
	{
		return false;
	}

}

bool CUtils::JsonToValue(Json::Value* jsonValue, double& value)
{
	value = 0;
	if (jsonValue == NULL || jsonValue->isNull())
	{
		return false;
	}else if (jsonValue->isDouble() || jsonValue->isInt())
	{
		value = jsonValue->asDouble();
		return true;
	}
	else
	{
		return false; 
	}
}


bool CUtils::JsonToValue(Json::Value* jsonValue, bool& value)
{
    value = false;
	if (jsonValue == NULL || jsonValue->isNull())
	{
		return false;
	}
	if (jsonValue->isBool())
	{
		value = jsonValue->asBool();
		return true;
	}
	else
	{
		return false;
	}
}
/*
void CUtils::SplitString(const wstring& strInput, const wstring& strDelim, vector<wstring>& vOut)
{
	vOut.clear();
	int inputLen = strInput.length();
	wchar_t* wcInput = new wchar_t[inputLen + 1];
	wmemset(wcInput, 0, inputLen + 1);
	wcsncpy(wcInput, strInput.c_str(), strInput.length());
	wchar_t* token;
	token = wcstok(wcInput, strDelim.c_str());
	while(token != NULL)
	{
		vOut.push_back(token);
		token = wcstok(NULL, strDelim.c_str());
	}
	delete[] wcInput;
}
*/
