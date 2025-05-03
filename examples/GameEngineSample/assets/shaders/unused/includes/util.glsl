#ifndef UTIL_GLSL
#define UTIL_GLSL

uint flatten_2d(in uvec2 index, in uint dimensionsX) {
	return index.x + index.y * dimensionsX;
}

uvec2 unflatten_2d(in uint index, in uint dimensionsX) {
	return uvec2(index % dimensionsX, index / dimensionsX);
}

uint rng_make_seed(in uint seed) {
	const uint state = seed * 747796405u + 2891336453u;
	const uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

uint rng_make_seed(in uvec2 pixel, in uvec2 resolution, in uint frameIndex) {
	const uint rngState = flatten_2d(pixel, resolution.x) ^ rng_make_seed(frameIndex);
	return rng_make_seed(rngState);
}

uint xor_shift(inout uint rng_state) {
	// Xorshift algorithm from George Marsaglia's paper.
	rng_state ^= (rng_state << 13);
	rng_state ^= (rng_state >> 17);
	rng_state ^= (rng_state << 5);
	return rng_state;
}

float rng_random_01(inout uint rng_state) {
	return uintBitsToFloat(0x3f800000 | xor_shift(rng_state) >> 9) - 1.0;
}

vec2 rng_random_vec2_01(inout uint rng_state) {
	return vec2(rng_random_01(rng_state), rng_random_01(rng_state));
}

vec3 rng_random_vec3_01(inout uint rng_state) {
	return vec3(rng_random_01(rng_state), rng_random_01(rng_state), rng_random_01(rng_state));
}

vec4 rng_random_vec4_01(inout uint rng_state) {
	return vec4(rng_random_01(rng_state), rng_random_01(rng_state), rng_random_01(rng_state), rng_random_01(rng_state));
}

// Based on http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
float random(vec2 co) {
	const float a = 12.9898f;
	const float b = 78.233f;
	const float c = 43758.5453f;
	const float dt= dot(co.xy ,vec2(a,b));
	const float sn= mod(dt,3.14f);
	return fract(sin(sn) * c);
}

vec3 position_from_depth(vec2 uv, float d, in mat4 converter) {
	vec4 clip = vec4(uv * 2.0 - 1.0, d, 1.0);
	vec4 pos = converter * clip;
	return vec3(pos.xyz / pos.w);
}

vec2 clip_to_uv(in vec2 clip) {
	return clip * 0.5f + 0.5f;
}

float linearize_depth(in float z, in float n, in float f) {
	return (2.0f * n * f) / (f + n - z * (f - n));
}

vec2 encode_velocity(in vec2 velocity)
{
#ifdef LOW_PRECISION_VELOCITY
    return pow(velocity * 0.5 + 0.5, vec2(3.0));
#else
    return velocity * 0.5 + 0.5;
#endif
}

vec2 decode_velocity(in vec2 velocity)
{
#ifdef LOW_PRECISION_VELOCITY
    return pow(velocity, vec2(1.0 / 3.0)) * 2.0 - 1.0;
#else
    return velocity * 2.0 - 1.0;
#endif
}


vec2 barrelDistortion(vec2 coord, float amt) {
	vec2 cc = coord - 0.5;
	float dist = dot(cc, cc);
	return coord + cc * dist * amt;
}

float saturate(float t) { return clamp(t, 0.0, 1.0); }
vec2 saturate(vec2 t) { return clamp(t, vec2(0.0), vec2(1.0)); }
vec3 saturate(vec3 t) { return clamp(t, vec3(0.0), vec3(1.0)); }
vec4 saturate(vec4 t) { return clamp(t, vec4(0.0), vec4(1.0)); }

float linterp( float t ) {
	return saturate( 1.0 - abs( 2.0*t - 1.0 ) );
}

float remap( float t, float a, float b ) {
	return saturate( (t - a) / (b - a) );
}

vec4 spectrum_offset( float t ) {
	vec4 ret;
	float lo = step(t,0.5);
	float hi = 1.0-lo;
	float w = linterp( remap( t, 1.0/6.0, 5.0/6.0 ) );
	ret = vec4(lo,1.0,hi, 1.) * vec4(1.0-w, w, 1.0-w, 1.);

	return pow( ret, vec4(1.0/2.2) );
}

vec3 convertXYZ2Yxy(vec3 _xyz)
{
	// Reference(s):
	// - XYZ to xyY
	//   https://web.archive.org/web/20191027010144/http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_xyY.html
	float inv = 1.0/dot(_xyz, vec3(1.0, 1.0, 1.0) );
	return vec3(_xyz.y, _xyz.x*inv, _xyz.y*inv);
}

vec3 convertXYZ2RGB(vec3 _xyz)
{
	vec3 rgb;
	rgb.x = dot(vec3( 3.2404542, -1.5371385, -0.4985314), _xyz);
	rgb.y = dot(vec3(-0.9692660,  1.8760108,  0.0415560), _xyz);
	rgb.z = dot(vec3( 0.0556434, -0.2040259,  1.0572252), _xyz);
	return rgb;
}

vec3 convertRGB2XYZ(vec3 _rgb)
{
	// Reference(s):
	// - RGB/XYZ Matrices
	//   https://web.archive.org/web/20191027010220/http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
	vec3 xyz;
	xyz.x = dot(vec3(0.4124564, 0.3575761, 0.1804375), _rgb);
	xyz.y = dot(vec3(0.2126729, 0.7151522, 0.0721750), _rgb);
	xyz.z = dot(vec3(0.0193339, 0.1191920, 0.9503041), _rgb);
	return xyz;
}

vec3 convertYxy2XYZ(vec3 _Yxy)
{
	// Reference(s):
	// - xyY to XYZ
	//   https://web.archive.org/web/20191027010036/http://www.brucelindbloom.com/index.html?Eqn_xyY_to_XYZ.html
	vec3 xyz;
	xyz.x = _Yxy.x*_Yxy.y/_Yxy.z;
	xyz.y = _Yxy.x;
	xyz.z = _Yxy.x*(1.0 - _Yxy.y - _Yxy.z)/_Yxy.z;
	return xyz;
}

vec3 convertRGB2Yxy(vec3 _rgb)
{
	return convertXYZ2Yxy(convertRGB2XYZ(_rgb) );
}

vec3 convertYxy2RGB(vec3 _Yxy)
{
	return convertXYZ2RGB(convertYxy2XYZ(_Yxy) );
}

float reinhard2(float _x, float _whiteSqr)
{
	return (_x * (1.0 + _x/_whiteSqr) ) / (1.0 + _x);
}

vec3 toGamma(vec3 _rgb, float gamma)
{
	return pow(abs(_rgb), vec3(1.0 / gamma) );
}

#endif // UTIL_GLSL