#include "debug_text.glsl"

layout (location = 0) in vec2 inPos;

out gl_PerVertex { vec4 gl_Position; };
layout (location = 0) out vec2 outUV;
layout (location = 1) flat out uint32_t outWord;
layout (location = 2) out vec4 outFGColor;
layout (location = 3) out vec4 outBGColor;

void main() 
{
	DebugTextData word_data = draw_data.word_data[gl_InstanceIndex].data;

	vec2 scale_factor = vec2(1.0, 2.0) * draw_data.inv_viewport_resolution;

	outWord = word_data.word;
	outFGColor = vec4(word_data.fg_color.r / 255.0, word_data.fg_color.g / 255.0, word_data.fg_color.b / 255.0, word_data.fg_color.a / 255.0);
	outBGColor = vec4(word_data.bg_color.r / 255.0, word_data.bg_color.g / 255.0, word_data.bg_color.b / 255.0, word_data.bg_color.a / 255.0);
	outUV = vec2((12 >> gl_VertexIndex) &1, (9 >> gl_VertexIndex ) &1);

	gl_Position = vec4(vec2(-1.0, -1.0) + (inPos * word_data.scale + word_data.pos) * scale_factor, 0.0, 1.0);
}