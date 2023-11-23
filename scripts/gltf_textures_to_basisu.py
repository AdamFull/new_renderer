from pygltflib import GLTF2, Texture, Material, Image
import os
import subprocess
import multiprocessing
import argparse

tex_format: str = "BASISU_UASTC"
tex_quality: str = "basisubest"
generate_mips: bool = True

def convert_to_ktx2(filepath: str, texture_type: str, texture_format: str, texture_quality: str, generate_mipmap: bool) -> str:
    packer_args = ["PVRTexToolCLI"]

    # add input image path
    packer_args.extend(["-i", filepath])

    outfile_path = os.path.splitext(filepath)[0] + '.ktx2'
    packer_args.extend(["-o", outfile_path])

    if os.path.exists(outfile_path):
        return outfile_path
    
    is_normal: bool = texture_type == "normalTexture"

    if generate_mipmap:
        packer_args.append("-m")

    packer_args.extend(["-j", str(multiprocessing.cpu_count())])

    packer_args.extend(["-f", texture_format])
    packer_args.extend(["-q", texture_quality])

    if is_normal:
        packer_args.extend(["-ics", "lRGB"])
    else:
        packer_args.extend(["-ics", "sRGB"])

    subprocess.Popen(packer_args).wait()
    
    return outfile_path

def set_texture_data(gltf_file: GLTF2, image_path, image: Image, texture: Texture):
    image.uri = os.path.relpath(image_path, gltf_file._path)
    texture.extensions = {'KHR_texture_basisu': {'source': texture.source}}
    texture.source = None

def process_texture(gltf_file: GLTF2, texture_info, attribute_name) -> bool:
    if texture_info is None:
        return False
    if texture_info.index is None:
        return False
    
    texture = gltf_file.textures[texture_info.index]
    if texture.source is None:
        return False
    
    image = gltf_file.images[texture.source]
    if image.uri is None:
        return False
    
    if ".ktx2" in image.uri:
        return False
    
    if 'KHR_texture_basisu' in texture.extensions:
        return False
    
    image_filepath = os.path.normpath(os.path.join(gltf_file._path, image.uri))
    out_image_path = os.path.splitext(image_filepath)[0] + '.ktx2'

    exists_original = os.path.exists(image_filepath)
    exists_output = os.path.exists(out_image_path)
    
    # Check is basisu already exists
    if not exists_original:
        if exists_output:
            set_texture_data(gltf_file, out_image_path, image, texture)
            print(f"Adding existing file {out_image_path}")
            return True
        return False
    
    basisu_image_path = convert_to_ktx2(image_filepath, attribute_name, tex_format, tex_quality, generate_mips)
    if basisu_image_path is None:
        return False
    
    set_texture_data(gltf_file, basisu_image_path, image, texture)
    
    #if exists_original:
    #    os.remove(image_filepath)
    
    return True

def process_material(gltf_file: GLTF2, material: Material):
    for attribute_name in ['baseColorTexture', 'metallicRoughnessTexture', 'normalTexture', 'occlusionTexture', 'emissiveTexture']:
        texture_info = None
        if hasattr(material.pbrMetallicRoughness, attribute_name):
            texture_info = getattr(material.pbrMetallicRoughness, attribute_name)
        elif hasattr(material, attribute_name):
            texture_info = getattr(material, attribute_name)

        process_texture(gltf_file, texture_info, attribute_name)

def main():
    parser = argparse.ArgumentParser(description='Convert glTF textures to KTX2 format.')
    parser.add_argument('-i', metavar='-i', type=str, help='the path to the glTF file')
    args = parser.parse_args()

    gltf_file = GLTF2.load(args.i)
    for material in gltf_file.materials:
        process_material(gltf_file, material)

    if 'KHR_texture_basisu' not in gltf_file.extensionsUsed:
        gltf_file.extensionsUsed.append('KHR_texture_basisu')

    #if 'KHR_texture_basisu' not in gltf_file.extensionsRequired:
    #    gltf_file.extensionsRequired.append('KHR_texture_basisu')

    gltf_file.save(args.i)

if __name__ == "__main__":
    main()