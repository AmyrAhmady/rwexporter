#include <iomanip>
#include <math.h>
#include <fstream>
#include <sstream>

#include "lodepng.h"
#include "json.hpp"
#include "renderware.h"
#include "export.hpp"

#pragma warning (disable : 4244) // I KNOW WE ARE NOT MEANT TO DO THAT BUT I HAVE TO FOR FLOAT TO DOUBLE CONVERSION
#define FLOATROUND(x) (std::ceil(static_cast<double>(x)*1000000.0)/1000000.0)

using namespace nlohmann;

void Export::DffModel(const std::string & path, const std::string & output)
{
	json jsonFile = json::array();


	std::ifstream rw(path, std::ios::binary);

	if (rw.fail())
	{
		std::cout << "\nCould not open " << path << std::endl;
	}

	// Read the clump structure
	rw::Clump * clump = new rw::Clump;

	// We have to do the reading part manually instead of using Clump::Read
	// cause some of SA-MP dff models have collisions inside under SA-MP's specific CHUNK/Section;
	// Which causes Clump::readExtension from rwtools library to freeze and being put in an infinite loop.

	rw::HeaderInfo header;

	header.read(rw);

	READ_HEADER(CHUNK_STRUCT);
	uint32_t numAtomics = rw::readUInt32(rw);
	uint32_t numLights = 0;
	if (header.length == 0xC) {
		numLights = rw::readUInt32(rw);
		rw.seekg(4, std::ios::cur); /* camera count, unused in gta */
	}
	clump->atomicList.resize(numAtomics);

	READ_HEADER(CHUNK_FRAMELIST);

	READ_HEADER(CHUNK_STRUCT);
	uint32_t numFrames = rw::readUInt32(rw);
	clump->frameList.resize(numFrames);
	for (uint32_t i = 0; i < numFrames; i++)
	{
		clump->frameList[i].readStruct(rw);
	}
		
	for (uint32_t i = 0; i < numFrames; i++)
	{
		clump->frameList[i].readExtension(rw);
	}

	READ_HEADER(CHUNK_GEOMETRYLIST);

	READ_HEADER(CHUNK_STRUCT);
	uint32_t numGeometries = rw::readUInt32(rw);
	clump->geometryList.resize(numGeometries);
	for (uint32_t i = 0; i < numGeometries; i++)
	{
		clump->geometryList[i].read(rw);
	}

	// Read atomics
	for (uint32_t i = 0; i < numAtomics; i++)
	{
		clump->atomicList[i].read(rw);
	}

	// Read lights
	clump->lightList.resize(numLights);
	for (uint32_t i = 0; i < numLights; i++) 
	{
		READ_HEADER(CHUNK_STRUCT);
		clump->lightList[i].frameIndex = rw::readInt32(rw);
		clump->lightList[i].read(rw);
	}
	
	// Looping through frame
	int frameCount = clump->frameList.size();
	auto & frames = clump->frameList;
	for (int i = 0; i < frameCount; i++)
	{
		bool hasGeometry = false;
		
		// Generate a proper 4x4 matrix
		std::vector<std::vector<double>>
			matrix =
		{
				{ frames[i].rotationMatrix[0], frames[i].rotationMatrix[1], frames[i].rotationMatrix[2], FLOATROUND(frames[i].position[0]) },
				{ frames[i].rotationMatrix[3], frames[i].rotationMatrix[4], frames[i].rotationMatrix[5], FLOATROUND(frames[i].position[1]) },
				{ frames[i].rotationMatrix[6], frames[i].rotationMatrix[7], frames[i].rotationMatrix[8], FLOATROUND(frames[i].position[2]) },
				{ 0.0f, 0.0f, 0.0f, 1.0f }
		};
		
		json jsonGeometry;
		for (auto & atomic : clump->atomicList)
		{
			if (atomic.frameIndex == i)
			{
				json material;
				auto & geometry = clump->geometryList[atomic.geometryIndex];
				hasGeometry = true;

				// texCoords (UVs)
				json texCoords = json::array();
				for (uint32_t i = 0; i < geometry.texCoords[0].size() / 2; i++)
				{
					texCoords.push_back(
						{
							{"uvx", FLOATROUND(geometry.texCoords[0][i * 2 + 0])},
							{"uvy", FLOATROUND(geometry.texCoords[0][i * 2 + 1])}
						}
					);
				}

				// Vertices
				std::vector<json> vertices;
				for (uint32_t i = 0; i < geometry.vertices.size() / 3; i++)
				{
					vertices.push_back(
						{
							{"x", FLOATROUND(geometry.vertices[i * 3 + 0])},
							{"y", FLOATROUND(geometry.vertices[i * 3 + 1])},
							{"z", FLOATROUND(geometry.vertices[i * 3 + 2])}
						}
					);
				}

				// FaceType (format)
				std::string faceType = geometry.faceType == 1 ? "Triangle_Strip" : "Triangles";

				// Textures
				std::vector<json> textures;
				for (auto & split : geometry.splits)
				{
					json texture;
					texture["indices"] = split.indices;
					auto & rwmaterial = geometry.materialList[split.matIndex];
					texture["name"] = rwmaterial.texture.name;
					texture["color"] = std::vector<uint8_t>({ rwmaterial.color[0], rwmaterial.color[1] , rwmaterial.color[2] , rwmaterial.color[3] });
					textures.push_back(texture);
				}

				// Vertex Colors
				if (!geometry.vertexColors.empty())
				{
					for (uint32_t i = 0; i < geometry.vertexColors.size() / 4; i++)
					{
						int color = (((((geometry.vertexColors[i * 4 + 0] << 8) + geometry.vertexColors[i * 4 + 1]) << 8) + geometry.vertexColors[i * 4 + 2]) << 8) + geometry.vertexColors[i * 4 + 3];
						vertices[i]["color"] = color;
					}

				}

				material["texcoords"] = texCoords;
				material["vertices"] = vertices;
				material["textures"] = textures;
				material["facetype"] = faceType;

				jsonGeometry = material;
			}
		}

		jsonFile.push_back(
			{
				{"frame", i},
				{"parent", frames[i].parent},
				{"name", frames[i].name},
				{"matrix", matrix},
				{"geometry", jsonGeometry},
				{"empty", hasGeometry ? false : true}
			}
		);
	}

	std::string dffFilename = path.substr(path.find_last_of("/\\") + 1);
	dffFilename = dffFilename.substr(0, dffFilename.find_last_of('.'));

	std::ofstream outputfile(output + "/" + dffFilename + ".json");
	outputfile << std::setw(4) << jsonFile << std::endl;
	outputfile.close();

	std::cout << "[\n"
		<< "\t\"" << dffFilename + ".dff\" has been converted to \"" << output + "/" + dffFilename + ".json\"\n"
		<< "]\n";
}

void Export::TexDic(const std::string & path, const std::string & output)
{
	std::ifstream rw(path, std::ios::binary);
	rw::TextureDictionary txd;
	txd.read(rw);
	rw.close();

	std::cout << "[\n";
	for (uint32_t i = 0; i < txd.texList.size(); i++)
	{
		rw::NativeTexture & t = txd.texList[i];

		std::cout << "\t\"" 
			<< t.name << "\", \"" << t.maskName << "\", "
			<< t.width[0] << ", " << t.height[0] << ", "
			<< t.depth << ", " << std::hex << t.rasterFormat << std::endl;

		if (txd.texList[i].dxtCompression)
		{
			txd.texList[i].decompressDxt();
		}

		txd.texList[i].convertTo32Bit();

		std::vector<uint8_t> sup;
		for (uint32_t j = 0; j < t.width[0] * t.height[0]; j++) 
		{
			sup.push_back(t.texels[0][j * 4 + 2]); // R
			sup.push_back(t.texels[0][j * 4 + 1]); // G
			sup.push_back(t.texels[0][j * 4 + 0]); // B
			sup.push_back(t.texels[0][j * 4 + 3]); // A
		}

		lodepng::encode(std::string(output + "/" + t.name + ".png"), sup, t.width[0], t.height[0], LodePNGColorType::LCT_RGBA, 8);
	}
	std::cout << "]\n";
}