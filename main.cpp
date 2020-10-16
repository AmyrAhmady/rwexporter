#include <iostream>
#include <conio.h> 
#include <dirent.h>
#include "src/export.hpp"

int main(int argc, char ** argv)
{

	std::cout << "RenderWare Exporter (C) Amyr Ahmady (iAmir) https://github.com/AmyrAhmady \n\n";

	//Export::TexDic("E:\\gtasa-models\\gta3\\bullet.txd", "E:\\rwexporter\\rwexporter\\build\\Debug");

	//return 0;

	if (argc != 4)
	{
		std::cout << "Usage: rwexporter [flag] path output_dir\n\n\n" <<
			"|- flags:\n" <<
			"\t-txd -> parse a texture dictionary and export all the textures\n" << 
			"\t        as png files into given output directory.\n\n" <<
			"\t-dff -> parse a dff model and export data into a json file.\n\n" <<
			"\t-dir -> enumerate over all files in the given directory path\n" << 
			"\t        and export them like what -txd and -dff does.\n\n\n" <<
			"|- path: path of the target file or directory.\n\n" <<
			"|- output_dir: path of the directory that exported files will save into.\n\n\n\n";

		std::cout << "Press any key to exit...";
		_getch();
		return 0;
	}

	if (!strcmp(argv[1], "-dff"))
	{
		Export::DffModel(argv[2], argv[3]);
	}
	else if (!strcmp(argv[1], "-txd"))
	{
		Export::TexDic(argv[2], argv[3]);
	}
	else if (!strcmp(argv[1], "-dir"))
	{
		DIR * inputDir;
		struct dirent * file;
		inputDir = opendir(argv[2]);
		if (inputDir != NULL)
		{
			std::cout << "Enumerating all files in " << argv[2] << std::endl;
			while (file = readdir(inputDir))
			{
				std::cout << "Processing " << file->d_name << std::endl;
				std::size_t found = std::string(file->d_name).find(".dff");
				if (found != std::string::npos)
				{
					Export::DffModel(std::string(argv[2]) + "/" + file->d_name, argv[3]);
				}

				found = std::string(file->d_name).find(".txd");
				if (found != std::string::npos)
				{
					Export::TexDic(std::string(argv[2]) + "/" + file->d_name, argv[3]);
				}
			}
			std::cout << "Finished processing all files" << std::endl;
		}
		closedir(inputDir);
	}
}
