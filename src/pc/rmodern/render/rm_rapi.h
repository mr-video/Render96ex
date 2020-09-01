#ifndef RM_RAPI_H
#define RM_RAPI_H

#include "engine/graph_node.h"
#include <types.h>
#include <vector>

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
#include <PR/gbi.h>

class rm_wapi;

struct rm_vtx
{
    Vec3f pos;
    Vec3f color;
};

class rm_mesh
{
public:
    virtual void preload(std::vector<rm_vtx> vertices, std::vector<uint32_t> indices) = 0;
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void cleanup() = 0;
    virtual void render() = 0;

    void preloadFromDL(const Gfx* displayList);
};

struct RMMeshGraphNode
{
    struct GraphNode node;
    rm_mesh* mMesh;
};

class rm_rapi
{
public:
    virtual void setWAPI(rm_wapi* wapi) = 0;
    virtual bool checkSupport() = 0;
    virtual bool init() = 0;
    virtual void cleanup() = 0;
    virtual rm_mesh* createMesh() = 0;
    virtual uint32_t getRequiredWindowFlags() = 0;
    virtual void beginFrame() = 0;
    virtual void completeFrame() = 0;
};

#endif