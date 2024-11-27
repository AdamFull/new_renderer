#ifndef VISIBILITY_BUFFER_GLSL
#define VISIBILITY_BUFFER_GLSL

#include "shader_inputs.glsl"

// Input buffers
layout(set = 1, binding = 1) uniform sampler2D sample_image3;
layout(set = 1, binding = 2) uniform sampler2D sample_image4;

layout(push_constant) uniform UniformCallData
{
	uint32_t call_id;
	uint32_t camera_id;
} call_data;

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
};

bool loadVisBuffer(in vec2 uv, out VisibilityBuffer res)
{
	uint geometryData = packUnorm4x8(texture(sample_image3, uv));

	res.drawID = (geometryData >> 23) & 0x000000FF;
	res.triangleID = (geometryData & 0x007FFFFF);
	res.transparent = (geometryData >> 31);
	res.triangleID -= 1u;

	if(res.triangleID == 0xFFFFFFFFu)
		return false;

	uint vboinstData = packUnorm4x8(texture(sample_image4, uv));
	res.vboID = vboinstData & 0xFFFF;
	uint instID = vboinstData >> 16;

	uint32_t draw_index = res.drawID + instID;
	DrawDescription draw_info = GetDrawInfo(draw_index);
	Mesh mesh = GetMesh(draw_info.meshID);
	
	res.objID = draw_info.nodeID;
	res.matID = mesh.materialID;

	return true;
}

FRawVertex LoadVertex(in uint32_t index, in int32_t vertex_offset)
{
	FRawVertex vertex;

	uint32_t vertex_index = index + vertex_offset;

	vertex.position = GetVertexPosition(vertex_index);
	vertex.color = GetVertexColor(vertex_index);
	vertex.uv = GetVertexTexcoord(vertex_index);
	vertex.normal = GetVertexNormal(vertex_index);
	vertex.tangent = GetVertexTangent(vertex_index);
	vertex.joints = GetVertexJoints(vertex_index);
	vertex.weights = GetVertexWeights(vertex_index);

	return vertex;
}

FRawVertex[3] LoadTriangle(in uint drawID, in uint primID)
{
	FRawVertex[3] vertices;

	DrawIndexedIndirectData draw_command = GetDrawIndexedCommand(drawID);

	// Index of the first vertex of this Draw call's geometry
	uint startIndex = draw_command.firstIndex;

	// Get position in the Index Buffer of each vertex in this triangle (eg 31, 32, 33)
	uint triVert1IndexBufferPosition = (primID * 3 + 0) + startIndex;
	uint triVert2IndexBufferPosition = (primID * 3 + 1) + startIndex;
	uint triVert3IndexBufferPosition = (primID * 3 + 2) + startIndex;	

	// Now get vertex index from index buffer (eg, 17, 41, 32)
	Index triVert0Index = GetIndex(triVert1IndexBufferPosition);
	Index triVert1Index = GetIndex(triVert2IndexBufferPosition); 
	Index triVert2Index = GetIndex(triVert3IndexBufferPosition);

	// Load vertex data of the 3 control points 
	vertices[0] = LoadVertex(triVert0Index, draw_command.vertexOffset);
	vertices[1] = LoadVertex(triVert1Index, draw_command.vertexOffset);
	vertices[2] = LoadVertex(triVert2Index, draw_command.vertexOffset);

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
	FRawVertex[3] vertices = LoadTriangle(visbuffer.drawID, visbuffer.triangleID);

	// Load vertex data of the 3 vertices
	vec3 position0 = vertices[0].position;
	vec3 position1 = vertices[1].position;
	vec3 position2 = vertices[2].position;

	// Load mesh model matrix
	SceneNode node = GetSceneNode(visbuffer.objID);
	mat4 model = GetModelMatrix(node.matrixOffset);

	Camera camera = GetCamera(call_data.camera_id);
	mat4 view_projection = GetViewProjectionMatrix(camera.matrices_offset);
			
#ifdef HAS_SKIN
//	if(node.skeletonID != ~0u)
//	{
	Joints joints0 = vertices[0].joints;
	Joints joints1 = vertices[1].joints;
	Joints joints2 = vertices[2].joints;

	vec4 weights0 = vertices[0].weights;
	vec4 weights1 = vertices[1].weights;
	vec4 weights2 = vertices[2].weights;

	mat4 skin_mat0 = 
		weights0.x * GetMatrixById(node.skeletonID, joints0.x) +
		weights0.y * GetMatrixById(node.skeletonID, joints0.y) +
		weights0.z * GetMatrixById(node.skeletonID, joints0.z) +
		weights0.w * GetMatrixById(node.skeletonID, joints0.w);

	mat4 skin_mat1 = 
		weights1.x * GetMatrixById(node.skeletonID, joints1.x) +
		weights1.y * GetMatrixById(node.skeletonID, joints1.y) +
		weights1.z * GetMatrixById(node.skeletonID, joints1.z) +
		weights1.w * GetMatrixById(node.skeletonID, joints1.w);

	mat4 skin_mat2 = 
		weights2.x * GetMatrixById(node.skeletonID, joints2.x) +
		weights2.y * GetMatrixById(node.skeletonID, joints2.y) +
		weights2.z * GetMatrixById(node.skeletonID, joints2.z) +
		weights2.w * GetMatrixById(node.skeletonID, joints2.w);

	mat4 mvp0 = view_projection * model * skin_mat0;
	mat4 mvp1 = view_projection * model * skin_mat1;
	mat4 mvp2 = view_projection * model * skin_mat2;

	// Transform positions to clip space
	vec4 clipPos0 = mvp0 * vec4(position0, 1.0);
	vec4 clipPos1 = mvp1 * vec4(position1, 1.0);
	vec4 clipPos2 = mvp2 * vec4(position2, 1.0);
//	}
#else
	// Transform positions to clip space
	mat4 mvp = view_projection * model;
	vec4 clipPos0 = mvp * vec4(position0, 1.0);
	vec4 clipPos1 = mvp * vec4(position1, 1.0);
	vec4 clipPos2 = mvp * vec4(position2, 1.0);
#endif // HAS_SKIN

	//uv.y = 1.0 - uv.y;
	vec2 ndcClip = uv * 2.0 - 1.0;

	BarycentricDeriv bary = computeBarycentrics(clipPos0, clipPos1, clipPos2, ndcClip, camera.invSize);

	VertexAttribute res;

	// INTERPOLATE POSITION
	// TODO: Skinning. Here i have to multiply to specific matrices
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
	vec2 uv0 = vertices[0].uv;
	vec2 uv1 = vertices[1].uv;
	vec2 uv2 = vertices[2].uv;

	res.uv = interpolate(uv0, uv1, uv2, bary.barycentrics);
	res.uv_dx = interpolate(uv0, uv1, uv2, bary.ddx);
	res.uv_dy = interpolate(uv0, uv1, uv2, bary.ddy);
#endif

	// INTERPOLATE NORMAL
#ifdef HAS_NORMAL
	vec3 normal0 = vertices[0].normal;
	vec3 normal1 = vertices[1].normal;
	vec3 normal2 = vertices[2].normal;

	vec3 normal = interpolate(normal0, normal1, normal2, bary.barycentrics);
	res.normal = normalize(mat3(model) * normal);
#else
	res.normal = cross(res.pos_dx, res.pos_dy);
#endif // HAS_NORMAL

	// INTERPOLATE TANGENT
#if HAS_TANGENT
	Tangent tangent0 = vertices[0].tangent;
	Tangent tangent1 = vertices[1].tangent;
	Tangent tangent2 = vertices[2].tangent;

	Tangent tangent = interpolate(tangent0, tangent1, tangent2, bary.barycentrics);
#if defined(TANGENT_VEC4)
	res.tangent = vec4(normalize(mat3(model) * vec3(tangent)), tangent.w);
#endif // TANGENT_VEC4
#else	
	vec3 tangent = (res.uv_dy.t * res.pos_dx - res.uv_dx.t * res.pos_dy) / (res.uv_dx.s * res.uv_dy.t - res.uv_dy.s * res.uv_dx.t); 
	tangent = normalize(tangent - res.normal * dot(res.normal, tangent));
	res.tangent = vec4(tangent, 1.0);
#endif // TANGENT

	// INTERPOLATE COLOR
#if HAS_COLOR
	Color color0 = vertices[0].color;
	Color color1 = vertices[1].color;
	Color color2 = vertices[2].color;

#if defined(COLOR_VEC3)
	res.color = vec4(interpolate(color0, color1, color2, bary.barycentrics), 1.f);
#else
	res.color = interpolate(color0, color1, color2, bary.barycentrics);
#endif // COLOR_VEC3

#else
	res.color = vec4(1.0);
#endif // COLOR

	return res;
}

#define SAMPLE_COUNT 8

//void resolveVisBuffer(in vec2 uv, out VisibilityBuffer vb)
//{	
//	ivec2 dim = textureSize(sample_image3);
//	ivec2 px = ivec2(uv * dim);
//
//	[[unroll]]
//	for(int i = 0; i < NUM_SAMPLES; i++)
//	{
//		vec4 vgeom = texelFetch(sample_image3, px, i); 
//
//		vb.drawID = (geometryData >> 23) & 0x000000FF;
//		vb.triangleID = (geometryData & 0x007FFFFF);
//		vb.transparent = (geometryData >> 31);
//		vb.triangleID -= 1u;
//
//		if(vb.triangleID == 0xFFFFFFFFu)
//			continue;
//
//		uint vboinstData = packUnorm4x8(texelFetch(sample_image4, px, i));
//		vb.vboID = vboinstData & 0xFFFF;
//		uint instID = vboinstData >> 16;
//
//		uint32_t draw_index = vb.drawID + instID;
//		FDrawMeshinfo draw_info = GetDrawInfo(draw_index);
//		Mesh mesh = GetMesh(draw_info.meshID);
//		
//		vb.objID = draw_info.nodeID;
//		vb.matID = mesh.materialID;
//
//		VertexAttribute va = getVertexData(uv, vb);
//	}
//}

#endif // VISIBILITY_BUFFER_GLSL