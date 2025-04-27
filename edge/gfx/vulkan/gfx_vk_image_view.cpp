#include "gfx_vk_image_view.h"
#include "gfx_vk_device.h"
#include "gfx_vk_image.h"

#include "gfx_vk_util.h"

namespace edge::gfx::vulkan {
	ImageView::ImageView(Device& device, vk::ImageViewCreateInfo create_info) :
		Resource{ nullptr, device.get_allocation_callbacks() },
		device{ device },
		create_info{ create_info } {

		vk::Result result = device.get_handle().createImageView(&create_info, allocation_callbacks, &handle_);
	}

	ImageView::ImageView(ImageView&& other) noexcept :
		Resource{ std::move(other) },
		device{ other.device },
		create_info{ other.create_info } {
	}

	ImageView::~ImageView() {
		device.get_handle().destroyImageView(handle_, allocation_callbacks);
	}
}