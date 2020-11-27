#pragma once
#include <string>
#include "renderware.h"

class Export
{
public:
	static void DffModel(const std::string & path, const std::string & output, bool toAMF = false);
	static void WriteToAMF(const rw::Clump * clump, std::ofstream & output);
	static void WriteToJSON(const rw::Clump * clump, std::ofstream & output);
	static void TexDic(const std::string & path, const std::string & output);
};