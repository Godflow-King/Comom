
#ifndef _TSUTILS_H_
#define _TSUTILS_H_
#include "value.h"
#include <algorithm>
#include <string>


class CUtils
{
public:

	static Json::Value LoadJsonFromString(char* text);
	static Json::Value LoadJsonFromString(std::string str);
	static Json::Value LoadJsonFromFile(std::string filePath);
	static bool JsonToValue(Json::Value* jValue, int& value);
	static bool JsonToValue(Json::Value* jsonValue, unsigned char& value);
	static bool JsonToValue(Json::Value* jsonValue, std::string& value);
	static bool JsonToValue(Json::Value* jsonValue, double& value);
	static bool JsonToValue(Json::Value* jsonValue, bool& value);
	static int JsonArraySize(Json::Value* value);
	static int replace_all(std::string& str,  const std::string& pattern,  const std::string& newpat);
	static void SaveToFile(const char* pData, unsigned long len, const std::string &filePath);
};

#endif
