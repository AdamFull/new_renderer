add_subdirectory("shader_compiler/glsl")

target_sources(${PROJECT_NAME} PRIVATE
	"shader_compiler/gfx_shader_compiler.cpp"
	"gfx_technique_system.cpp"
)