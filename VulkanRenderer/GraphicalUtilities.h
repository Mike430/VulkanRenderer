#pragma once
namespace GraphicalUtilities
{
	struct vec4
	{
		float x, y, z, w;
	};

	struct colour
	{
		float r, g, b;
	};

	struct vertex
	{
		vec4 pos;
		colour colour;
	};

	struct indiceSet
	{
		uint32_t a, b, c;
	};

	struct polygon
	{
		vertex vA, vB, vC;
		indiceSet i;
	};
};

