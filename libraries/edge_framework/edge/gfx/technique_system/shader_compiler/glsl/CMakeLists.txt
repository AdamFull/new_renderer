#glslang
set(BUILD_EXTERNAL OFF CACHE BOOL "Build external dependencies in /External" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "Builds glslangValidator and spirv-remap" FORCE)
set(ENABLE_CTEST OFF CACHE BOOL "Enables testing" FORCE)

find_package(glslang CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} 
	PRIVATE
	glslang::glslang
	glslang::SPIRV
)

target_sources(${PROJECT_NAME} PRIVATE
	"gfx_glsl_shader_compiler.cpp" 
)