target_sources(${PROJECT_NAME} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/fs_directory_iterator.h" 
	"${CMAKE_CURRENT_SOURCE_DIR}/fs_file.h" 
	"${CMAKE_CURRENT_SOURCE_DIR}/fs_filesystem.h" 
	"${CMAKE_CURRENT_SOURCE_DIR}/fs_helper.h" 
	"${CMAKE_CURRENT_SOURCE_DIR}/fs_virtual_filesystem.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/fs_virtual_filesystem.h" 

	# native filesystem soruces
	"${CMAKE_CURRENT_SOURCE_DIR}/native/fs_native_directory_iterator.h" 
	"${CMAKE_CURRENT_SOURCE_DIR}/native/fs_native_file.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/native/fs_native_file.h" 
	"${CMAKE_CURRENT_SOURCE_DIR}/native/fs_native_filesystem.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/native/fs_native_filesystem.h" 

	# zip filesystem sources
	"${CMAKE_CURRENT_SOURCE_DIR}/zip/fs_zip_derectory_iterator.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/zip/fs_zip_derectory_iterator.h" 
	"${CMAKE_CURRENT_SOURCE_DIR}/zip/fs_zip_file.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/zip/fs_zip_file.h" 
	"${CMAKE_CURRENT_SOURCE_DIR}/zip/fs_zip_filesystem.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/zip/fs_zip_filesystem.h" 
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
	libzip::zip 
)