add_subdirectory("shader_compiler/glsl")

target_sources(${PROJECT_NAME} PRIVATE
	"gfx_technique_system.cpp"
)