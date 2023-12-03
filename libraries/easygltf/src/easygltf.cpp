#include <include/easygltf/easygltf_parse.h>
#include <include/easygltf/easygltf_validator.h>
#include <include/easygltf/easygltf_helpers.h>

using namespace easygltf;

Stream FGLTFRoot::getStream(const Accessor& accessor) const
{
	Stream stream{};

	const BufferView& buffer_view = bufferViews[accessor.bufferView];
	auto& buffer = buffers[buffer_view.buffer];

	stream.stride = get_accessor_element_size(accessor);
	stream.count = accessor.count;
	stream.data = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];

	return stream;
}

Stream FGLTFRoot::getStream(int32_t accessor_id) const
{
	const Accessor& accessor = accessors[accessor_id];
	return getStream(accessor);
}