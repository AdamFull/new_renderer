import os
from argparse import ArgumentParser
from collections import deque
from types import SimpleNamespace

from engine_compiler import EngineCompiler
from shader_compiler import ShaderCompiler
from copy_embed import CopyEmbedded

def main():
    parser = ArgumentParser(description='Engine toolset script.')
    parser.add_argument('--compile_engine', action='store_true', help='Execute engine compiler work.')
    parser.add_argument('--compile_shaders', action='store_true', help='Execute shader compiler work.')
    parser.add_argument('--copy_embed', action='store_true', help='Copying embedded resources.')
    parser.add_argument('-slang', type=str, action='store', default='vk11', help='Shader language.')
    parser.add_argument('-platform', type=str, action='store', default='win64', help='Compile platform type.')
    parser.add_argument('-build_type', type=str, action='store', default='Debug', help='Project build type.')

    args = parser.parse_args()

    work_queue = deque()

    if args.compile_engine:
        work_queue.append(EngineCompiler(args.platform, args.build_type))

    if args.compile_shaders:
        work_queue.append(ShaderCompiler(args.slang, args.build_type))

    if args.copy_embed:
        work_queue.append(CopyEmbedded(args.build_type))

    os.chdir("../")
    while work_queue:
        work = work_queue.popleft()
        work.execute()

if __name__ == "__main__":
    main()

#with open(os.path.join(embed_path, "engine_config.json"), "r") as configfile:
#		engine_config = json.load(configfile, object_hook=lambda d: SimpleNamespace(**d))
#		shader_compiler_api = engine_config.api
