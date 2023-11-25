import os, shutil, subprocess
from argparse import ArgumentParser

def copy_with_override(src, dst):
    src = os.path.normpath(src)
    dst = os.path.normpath(dst)

    if os.path.exists(dst):
        if os.path.isfile(dst):
            os.remove(dst)
        else:
            shutil.rmtree(dst)
    if os.path.isfile(src):
        shutil.copy(src, dst)
    else:
        shutil.copytree(src, dst)

class CopyEmbedded:
    def __init__(self, build_type : str):
        self.build_type = build_type

    def execute(self):
        engine_workdir = os.path.normpath(os.getcwd())

        embed_path = os.path.join(engine_workdir, "embed")
        engine_binaries = os.path.join(engine_workdir, f"bin/{self.build_type}/engine")
        
        #copy_with_override(os.path.join(embed_path, "engine_config.json"), os.path.join(engine_binaries, "engine_config.json"))
        #copy_with_override(os.path.join(embed_path, "mono"), os.path.join(embed_binaries, "mono"))

        for root, dirs, files in os.walk(embed_path):
            for file in files:
                if "shaders" in root:
                    continue

                relpath = os.path.relpath(root, engine_workdir)
                dst_dir = os.path.normpath(os.path.join(engine_binaries, relpath))

                src = os.path.join(root, file)
                dst = os.path.join(dst_dir, file)
                
                if not os.path.exists(dst_dir):
                    os.makedirs(dst_dir)

                copy_with_override(src, dst)

def main():
    parser = ArgumentParser(description='Embedder resource copyer.')
    parser.add_argument('-build_type', type=str, action='store', default='Debug', help='Project build type.')
    args = parser.parse_args()

    os.chdir("../")
    compiler = CopyEmbedded(args.build_type)
    compiler.execute()

if __name__ == "__main__":
    main()