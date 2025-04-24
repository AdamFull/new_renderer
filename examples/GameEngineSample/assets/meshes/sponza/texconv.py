import json
import subprocess
import os
from pathlib import Path

class TextureTranscoder:
    def __init__(self, config_path, output_dir="output"):
        self.config_path = Path(config_path)
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)

    def load_config(self):
        with open(self.config_path, "r") as file:
            self.config = json.load(file)

    def build_command(self, src, preset_name):
        preset = self.config["presets"][preset_name]
        output_file = self.output_dir / (Path(src).stem + ".ktx2")

        command = ["PVRTexToolCLI"]
        command.extend(["-i", src])
        command.extend(["-o", str(output_file)])

        # Set format
        command.extend(["-f", preset["format"]])

        # Set mipmaps generation
        if self.config.get("gen_mips", False):
            command.append("-m")
            command.extend(["-mfilter", "linear"])

        # Set quality
        quality = self.config.get("quality")
        if quality:
            command.extend(["-q", quality])

        # Set channel mask if specified
        channel_mask = preset.get("channel_mask")
        if channel_mask:
            for channel, value in channel_mask.items():
                value_formatted = value.replace("{src}", src)
                command.extend([f"-{channel}", value_formatted])

        return command

    def transcode_textures(self):
        for desc in self.config["descs"]:
            src = desc["src"]
            preset_name = desc["preset"]

            command = self.build_command(src, preset_name)

            print(f"Transcoding {src} with preset {preset_name}...")
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            if result.returncode == 0:
                print(f"Successfully transcoded {src} to KTX2 format.")
            else:
                print(f"Error transcoding {src}:")
                print(result.stderr.decode())


if __name__ == "__main__":
    import argparse

    #parser = argparse.ArgumentParser(description="Texture Transcoding Script with PVRTexToolCLI")
    #parser.add_argument("config", type=str, help="Path to JSON configuration file")
    #parser.add_argument("-o", "--output", type=str, default="output", help="Output directory")
#
    #args = parser.parse_args()

    transcoder = TextureTranscoder("texconv_cfg.json", "")
    transcoder.load_config()
    transcoder.transcode_textures()