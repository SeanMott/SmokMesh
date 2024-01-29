#pragma once

//defines the mesh data

#include <SmokGraphics/Utils/Uniforms.hpp>

#include <BTDSTD/String.hpp>
#include <BTDSTD/IO/File.hpp>
#include <BTDSTD/IO/YAML.hpp>

#include <yaml-cpp/yaml.h>

namespace Smok::Mesh
{
	//defines the model vertex
	struct Vertex {
		glm::vec4 color;
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 textureCords;

		//gets the vertex layout
		inline static Graphics::Util::Uniform::VertexLayout VertexLayout()
		{
			//defines a vertex layout
			Smok::Graphics::Util::Uniform::VertexLayout vertexLayout = {
			0, //the set that owns this layout
			sizeof(Vertex), //the size of the whole struct

			//the entries of data
			//the shader location, the datatype, the mem offset, the name
			{
			  {0, BTD::Reflection::Datatype::FVec3, offsetof(Vertex, position), "position"},
			  {1, BTD::Reflection::Datatype::FVec3, offsetof(Vertex, normal), "normal"},
			{2, BTD::Reflection::Datatype::FVec2, offsetof(Vertex, textureCords), "textureCords"},
			 {3, BTD::Reflection::Datatype::FVec4, offsetof(Vertex, color), "color"}
			}
			};

			return vertexLayout;
		}

		//checks if the vertex are the same
		bool operator==(const Vertex& other)
		{
			return (position.x == other.position.x && position.y == other.position.y && position.z == other.position.z &&
				color.r == other.color.r && color.g == other.color.g && color.b == other.color.b && color.a == other.color.a &&
				textureCords.x == other.textureCords.x && textureCords.y == other.textureCords.y);
		}

		//checks if the vertex are the same
		bool operator==(Vertex& other)
		{
			return (position.x == other.position.x && position.y == other.position.y && position.z == other.position.z &&
				color.r == other.color.r && color.g == other.color.g && color.b == other.color.b && color.a == other.color.a &&
				textureCords.x == other.textureCords.x && textureCords.y == other.textureCords.y);
		}
	};

	//defines a mesh
	struct Mesh
	{
		BTD::Math::FVec3 baseScale = BTD::Math::FVec3::One();

		std::vector<Vertex> vertices;
	};

	//defines a smesh file path
#define SMOK_MESH_MESH_DATA_DECL_EXTENTION ".smeshdecl"
#define SMOK_MESH_MESH_DATA_EXTENTION ".smesh"

	//writes a mesh writing taks
	inline bool Mesh_WriteMeshDataToFile(const std::string& directory, const std::string& assetName, 
		const std::vector<Mesh>& meshes)
	{
		//checks if the file exists/can be made
		BTD::IO::File file;
		std::string filePath = directory + "/" + assetName;
		file.Open(std::string(filePath + SMOK_MESH_MESH_DATA_DECL_EXTENTION), BTD::IO::FileOP::TextWrite_OpenCreateStart);
		//	return false;

		//stores the vertices for the binary data
		std::vector<Vertex> data; data.reserve(25);

		//starts the file
		YAML::Emitter emitter;
		emitter << YAML::BeginMap;
		
		emitter << YAML::Key << "name" << YAML::DoubleQuoted << assetName;

		emitter << YAML::Key << "meshCount" << YAML::Value << meshes.size();

		//goes through the meshes
		emitter << YAML::Key << "meshes" << YAML::Value << YAML::BeginSeq;

		//goes through the meshes
		for (uint32 i = 0; i < meshes.size(); ++i)
		{
			emitter << YAML::BeginMap;

			emitter << YAML::Key << "vertexStartOffset" << YAML::DoubleQuoted << data.size();

			//vertex saves
			for (uint32 v = 0; v < meshes[i].vertices.size(); ++v)
				data.emplace_back(meshes[i].vertices[v]);

			emitter << YAML::Key << "vertexEndOffset" << YAML::DoubleQuoted << data.size();

			emitter << YAML::Key << "baseMeshScale" << YAML::Key << meshes[i].baseScale;

			emitter << YAML::EndMap;
		}

		emitter << YAML::EndSeq;

		//stores vertex data
		emitter << YAML::Key << "vertexCount" << YAML::Value << data.size();

		//stores the path for the vertex data
		filePath += SMOK_MESH_MESH_DATA_EXTENTION;
		emitter << YAML::Key << "binaryDataPath" << YAML::DoubleQuoted << filePath;

		emitter << YAML::EndMap;

		//write decl file
		file.Write(emitter.c_str());
		file.Close();

		//write binary file
		file.Open(filePath, BTD::IO::FileOP::BinaryWrite_OpenCreateStart);
		file.WriteBinary(data.data(), sizeof(Vertex), data.size());

		return true;
	}

	//defines the vertex offsets in the decl file
	struct MeshDeclData_MeshOffset
	{
		size_t vertexStartOffset = 0, //the index to start at when reading mesh data
			vertexEndOffset = 0; //the index to end at when reading mesh data
	};

	//defines a mesh decl data, the data retrived from the file
	struct MeshDeclData
	{
		size_t meshCount = 0; //the total number of vertexs making up the binary blob

		std::vector<Mesh> meshes; //the meshes
		std::vector<MeshDeclData_MeshOffset> meshOffsets; //the mesh offsets for start and end reading binary data

		std::string binaryBlobPath = ""; //the binary blob path

		std::string name = ""; //the name of the mesh
	};

	//loads mesh data from a binary file and a decl file
	bool Mesh_LoadMeshDataFromFile(const std::string& declFilePath, MeshDeclData& data)
	{
		//checks if the decl file exists
		BTD::IO::File file;
		if (!file.Open(declFilePath, BTD::IO::FileOP::TextRead_OpenExisting))
		{
			BTD::Logger::LogError("Smok Mesh", "Mesh", "Mesh_LoadMeshDataToFile",
				std::string("The given decl file path can't be opened at \"" + declFilePath + "\"").c_str());
			return false;
		}

		//loads the YAML data
		YAML::Node _data = YAML::Load(file.Read());
		file.Close();

		if (!_data)
		{
			BTD::Logger::LogError("Smok Mesh", "Mesh", "Mesh_LoadMeshDataToFile",
				"The decl file is invalid!");
			return false;
		}

		//gets the file settings
		data.binaryBlobPath = _data["binaryDataPath"].as<std::string>();
		const size_t vertexCount = _data["vertexCount"].as<size_t>();
		data.meshCount = _data["meshCount"].as<size_t>();
		data.name = _data["name"].as<std::string>();

		//gets the mesh offsets
		data.meshOffsets.reserve(data.meshCount);
		data.meshes.reserve(data.meshCount);
		auto _meshData = _data["meshes"];
		if (_meshData)
		{
			//loads the binary blob into a vertices buffer
			if (!file.Open(data.binaryBlobPath, BTD::IO::FileOP::BinaryRead_OpenExisting))
			{
				BTD::Logger::LogError("Smok Mesh", "Mesh", "Mesh_LoadMeshDataToFile",
					std::string("Failed to load binary vertex data from the SMESH file at \"" +
					data.binaryBlobPath + "\"!").c_str());
				return false;
			}

			std::vector<Vertex> vertices; vertices.resize(vertexCount);
			file.ReadBinary(vertices.data(), sizeof(Vertex), vertexCount);
			file.Close();

			//gets the vertex offsets
			for (auto _mesh : _meshData)
			{
				//gets the mesh offsets
				MeshDeclData_MeshOffset* vo = &data.meshOffsets.emplace_back(MeshDeclData_MeshOffset());
				vo->vertexStartOffset = _mesh["vertexStartOffset"].as<size_t>();
				vo->vertexEndOffset = _mesh["vertexEndOffset"].as<size_t>();

				//generates the meshes
				Mesh* m = &data.meshes.emplace_back(Mesh()); m->vertices.reserve(vo->vertexEndOffset - vo->vertexStartOffset);
				for (uint32 i = vo->vertexStartOffset; i < vo->vertexEndOffset; ++i)
				{
					m->vertices.emplace_back(vertices[i]);
				}

				//m->baseScale = _mesh["baseMeshScale"].as<BTD::Math::FVec3>();
			}
		}

		return true;
	}
}