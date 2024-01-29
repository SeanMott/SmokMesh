#pragma once

//defines a mega mesh buffer for managing multiable meshes

#include <SmokGraphics/Utils/MeshBuffer.hpp>

#include <SmokMesh/Mesh.hpp>

namespace Smok::Mesh::Util
{
	//stores the mesh offsets for the index buffer
	struct MeshOffset
	{
		uint32 indexStart = 0, //the offset into the index it should start
			indexCount = 0; //the total amount of indices to draw
	};

	//defines a mega buffer
	struct MegaMeshBuffer
	{
		bool isDirty = false; //has the buffer been modified

		SMGraphics_Util_VertexBuffer vertexBuffer; //the main vertex buffer
		SMGraphics_Util_IndexBuffer indexBuffer; //the index buffer for drawing

		std::vector<MeshOffset> meshOffsets; //the mesh offsets
		std::vector<uint32> indices; //the indices
		std::vector<Vertex> vertices; //the vertices
	};

	//adds a mesh to the mega buffer
	inline void MegaMeshBuffer_AddMesh(MegaMeshBuffer* megaMeshBuffer, const Mesh& mesh,
		uint32& meshOffsetIndex)
	{
		//gets the new starting point for the indices
		MeshOffset* meshOffset = &megaMeshBuffer->meshOffsets.emplace_back(MeshOffset());
		meshOffset->indexStart = megaMeshBuffer->indices.size();
		meshOffsetIndex = megaMeshBuffer->meshOffsets.size() - 1;

		//goes through the mesh vertices
		for (uint32 v = 0; v < mesh.vertices.size(); ++v)
		{
			//checks if the vertex is already in the array
			uint32 index = 0; bool wasFound = false;
			for (uint32 i = 0; i < megaMeshBuffer->vertices.size(); ++i)
			{
				if (megaMeshBuffer->vertices[i] == mesh.vertices[v])
				{
					index = i; wasFound = true;
					break;
				}
			}

			//if it wasn't there, add it
			if (!wasFound)
			{
				//stores the index before incrementing the count
				index = megaMeshBuffer->vertices.size();

				//stores the vertex
				megaMeshBuffer->vertices.emplace_back(mesh.vertices[v]);
			}

			//adds the index
			megaMeshBuffer->indices.emplace_back(index);

			//increments the number of indices counted in the mesh offset
			meshOffset->indexCount++;
		}

		megaMeshBuffer->isDirty = true;
	}

	//removes a mesh from the mega buffer

	//clears the mega buffer
	inline void MegaMeshBuffer_ClearMeshes(MegaMeshBuffer* megaMeshBuffer)
	{
		megaMeshBuffer->meshOffsets.clear();
		megaMeshBuffer->indices.clear();
		megaMeshBuffer->vertices.clear();
		megaMeshBuffer->isDirty = true;
	}

	//destroys the mega buffer
	inline void MegaMeshBuffer_DestroyBuffer(MegaMeshBuffer* megaMeshBuffer, VmaAllocator& allocator)
	{
		//if no mesh data
		if (!megaMeshBuffer->vertexBuffer.vertexCount)
			return;

		SMGraphics_Util_IndexBuffer_Destroy(&megaMeshBuffer->indexBuffer, allocator);
		SMGraphics_Util_VertexBuffer_Destroy(&megaMeshBuffer->vertexBuffer, allocator);
	}

	//creates the mega buffer
	inline void MegaMeshBuffer_CreateBuffer(MegaMeshBuffer* megaMeshBuffer, VmaAllocator& allocator,
		SMGraphics_Core_GPU* GPU, SMGraphics_Pool_CommandPool* commandPool)
	{
		//if it's not dirty, leave
		if (!megaMeshBuffer->isDirty)
			return;

		//if a buffer exists, we destroy it, the func checks if it exists.
		//It probably is a waste of a instruction, going into a func, then leaving
		//but it's inline so it might be optimized away, who knows
		MegaMeshBuffer_DestroyBuffer(megaMeshBuffer, allocator);

		//creates vertex buffer
		SMGraphics_Util_VertexBuffer_CreateInfo vertexBufferCreateInfo;
		vertexBufferCreateInfo.vertexBufferElementMemSize = sizeof(Smok::Mesh::Vertex);
		vertexBufferCreateInfo.verticesCount = megaMeshBuffer->vertices.size();
		vertexBufferCreateInfo.vertices = megaMeshBuffer->vertices.data();

		SMGraphics_Util_VertexBuffer_InitalizeDefaultValues(&megaMeshBuffer->vertexBuffer);
		SMGraphics_Util_VertexBuffer_Create(&megaMeshBuffer->vertexBuffer, &vertexBufferCreateInfo, allocator, commandPool, GPU);

		//creates index buffer
		SMGraphics_Util_IndexBuffer_CreateInfo indexBufferCreateInfo;
		indexBufferCreateInfo.indicesCount = megaMeshBuffer->indices.size();
		indexBufferCreateInfo.indices = megaMeshBuffer->indices.data();

		SMGraphics_Util_IndexBuffer_Create(&megaMeshBuffer->indexBuffer, &indexBufferCreateInfo,
			allocator, commandPool, GPU);

		megaMeshBuffer->isDirty = false;
	}

	//binds the vertex buffer of the mega mesh buffer
	inline void MegaMeshBuffer_Bind(MegaMeshBuffer* megaMeshBuffer, VkCommandBuffer& comBuffer)
	{
		if (!megaMeshBuffer->vertexBuffer.vertexCount)
			return;

		SMGraphics_Util_VertexBuffer_Bind(comBuffer, &megaMeshBuffer->vertexBuffer, 0);
		SMGraphics_Util_IndexBuffer_Bind(comBuffer, &megaMeshBuffer->indexBuffer, 0);
	}

	//binds and draws the index buffer of the chosen mesh
	inline void MegaMeshBuffer_Draw(MegaMeshBuffer* megaMeshBuffer, VkCommandBuffer& comBuffer, const uint32& meshOffsetIndex,
		const uint32& instanceStart, const uint32& instanceCount)
	{
		if (!megaMeshBuffer->vertexBuffer.vertexCount)
			return;

		//draws the buffer
		MeshOffset* meshOffset = &megaMeshBuffer->meshOffsets[meshOffsetIndex];
		SMGraphics_Util_IndexBuffer_Draw(comBuffer,
			&megaMeshBuffer->indexBuffer,
			meshOffset->indexStart, meshOffset->indexCount,
			instanceCount, instanceStart);
	}
}