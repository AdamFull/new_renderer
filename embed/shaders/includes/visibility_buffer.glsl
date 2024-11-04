#ifndef VISIBILITY_BUFFER_GLSL
#define VISIBILITY_BUFFER_GLSL

#include "shader_inputs"

// Input buffers
layout(set = 1, binding = 1) uniform sampler2D visbuffer_geometry;
layout(set = 1, binding = 2) uniform sampler2D visbuffer_vboinst;

layout(std140, set = 1, binding = 4) uniform GeometryBuffers
{
	FGPUBufferDesc vb;
	FGPUBufferDesc ib;
	FGPUBufferDesc mb; 
	FGPUBufferDesc vrb;
	FGPUBufferDesc bb;
	FGPUBufferDesc tb;
	FGPUBufferDesc lb;
};

// Visbuffer structures
struct VertexAttribute
{
	vec3 pos;
	vec3 pos_dx;
	vec3 pos_dy;

	float linearZ;

	vec2 uv;
	vec2 uv_dx;
	vec2 uv_dy;

	vec3 normal;
	vec4 tangent;
	vec4 color;
	
	vec3 barycentrics;
};

struct BarycentricDeriv
{
	vec3 barycentrics;
	vec3 ddx;
	vec3 ddy;
};

struct VisibilityBuffer
{
	uint drawID;
	uint triangleID;
	uint transparent;

	uint matID;
	uint objID;

	uint vboID;
	uint cameraID;
};

bool loadVisBuffer(in vec2 uv, out VisibilityBuffer res)
{
	uint geometryData = packUnorm4x8(texture(visbuffer_geometry, uv));

	res.drawID = (geometryData >> 23) & 0x000000FF;
	res.triangleID = (geometryData & 0x007FFFFF);
	res.transparent = (geometryData >> 31);
	res.triangleID -= 1u;

	if(res.triangleID == 0xFFFFFFFFu)
		return false;

	uint vboinstData = packUnorm4x8(texture(visbuffer_vboinst, uv));
	res.vboID = vboinstData & 0xFFFF;
	uint instID = vboinstData >> 16;

	uint32_t draw_index = res.drawID + instID;
	FGeometryBatch draw_info = getDrawInfo(draw_index);
	FMesh mesh = getMesh(draw_info.meshID);
	
	res.objID = draw_info.nodeID;
	res.matID = mesh.materialID;
	res.cameraID = draw_info.cameraID;

	return true;
}

FRawVertex[3] loadTriangle(in uint drawID, in uint primID)
{
	FRawVertex[3] vertices;

	FDrawIndexedIndirectData draw_command = getDrawIndexedCommand(drawID);

	// Index of the first vertex of this Draw call's geometry
	uint startIndex = draw_command.firstIndex;

	// Get position in the Index Buffer of each vertex in this triangle (eg 31, 32, 33)
	uint triVert1IndexBufferPosition = (primID * 3 + 0) + startIndex;
	uint triVert2IndexBufferPosition = (primID * 3 + 1) + startIndex;
	uint triVert3IndexBufferPosition = (primID * 3 + 2) + startIndex;	

	// Now get vertex index from index buffer (eg, 17, 41, 32)
	Index triVert0Index = GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(RawIndexPtr, ib), triVert1IndexBufferPosition);
	Index triVert1Index = GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(RawIndexPtr, ib), triVert2IndexBufferPosition); 
	Index triVert2Index = GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(RawIndexPtr, ib), triVert3IndexBufferPosition);

	// Load vertex data of the 3 control points 
	vertices[0] = GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(RawVertexPtr, vb), draw_command.vertexOffset + triVert0Index);
	vertices[1] = GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(RawVertexPtr, vb), draw_command.vertexOffset + triVert1Index);
	vertices[2] = GET_VALUE_PTR_BY_ADDRESS(GET_BUFFER_POINTER(RawVertexPtr, vb), draw_command.vertexOffset + triVert2Index);

	return vertices;
}

// Computes the partial derivatives of a triangle from the projected screen space vertices
// Based on: http://filmicworlds.com/blog/visibility-buffer-rendering-with-material-graphs/
BarycentricDeriv computeBarycentrics(vec4 vertexCS0, vec4 vertexCS1, vec4 vertexCS2, vec2 pixelCS, vec2 invSize)
{
	BarycentricDeriv ret ;

	vec3 invW =  1.0f / vec3(vertexCS0.w, vertexCS1.w, vertexCS2.w);

	vec2 ndc0 = vertexCS0.xy * invW.x;
	vec2 ndc1 = vertexCS1.xy * invW.y;
	vec2 ndc2 = vertexCS2.xy * invW.z;

	float invDet = 1.0 / determinant(mat2(ndc2 - ndc1, ndc0 - ndc1));
	ret.ddx = vec3(ndc1.y - ndc2.y, ndc2.y - ndc0.y, ndc0.y - ndc1.y) * invDet * invW;
	ret.ddy = vec3(ndc2.x - ndc1.x, ndc0.x - ndc2.x, ndc1.x - ndc0.x) * invDet * invW;
	float ddxSum = dot(ret.ddx, vec3(1.0,1.0,1.0));
	float ddySum = dot(ret.ddy, vec3(1.0,1.0,1.0));
	
	vec2 deltaVec = pixelCS - ndc0;
	float interpInvW = invW.x + deltaVec.x*ddxSum + deltaVec.y*ddySum;
	float interpW = 1.0 / interpInvW;

	ret.barycentrics.x = interpW * (invW[0] + deltaVec.x*ret.ddx.x + deltaVec.y*ret.ddy.x);
	ret.barycentrics.y = interpW * (0.0f    + deltaVec.x*ret.ddx.y + deltaVec.y*ret.ddy.y);
	ret.barycentrics.z = interpW * (0.0f    + deltaVec.x*ret.ddx.z + deltaVec.y*ret.ddy.z);

	ret.ddx *= (2.0f*invSize.x);
	ret.ddy *= (2.0f*invSize.y);
	ddxSum *= (2.0f*invSize.x);
	ddySum *= (2.0f*invSize.y);

	float interpW_ddx = 1.0f / (interpInvW + ddxSum);
	float interpW_ddy = 1.0f / (interpInvW + ddySum);

	ret.ddx = interpW_ddx*(ret.barycentrics*interpInvW + ret.ddx) - ret.barycentrics;
	ret.ddy = interpW_ddy*(ret.barycentrics*interpInvW + ret.ddy) - ret.barycentrics;  

	return ret;
}

float interpolate(float a, float b, float c, vec3 barycentrics)
{
	return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec2 interpolate(vec2 a, vec2 b, vec2 c, vec3 barycentrics)
{
	return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec3 interpolate(vec3 a, vec3 b, vec3 c, vec3 barycentrics)
{
	return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec4 interpolate(vec4 a, vec4 b, vec4 c, vec3 barycentrics)
{
	return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

VertexAttribute getVertexData(vec2 uv, in VisibilityBuffer visbuffer)
{
	FRawVertex[3] vertices = loadTriangle(visbuffer.drawID, visbuffer.triangleID);

	// Load vertex data of the 3 vertices
	vec3 position0 = vec3(vertices[0].px, vertices[0].py, vertices[0].pz);
	vec3 position1 = vec3(vertices[1].px, vertices[1].py, vertices[1].pz);
	vec3 position2 = vec3(vertices[2].px, vertices[2].py, vertices[2].pz);

	// Load mesh model matrix
	FSceneNode node = getSceneNode(visbuffer.objID);
	mat4 model = getModelMatrix(node.matrixOffset);
			
#ifdef HAS_SKIN
	if(node.skeletonID != ~0u)
	{
#if defined(JOINTS_8BIT)
	u8vec4 joints;
	joints.x = uint8_t(vertices[0].joints_8 & 0xFF);
	joints.y = uint8_t((vertices[0].joints_8 >> 8) & 0xFF);
	joints.z = uint8_t((vertices[0].joints_8 >> 16) & 0xFF);
	joints.w = uint8_t((vertices[0].joints_8 >> 24) & 0xFF);
#elif defined(JOINTS_16BIT)
	u16vec4 joints;
#endif
		mat4 skinMat = 
			vertices[0].wx * getMatrixById(node.skeletonID, joints.x) +
			vertices[0].wy * getMatrixById(node.skeletonID, joints.y) +
			vertices[0].wz * getMatrixById(node.skeletonID, joints.z) +
			vertices[0].ww * getMatrixById(node.skeletonID, joints.w);
	
		model = model * skinMat;
	}
#endif // HAS_SKIN

	FCamera camera = GetCamera(visbuffer.cameraID);
	mat4 view = getViewMatrix(camera.matrices_offset);
	mat4 projection = getProjectionMatrix(camera.matrices_offset);

	// Transform positions to clip space
	mat4 mvp = projection * view * model;
	vec4 clipPos0 = mvp * vec4(position0, 1.0);
	vec4 clipPos1 = mvp * vec4(position1, 1.0);
	vec4 clipPos2 = mvp * vec4(position2, 1.0);

	//uv.y = 1.0 - uv.y;
	vec2 ndcClip = uv * 2.0 - 1.0;

	BarycentricDeriv bary = computeBarycentrics(clipPos0, clipPos1, clipPos2, ndcClip, camera.invSize);

	VertexAttribute res;

	// INTERPOLATE POSITION
	position0 = (model * vec4(position0, 1.0)).xyz;
	position1 = (model * vec4(position1, 1.0)).xyz;
	position2 = (model * vec4(position2, 1.0)).xyz;
	
	res.pos = interpolate(position0, position1, position2, bary.barycentrics);
	res.pos_dx = interpolate(position0, position1, position2, bary.ddx);
	res.pos_dy = interpolate(position0, position1, position2, bary.ddy);

	res.linearZ = interpolate(clipPos0.w, clipPos1.w, clipPos2.w, bary.barycentrics);
	res.barycentrics = bary.barycentrics;

	// INTERPOLATE TEXCOORD
#ifdef HAS_TEXCOORD
	vec2 uv0 = vec2(vertices[0].u, vertices[0].v);
	vec2 uv1 = vec2(vertices[1].u, vertices[1].v);
	vec2 uv2 = vec2(vertices[2].u, vertices[2].v);

	res.uv = interpolate(uv0, uv1, uv2, bary.barycentrics);
	res.uv_dx = interpolate(uv0, uv1, uv2, bary.ddx);
	res.uv_dy = interpolate(uv0, uv1, uv2, bary.ddy);
#endif

	// INTERPOLATE NORMAL
#ifdef HAS_NORMAL
	vec3 normal0 = vec3(vertices[0].nx, vertices[0].ny, vertices[0].nz);
	vec3 normal1 = vec3(vertices[1].nx, vertices[1].ny, vertices[1].nz);
	vec3 normal2 = vec3(vertices[2].nx, vertices[2].ny, vertices[2].nz);

	vec3 normal = interpolate(normal0, normal1, normal2, bary.barycentrics);
	res.normal = normalize(mat3(model) * normal);
#else
	res.normal = cross(res.pos_dx, res.pos_dy);
#endif // HAS_NORMAL

	// INTERPOLATE TANGENT
#if defined(TANGENT_VEC3) || defined(TANGENT_VEC4)
	vec4 tangent0 = vec4(vertices[0].tx, vertices[0].ty, vertices[0].tz, 1.0);
	vec4 tangent1 = vec4(vertices[1].tx, vertices[1].ty, vertices[1].tz, 1.0);
	vec4 tangent2 = vec4(vertices[2].tx, vertices[2].ty, vertices[2].tz, 1.0);
#if defined(TANGENT_VEC4)
	tangent0.w = vertices[0].tw;
	tangent1.w = vertices[1].tw;
	tangent2.w = vertices[2].tw;
#endif // TANGENT_VEC4

	vec4 tangent = interpolate(tangent0, tangent1, tangent2, bary.barycentrics);
	res.tangent = vec4(normalize(mat3(model) * vec3(tangent)), tangent.w);
#else	
	vec3 tangent = (res.uv_dy.t * res.pos_dx - res.uv_dx.t * res.pos_dy) / (res.uv_dx.s * res.uv_dy.t - res.uv_dy.s * res.uv_dx.t); 
	tangent = normalize(tangent - res.normal * dot(res.normal, tangent));
	res.tangent = vec4(tangent, 1.0);
#endif // TANGENT

	// INTERPOLATE COLOR
#if defined(COLOR_VEC3) || defined(COLOR_VEC4)
	vec4 color0 = vec4(vertices[0].cr, vertices[0].cg, vertices[0].cb, 1.0);
	vec4 color1 = vec4(vertices[1].cr, vertices[1].cg, vertices[1].cb, 1.0);
	vec4 color2 = vec4(vertices[2].cr, vertices[2].cg, vertices[2].cb, 1.0);
#if defined(COLOR_VEC4)
	color0.a = vertices[0].ca;
	color1.a = vertices[1].ca;
	color2.a = vertices[2].ca;
#endif // COLOR_VEC4
	res.color = interpolate(color0, color1, color2, bary.barycentrics);
#else
	res.color = vec4(1.0);
#endif // COLOR

	return res;
}

#define SAMPLE_COUNT 8

//void resolveVisBuffer(in vec2 uv, out VisibilityBuffer vb)
//{	
//	ivec2 dim = textureSize(visbuffer_geometry);
//	ivec2 px = ivec2(uv * dim);
//
//	[[unroll]]
//	for(int i = 0; i < NUM_SAMPLES; i++)
//	{
//		vec4 vgeom = texelFetch(visbuffer_geometry, px, i); 
//
//		vb.drawID = (geometryData >> 23) & 0x000000FF;
//		vb.triangleID = (geometryData & 0x007FFFFF);
//		vb.transparent = (geometryData >> 31);
//		vb.triangleID -= 1u;
//
//		if(vb.triangleID == 0xFFFFFFFFu)
//			continue;
//
//		uint vboinstData = packUnorm4x8(texelFetch(visbuffer_vboinst, px, i));
//		vb.vboID = vboinstData & 0xFFFF;
//		uint instID = vboinstData >> 16;
//
//		uint32_t draw_index = vb.drawID + instID;
//		FDrawMeshinfo draw_info = getDrawInfo(draw_index);
//		FMesh mesh = getMesh(draw_info.meshID);
//		
//		vb.objID = draw_info.nodeID;
//		vb.matID = mesh.materialID;
//
//		VertexAttribute va = getVertexData(uv, vb);
//	}
//}

#endif // VISIBILITY_BUFFER_GLSL