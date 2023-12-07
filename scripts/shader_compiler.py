import os, subprocess
from argparse import ArgumentParser

class ShaderCompiler:
    def __init__(self, shader_lang : str, build_type : str):
        self.shader_lang = shader_lang
        self.build_type = build_type

    def execute(self):
        engine_workdir = os.path.normpath(os.getcwd())

        embed_path = os.path.join(engine_workdir, "embed")
        engine_binaries = os.path.normpath(os.path.join(engine_workdir, f"bin/{self.build_type}/engine"))
        embed_binaries = os.path.join(engine_binaries, "embed")
        shader_compiler_exec = os.path.normpath(os.path.join(engine_workdir, f"bin/{self.build_type}/tools/shader_compiler/shader_compiler"))
        rendering_path = os.path.join(embed_binaries, "rendering")
        
        if not os.path.exists(embed_binaries):
            os.makedirs(embed_binaries)

        if not os.path.exists(rendering_path):
            os.makedirs(rendering_path)

        shader_compiler_api = self.shader_lang.lower()
        shader_compile_type = self.build_type.lower()
        shader_compile_type = "debug"

        shader_input_directory = os.path.join(embed_path, "shaders")
        shader_output_directory = os.path.join(rendering_path, "shader_cache.shc")
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
	
        subprocess.Popen(shader_compiler_comand).communicate()

def main():
    parser = ArgumentParser(description='Shader compiler script.')
    parser.add_argument('-slang', type=str, action='store', default='vk11', help='Shader language.')
    parser.add_argument('-build_type', type=str, action='store', default='Debug', help='Project build type.')
    args = parser.parse_args()

    os.chdir("../")
    compiler = ShaderCompiler(args.slang, args.build_type)
    compiler.execute()

if __name__ == "__main__":
    main()