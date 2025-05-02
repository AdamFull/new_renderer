#include "copy_buffer.h"

#define GROUP_SIZE 256

[[vk::push_constant]] ConstantBuffer<copy_buffer::PushConstant> pc;

[shader("compute")]
[numthreads(GROUP_SIZE, 1, 1)]
void main(uint32_t group_id: SV_GroupID, uint32_t local_id: SV_GroupThreadID) {
    pc.copy_dst[(group_id * 2 + 0) * GROUP_SIZE + local_id.x] = pc.copy_src[(group_id * 2 + 0) * GROUP_SIZE + local_id.x];
    pc.copy_dst[(group_id * 2 + 1) * GROUP_SIZE + local_id.x] = pc.copy_src[(group_id * 2 + 1) * GROUP_SIZE + local_id.x];
}