import subprocess, os, sys
from argparse import ArgumentParser

#vcpkg x-update-baseline --add-initial-baseline
# DVCPKG_TARGET_TRIPLET
# Command line: "C:\WINDOWS\system32\cmd.exe" /c "%SYSTEMROOT%\System32\chcp.com 65001 >NUL && "D:\PROGRAM FILES\MICROSOFT VISUAL STUDIO\2022\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\CMake\bin\cmake.exe"  -G "Ninja"  -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_INSTALL_PREFIX:PATH="D:\GitHub\new_renderer\out\install\x64-Debug" -DCMAKE_C_COMPILER:FILEPATH="D:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.37.32822/bin/Hostx64/x64/cl.exe" -DCMAKE_CXX_COMPILER:FILEPATH="D:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.37.32822/bin/Hostx64/x64/cl.exe"   -DCMAKE_MAKE_PROGRAM="D:\PROGRAM FILES\MICROSOFT VISUAL STUDIO\2022\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\Ninja\ninja.exe" -DCMAKE_TOOLCHAIN_FILE="D:/Program Files/vcpkg/scripts/buildsystems/vcpkg.cmake" "D:\GitHub\new_renderer" 2>&1"

def locate_program(program : str) -> str:
    try:
        command = "where" if os.name == "nt" else "which"
        result = subprocess.check_output([command, program])
        return result.decode('utf-8').strip()
    except subprocess.CalledProcessError:
        print(f"{program} was not located.")
        return None

def is_program_installed(program : str) -> bool:
    return locate_program(program) is not None

def get_vcpkg_triplet(platform_str : str) -> str:
    if "win" in platform_str:
        if "32" in platform_str:
            return "x86-windows"
        else:
            return "x64-windows"
        
    return "unknown"

def get_output_dir_path(build_type : str, platform : str):
    if "32" in platform:
        return f"x86-{build_type}"
    else:
        return f"x64-{build_type}"


def get_installed_libraries(platform : str):
    result = subprocess.run(['vcpkg', 'list', platform], stdout=subprocess.PIPE, text=True)
    return [line.split(':')[0] for line in result.stdout.split('\n') if line]

def get_missing_libraries(platform : str, libraries : list):
    installed_libraries = get_installed_libraries(platform)
    return [lib for lib in libraries if lib not in installed_libraries]

class EngineCompiler:
    def __init__(self, platform : str, build_type : str):
        self.platform = platform
        self.build_type = build_type

    def execute(self):
        vcpkg_root = os.path.dirname(locate_program("vcpkg"))
        vcpkg_toolchain_file_path = os.path.normpath(os.path.join(vcpkg_root, "scripts/buildsystems/vcpkg.cmake"))
        vcpkg_triplet = get_vcpkg_triplet(self.platform)

        output_dir = f"./out/{get_output_dir_path(self.build_type, self.platform)}"
        #os.makedirs(output_dir)

        cmake_command = [
            "cmake", f"./",
            "-B", output_dir,
            "-G Visual Studio 17 2022",
            "-A", "Win64",
            f'-DVCPKG_INSTALLED_DIR={os.path.join(output_dir, "vcpkg_installed")}'
            f"-DCMAKE_BUILD_TYPE:STRING={self.build_type}",
            f"-DVCPKG_TARGET_TRIPLET={vcpkg_triplet}",
            f"-DCMAKE_TOOLCHAIN_FILE={vcpkg_toolchain_file_path}"
        ]

        result = subprocess.run(cmake_command, stdout=subprocess.PIPE, text=True)

        #print(installed_libraries)

def main():
    parser = ArgumentParser(description='Shader compiler script.')
    parser.add_argument('-platform', type=str, action='store', default='win64', help='Compile platform type.')
    parser.add_argument('-build_type', type=str, action='store', default='Debug', help='Project build type.')
    args = parser.parse_args()

    if not is_program_installed("vcpkg"):
        print("vcpkg is not installed. You need to install it by yourself.")
        sys.exit(1)

    if not is_program_installed("cmake"):
        print("cmake is not installed. You need to install it by yourself.")
        sys.exit(1)

    os.chdir("../")
    compiler = EngineCompiler(args.platform, args.build_type)
    compiler.execute()

if __name__ == "__main__":
    main()