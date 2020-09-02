#ifndef DL_DECODE_H
#define DL_DECODE_H

#include <PR/gbi.h>

// This header contains functions to decode displaylist commands
// They all assume that f3d2 is used

/**
 * @brief Returns the opcode of a displaylist command
 */
inline int8_t dl_cmd(const Gfx& gfx)
{
    return (gfx.words.w0 >> 24) & 0x00FF;
}

/**
 * @brief Returns a pointer contained in a displaylist command
 */
inline void* dl_ptr(const Gfx& gfx)
{
    return (void*) gfx.words.w1;
}

/**
 * @brief Returns the number of vertices to be loaded
 */
inline size_t dl_vtx_count(const Gfx& gfx)
{
    return (gfx.words.w0 >> 12) & 0x00FF;
}

/**
 * @brief Returns the start position in the RDP's vertex buffer
 */
inline size_t dl_vtx_start(const Gfx& gfx)
{
    return ((gfx.words.w0 >> 1) & 0x3F) - ((gfx.words.w0 >> 12) & 0xFF);
}

/**
 * @brief Returns the first triangle's indices as a uint8_t array
 */
inline uint8_t* dl_tri1_verts(const Gfx& gfx)
{
    return (uint8_t*)&(gfx.words.w0);
}

/**
 * @brief Returns the second triangle's indices as a uint8_t array
 */
inline uint8_t* dl_tri2_verts(const Gfx& gfx)
{
    return (uint8_t*)&(gfx.words.w1);
}

#endif