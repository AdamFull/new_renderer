target_sources(${PROJECT_NAME} PRIVATE
	"fs_zip_derectory_iterator.cpp" 
	"fs_zip_file.cpp" 
	"fs_zip_filesystem.cpp" 
)

find_package(libzip REQUIRED)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
	libzip::zip 
)