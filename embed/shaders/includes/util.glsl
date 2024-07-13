#ifndef UTIL_GLSL
#endif UTIL_GLSL

vec4 sRGBToLinear(in vec4 srgbIn)
{
    vec3 bLess = step(vec3(0.04045),srgbIn.xyz);
	vec3 linOut = mix( srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
	return vec4(linOut,srgbIn.w);
}

vec4 linearTosRGB(vec4 linearRGB)
{
    bvec4 cutoff = lessThan(linearRGB, vec4(0.0031308));
    vec4 higher = vec4(1.055) * pow(linearRGB, vec4(1.0 / 2.4)) - vec4(0.055);
    vec4 lower = linearRGB * vec4(12.92);

    return mix(higher, lower, cutoff);
}

// From http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 Uncharted2Tonemap(vec3 color)
{
	float A = 0.15f;
	float B = 0.50f;
	float C = 0.10f;
	float D = 0.20f;
	float E = 0.02f;
	float F = 0.30f;
	float W = 11.2f;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec3 getTangentSpaceNormalMap(in vec3 normalColor, mat3 TBN, float scale)
{
    vec3 normal = normalize(TBN * ((2.0 * normalColor - 1.0) * vec3(scale, scale, 1.0)));
	return normal;
}

// Calculate normal in tangent space
mat3 calculateTBN(vec3 normal, vec4 tangent)
{
	vec3 N = normalize(normal);
	vec3 T = normalize(tangent.xyz);
    vec3 B = normalize(cross(N, T) * tangent.w);
	
	return mat3(T, B, N);
}

// Based omn http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
float random(vec2 co)
{
	float a = 12.9898f;
	float b = 78.233f;
	float c = 43758.5453f;
	float dt= dot(co.xy ,vec2(a,b));
	float sn= mod(dt,3.14f);
	return fract(sin(sn) * c);
}

vec3 getPositionFromDepth(vec2 texcoord, float depth, mat4 invViewProjection)
{
    vec4 clipSpaceLocation;
    clipSpaceLocation.x = texcoord.x * 2.0f - 1.0f;
    clipSpaceLocation.y = texcoord.y * 2.0f - 1.0f;
    clipSpaceLocation.z = depth;
    clipSpaceLocation.w = 1.0f;
    vec4 homogenousLocation = invViewProjection * clipSpaceLocation;
    return homogenousLocation.xyz / homogenousLocation.w;
}

#ifdef FRAGMENT_SHADER
vec3 getNormalFromDepth(in sampler2D depth_sampler, in vec2 uv, in mat4 invProjection)
{
    float depth = texture(depth_sampler, uv).x;

    vec4 clipSpace = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 viewSpace = invProjection * clipSpace;
    viewSpace.xyz /= viewSpace.w;

    vec3 pos = viewSpace.xyz;
    vec3 n = normalize(cross(dFdx(pos), dFdy(pos)));
    n *= -1;

    return n;
}
#endif

float linearizeDepth(in float depth, in float zNear, in float zFar)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

uvec4 packTextures(vec3 normal_map, vec3 albedo_map, vec4 pbr_map)
{
    uvec4 texture_pack;
    //Storing RG components of normal map in 32bit R component
    texture_pack.r = packHalf2x16(normal_map.rg);
    //Storing B component of normal map and R component of albedo in G
    texture_pack.g = packHalf2x16(vec2(normal_map.b, albedo_map.r));
    //Storing GB components of albedo in B
    texture_pack.b = packHalf2x16(albedo_map.gb);
    // Store metal, roughness, ao and heignt in A component
    texture_pack.a = packUnorm4x8(pbr_map);
    return texture_pack;
}

void unpackTextures(in uvec4 texture_pack, out vec3 normal_map, out vec3 albedo_map, out vec4 pbr_map)
{
    vec2 pack_R = unpackHalf2x16(texture_pack.r);
	vec2 pack_G = unpackHalf2x16(texture_pack.g);
	vec2 pack_B = unpackHalf2x16(texture_pack.b);
	pbr_map = unpackUnorm4x8(texture_pack.a);
    normal_map = vec3(pack_R, pack_G.r);
    albedo_map = vec3(pack_G.g, pack_B);
}

vec4 per_object_motion_blir(sampler2D velocity_texture, sampler2D color_texture, vec2 inUV, float velocityScale, int max_samples)
{
    vec2 texelSize = 1.0 / vec2(textureSize(color_texture, 0));
    vec2 velocity = texture(velocity_texture, inUV * vec2(1.0, -1.0)).rg * velocityScale;
    float speed = length(velocity / texelSize);
    int samples = clamp(int(speed), 1, max_samples);

    vec4 motionBlur = texture(color_texture, inUV);
    for (int i = 1; i < samples; ++i) 
    {
        vec2 offset = velocity * (float(i) / float(samples - 1) - 0.5);
        motionBlur += texture(color_texture, inUV + offset);
    }
    motionBlur /= float(samples);
    return motionBlur;
}

vec2 encodeVelocity(in vec2 velocity)
{
#ifdef LOW_PRECISION_VELOCITY
    return pow(velocity * 0.5 + 0.5, vec2(3.0));
#else
    return velocity * 0.5 + 0.5;
#endif
}

vec2 decodeVelocity(in vec2 velocity)
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

bool is_saturated(float a) { return a == saturate(a); }
bool is_saturated(vec2 a) { return a == saturate(a); }
bool is_saturated(vec3 a) { return a == saturate(a); }
bool is_saturated(vec4 a) { return a == saturate(a); }

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