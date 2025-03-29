target_link_libraries(${PROJECT_NAME} 
	PRIVATE
	glslang::glslang
	glslang::SPIRV
)

target_sources(${PROJECT_NAME} PRIVATE
	"gfx_glsl_shader_compiler.cpp" 
)