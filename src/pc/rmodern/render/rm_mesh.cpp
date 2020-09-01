#include "rm_vk.h"

#include <stdexcept>
#include <vector>
#include <iostream>

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
#include <PR/gbi.h>

int8_t getGfxCmd(const Gfx& gfx)
{
	return (gfx.words.w0 >> 24) & 0xFF;
}

void* getGfxPtr(const Gfx& gfx)
{
	return (void*)gfx.words.w1;
}

uint16_t getGfxLength(const Gfx& gfx)
{
	return (gfx.words.w0) & 0xFFFF;
}

uint8_t getGfxParam(const Gfx& gfx)
{
	return (gfx.words.w0 >> 16) & 0xFF;
}

const uint8_t* getGfxVertices(const Gfx& gfx)
{
	uint8_t* ptr = (uint8_t*)&(gfx.words.w1);
	return &ptr[0];
}

#define RDP_MAX_VERTICES 32

void rm_mesh::preloadFromDL(const Gfx* displayList)
{
	std::vector<Vtx> dlVertices = {};
	std::vector<uint32_t> indices = {};
	uint32_t dlVertexMap[RDP_MAX_VERTICES];

	for (size_t i = 0; i < RDP_MAX_VERTICES; i++)
		dlVertexMap[i] = 0;

	int8_t cmd = getGfxCmd(displayList[0]);

	for (size_t i = 0; cmd != (int8_t)G_ENDDL; cmd = getGfxCmd(displayList[++i]))
	{
		switch (cmd)
		{
		case G_MOVEMEM:
			break;
		case G_VTX:
			{
				Vtx* vtx = (Vtx*)getGfxPtr(displayList[i]);						// vertex data source
				uint16_t numVertices = getGfxLength(displayList[i]) / sizeof(Vtx);	// number of vertices
				uint8_t nStart = getGfxParam(displayList[i]) & 0x0F;				// start position in vertex buffer
																					// TODO: make sure this is right
				uint32_t mapPos = dlVertices.size();								// start position in map
				uint32_t initMapPos = mapPos;

				// Update map
				for (size_t j = nStart; j < nStart + numVertices; j++)
					dlVertexMap[j] = mapPos++;
				
				// Push vertices onto dlVertices
				dlVertices.insert(dlVertices.end(), vtx, &vtx[numVertices]);
			}
			break;
		case G_TRI1:
			{
				// load this triangle's vertex data
				const uint8_t* cVertices = getGfxVertices(displayList[i]);

				// map the indices and push them onto the index vector
				uint32_t mappedIndices[3];
				for (size_t i = 0; i < 3; i++)
					mappedIndices[i] = dlVertexMap[cVertices[2 - i]/10];

				indices.insert(indices.end(), mappedIndices, &mappedIndices[3]);
			}
			break;
		default:
			break;
		}
	}

	std::cout << "Loaded " << dlVertices.size() << " vertices!!" << std::endl;

	// Translate DL Vertices into RAPI_VULKAN vertices
	std::vector<rm_vtx> finalVertices;
	finalVertices.resize(dlVertices.size());

	for (size_t i = 0; i < dlVertices.size(); i++)
	{
		const Vtx_t* v = &dlVertices[i].v;

		for(size_t j=0; j<3; j++)
		{
			finalVertices[i].pos[j] = v->ob[j];
			finalVertices[i].color[j] = v->cn[j];
		}
		//finalVertices[i].color = { 1.0f, 1.0f, 1.0f };
	}

	// Load mesh using the virtual void "preload"
	this->preload(finalVertices, indices);
}