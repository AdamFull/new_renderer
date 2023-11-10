@echo off

set workdir_path=%cd%\..
set binary_directory_path=%workdir_path%\bin\Debug
set shader_compiler_path=%binary_directory_path%\tools\shader_compiler\shader_compiler
set engine_binaries_path=%binary_directory_path%\engine

python prepare_engine_resources.py -build_type Debug
pause