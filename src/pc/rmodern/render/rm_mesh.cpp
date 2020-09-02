#include "rm_vk.h"

#include <stdexcept>
#include <vector>
#include <iostream>

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
#include <PR/gbi.h>

#include "dl_decode.h"

#define RDP_MAX_VERTICES 32

void rm_mesh::preloadFromDL(const Gfx* displayList)
{
	/*** DETERMINE NEEDED BUFFER SIZES ***/
	uint32_t numVertices = 0;
	uint32_t numIndices = 0;

	int8_t cmd = dl_cmd(displayList[0]);

	for (size_t i = 0; cmd != (int8_t)G_ENDDL; cmd = dl_cmd(displayList[++i]))
	{
		switch (cmd)
		{
		case G_MOVEMEM:
			break;
		case G_VTX:
			numVertices += dl_vtx_count(displayList[i]);	// number of vertices
			break;
		case G_TRI1:
			numIndices += 3;
			break;
	#ifdef G_TRI2
		case G_TRI2:
			numIndices += 6;
			break;
	#endif
		default:
			break;
		}
	}

	/*** ALLOCATE BUFFERS ***/

	Vtx* dlVertices = new Vtx[numVertices];
	rm_vtx* finalVertices = new rm_vtx[numVertices];
	uint32_t* indices = new uint32_t[numIndices];
	uint32_t dlVertexMap[RDP_MAX_VERTICES];

	for (size_t i = 0; i < RDP_MAX_VERTICES; i++)
		dlVertexMap[i] = 0;

	uint32_t nextVertex = 0;
	uint32_t nextIndex = 0;

	/*** PARSE DISPLAYLIST TO FILL BUFFERS ***/

	cmd = dl_cmd(displayList[0]);

	for (size_t i = 0; cmd != (int8_t)G_ENDDL; cmd = dl_cmd(displayList[++i]))
	{
		switch (cmd)
		{
		case G_MOVEMEM:
			break;
		case G_VTX:
			{
				Vtx* vtx = (Vtx*)dl_ptr(displayList[i]);						// vertex data source
				uint16_t numNewVertices = dl_vtx_count(displayList[i]);			// number of vertices
				size_t nStart = dl_vtx_start(displayList[i]);					// start position in vertex buffer

																					// TODO: make sure this is right
				uint32_t mapPos = nextVertex;										// start position in map
				uint32_t initMapPos = mapPos;

				// Update map
				for (size_t j = nStart; j < nStart + numNewVertices; j++)
					dlVertexMap[j] = mapPos++;
				
				// Push vertices onto dlVertices
				memcpy(&dlVertices[nextVertex], vtx, sizeof(Vtx) * numNewVertices);
				nextVertex += numNewVertices;
				//dlVertices.reserve(dlVertices.size() + numNewVertices);
				//dlVertices.insert(dlVertices.end(), vtx, &vtx[numNewVertices]);
			}
			break;
		case G_TRI1:
			{
				// load this triangle's vertex data
				const uint8_t* cVertices = dl_tri1_verts(displayList[i]);

				// map the indices and push them onto the index vector
				uint32_t mappedIndices[3];
				for (size_t i = 0; i < 3; i++)
					mappedIndices[i] = dlVertexMap[cVertices[2 - i]/2];

				// push new indices
				memcpy(&indices[nextIndex], mappedIndices, sizeof(uint32_t) * 3);
				nextIndex += 3;
			}
			break;
	#ifdef G_TRI2
		case G_TRI2:
			{
				// load this triangle's vertex data
				const uint8_t* cVertices0 = dl_tri1_verts(displayList[i]);
				const uint8_t* cVertices1 = dl_tri2_verts(displayList[i]);

				// map the indices and push them onto the index vector
				uint32_t mappedIndices[6];
				for (size_t i = 0; i < 3; i++)
					mappedIndices[i] = dlVertexMap[cVertices0[2 - i]/2];
				for (size_t i = 3; i < 6; i++)
					mappedIndices[i] = dlVertexMap[cVertices1[2 - (i - 3)]/2];

				// push new indices
				memcpy(&indices[nextIndex], mappedIndices, sizeof(uint32_t) * 6);
				nextIndex += 6;
			}
			break;
	#endif
		default:
			break;
		}
	}

	std::cout << "Loaded " << numVertices << " vertices!!" << std::endl;

	// Translate DL Vertices into RAPI_VULKAN vertices

	for (size_t i = 0; i < numVertices; i++)
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
	this->preload(numVertices, finalVertices, numIndices, indices);

	// Cleanup
	delete[] dlVertices;
	delete[] finalVertices;
	delete[] indices;
}
