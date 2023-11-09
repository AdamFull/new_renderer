import subprocess, os, shutil, json
from argparse import ArgumentParser
from types import SimpleNamespace

def main():
	parser = ArgumentParser(description='Project prepare script.')
	parser.add_argument('-build_type', type=str, action='store', help='Project build type.')
	args = parser.parse_args()

	build_type = args.build_type

	current_workdir = os.getcwd()
	engine_workdir = os.path.normpath(os.path.join(current_workdir, "../"))
	
	embed_path = os.path.join(engine_workdir, "embed")
	engine_binaries = os.path.join(engine_workdir, f"bin/{build_type}/engine")
	embed_binaries = os.path.join(engine_binaries, "embed")
	shader_compiler_exec = os.path.join(engine_workdir, f"bin/{build_type}/tools/shader_compiler/shader_compiler")

	if not os.path.exists(embed_binaries):
		os.makedirs(embed_binaries)

	shader_compiler_api = "vk13" # parse from config
	shader_compile_type = "release" # parse from config

	with open(os.path.join(embed_path, "engine_config.json"), "r") as configfile:
		engine_config = json.load(configfile, object_hook=lambda d: SimpleNamespace(**d))
		shader_compiler_api = engine_config.api

	shader_input_directory = os.path.join(embed_path, "shaders")
	shader_output_directory = os.path.join(embed_binaries, "shader_libraries")
	shader_common_directory = os.path.join(shader_input_directory, "common")
	shader_includes_directory = os.path.join(shader_input_directory, "includes")

	shader_compiler_comand = [
		shader_compiler_exec, 
		"-in", shader_input_directory,
		"-out", shader_output_directory,
		"-common", shader_common_directory,
		"-includes", shader_includes_directory,
		"-api", shader_compiler_api,
		"-build_type", shader_compile_type
		]
	
	#subprocess.Popen(shader_compiler_comand).communicate()

	shutil.copy(os.path.join(embed_path, "engine_config.json"), os.path.join(engine_binaries, "engine_config.json"))
	shutil.copytree(os.path.join(embed_path, "mono"), os.path.join(embed_binaries, "mono"))
	shutil.copytree(os.path.join(embed_path, "rendering"), os.path.join(embed_binaries, "rendering"))

if __name__ == "__main__":
	main()