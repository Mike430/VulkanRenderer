#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

namespace GraphicalUtilities
{
	struct Vertex
	{
		vec3 pos;
		vec3 color;

		static VkVertexInputBindingDescription getBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof( Vertex );
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
		{
			array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof( Vertex, pos );

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof( Vertex, color );

			return attributeDescriptions;
		}
	};

	// Only triangles permitted
	struct TriIndiceSet
	{
		uint32_t a, b, c;
	};

	struct SingleObjMeshData
	{
		string name;
		vector<Vertex> verts;
		vector<TriIndiceSet> indicies;
	};

	struct GameMesh
	{
		SingleObjMeshData mesh;
		//Texture* texture;
	};
};

