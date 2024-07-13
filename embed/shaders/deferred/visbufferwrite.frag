#version 450

#include "shader_inputs"
#include "util"

// Force early depth/stencil test
layout(early_fragment_tests) in;

layout(location = 0) in flat uint drawID;
layout(location = 1) in flat uint instanceID;
layout(location = 2) in flat uint vboID;
layout(location = 3) in vec2 texCoord;
//layout(location = 5) smooth in vec4 currentPos;
//layout(location = 6) smooth in vec4 oldPos;

layout(location = 0) out vec4 vbid;
layout(location = 1) out vec4 vbinst;
//layout(location = 3) out vec2 mv;

// Engel's packing function (without alpha bit)
uint packVBID(bool opaque, uint drawID, uint primitiveID)
{
	uint drawID_primID = ((drawID << 23) & 0x7F800000) | (primitiveID & 0x007FFFFF);
	return (opaque) ? drawID_primID : (1 << 31) | drawID_primID;
}

uint packMOID(uint matID, uint objID)
{
	return (objID << 16) | (matID & 0xFFFF);
}

void main() 
{
	uint32_t draw_index = drawID + instanceID;
	FGeometryBatch draw_info = getDrawInfo(draw_index);
	FMesh mesh = getMesh(draw_info.meshID);
	FMaterial material = GetMaterial(mesh.materialID);
	
	float opacity = material.baseColorFactor.a;
	if(material.baseColorTexture != INVALID_HANDLE)
		opacity = sampleTexture2D(material.baseColorTexture, 0, texCoord).a;
	
	if(material.alphaMode == 1u && opacity < material.alphaCutoff)
		discard;

	vbid = unpackUnorm4x8(packVBID(true, drawID, gl_PrimitiveID + 1u));
	vbinst = unpackUnorm4x8(packMOID(vboID, instanceID));

	// Caclulate motion vectors
	//mv = encodeVelocity(currentPos.xy - oldPos.xy);
}