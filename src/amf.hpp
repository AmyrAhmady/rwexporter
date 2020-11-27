#pragma once
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <fstream>
#include "structs.hpp"

namespace amf
{

	struct Vector2D
	{
		float x;
		float y;
	};

	struct Vector3D
	{
		float x;
		float y;
		float z;
	};

	typedef Vector2D AMFTexCoord;
	typedef Vector3D AMFVertex;

	struct AMFTexture
	{
		std::string name;
		std::vector<uint32_t> indices;
		std::array<uint8_t, 4> color;
	};

	struct AMFGeometry
	{
		std::string faceType;
		std::vector<AMFTexture> textures;
		std::vector<AMFTexCoord> texCoords;
		std::vector<AMFVertex> vertices;
		std::vector<uint32_t> vertexColors;
	};

	struct AMFFrame
	{
		std::string name;
		uint16_t index;
		uint16_t parent;
		bool damaged;
		std::vector<std::vector<float>> matrix;
		AMFGeometry geometry;
	};

	struct AMFormat
	{
		std::vector<std::string> textureNames;
		std::vector<AMFFrame> frames;
	};

	class AMFModel
	{
	public:
		AMFModel()
		{}

		AMFModel(const AMFormat value)
			: data(value)
		{}

		~AMFModel()
		{}

		void Set(const AMFormat value)
		{
			data = value;
		}

		const AMFormat & Get() const
		{
			return data;
		}
		

	private:
		AMFormat data;

	};

	class AMFFile
	{
	public:
		AMFFile()
		{}

		AMFFile(const AMFModel & model)
			: model(model)
		{}

		~AMFFile()
		{}

		void SetModel(const AMFModel & value)
		{
			model = value;
		}

		const AMFModel & GetModel() const
		{
			return model;
		}

		void WriteToFile(std::ofstream & file)
		{
			AMFormat data = model.Get();
			
			writeVector(data.textureNames);

			write(static_cast<uint16_t>(data.frames.size()));

			for (auto & frame : data.frames)
			{
				write(frame.index);
				write(frame.parent);
				write(frame.damaged);
				write(frame.name);
				write(frame.geometry.faceType);
				writeVector(frame.geometry.textures);
				writeVector(frame.geometry.texCoords);
				writeVector(frame.geometry.vertices);
				writeVector(frame.geometry.vertexColors);
			}

			file << stream.str();
		}

	private:
		AMFModel model;
		std::stringstream stream;

		void write(const char * value)
		{
			stream << value;
		}

		void write(char * value)
		{
			stream << value;
		}

		template<typename T>
		void write(T value)
		{
			stream << (char *)&value;
		}

		template<>
		void write<AMFVertex>(AMFVertex value)
		{
			write(value.x);
			write(value.y);
			write(value.z);
		}

		template<>
		void write<AMFTexCoord>(AMFTexCoord value)
		{
			write(value.x);
			write(value.y);
		}

		template<>
		void write<AMFTexture>(AMFTexture value)
		{
			write(value.name);
			writeVector(value.indices);
			writeArray<uint8_t, value.color.size()>(value.color);
		}

		template<>
		void write<std::string>(std::string value)
		{
			write(static_cast<uint8_t>(value.size()));
			write(value.data());
		}

		template<typename T>
		void writeVector(const std::vector<T> & vector)
		{
			stream << static_cast<uint32_t>(vector.size());
			for (auto & element : vector)
			{
				write<T>(element);
			}
		}

		template<typename T, std::size_t size>
		void writeArray(const std::array<T, size> & array)
		{
			stream << static_cast<uint32_t>(size);
			for (auto & element : array)
			{
				write<T>(element);
			}
		}
	};
}

