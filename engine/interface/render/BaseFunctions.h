#pragma once

#include "BaseStructures.h"
#include <unordered_map>
#include <algorithm>

namespace engine
{
	namespace render
	{
        enum EGeometryBatcherFlagBits
        {
            GEOMETRY_BATCHER_COUNT_ON_MATERIAL_FLAG_BIT = 1 << 0,
            GEOMETRY_BATCHER_MERGE_DRAWCALLS_FLAG_BIT = 1 << 1,
            GEOMETRY_BATCHER_ALL_FLAG_BIT = 
                                            GEOMETRY_BATCHER_COUNT_ON_MATERIAL_FLAG_BIT | 
                                            GEOMETRY_BATCHER_MERGE_DRAWCALLS_FLAG_BIT
        };

		namespace batcher
		{
            struct BatchKey
            {
                uint32_t flags{};
                size_t vboId{};
                size_t materialId{};
                uint32_t node{};
                int32_t skin{};

                bool operator==(const BatchKey& other) const
                {
                    bool is_same = vboId == other.vboId && node == other.node && skin == other.skin;
                    return is_same && ((flags & GEOMETRY_BATCHER_COUNT_ON_MATERIAL_FLAG_BIT) ? materialId == other.materialId : true);
                }
            };

            struct BatchKeyHash
            {
                size_t operator()(const BatchKey& key) const
                {
                    size_t hash = std::hash<size_t>()(key.vboId) ^ std::hash<uint32_t>()(key.node) ^ std::hash<int32_t>()(key.skin);

                    if (key.flags & GEOMETRY_BATCHER_COUNT_ON_MATERIAL_FLAG_BIT)
                        hash ^= std::hash<size_t>()(key.materialId);

                    return hash;
                }
            };
		}

		inline void createGeometryBatches(const std::vector<FGeometryDrawData>& geometry_list, std::vector<FGeometryBatch>& batches, uint32_t flags = GEOMETRY_BATCHER_ALL_FLAG_BIT)
		{
            std::unordered_map<batcher::BatchKey, FGeometryBatch, batcher::BatchKeyHash> batch_map;

            for (const auto& data : geometry_list)
            {
                batcher::BatchKey key{ flags, data.vboId, data.materialId, data.node, data.skin };
                auto& batch = batch_map[key];

                batch.vboId = data.vboId;
                batch.materialId = (flags & GEOMETRY_BATCHER_COUNT_ON_MATERIAL_FLAG_BIT) ? data.materialId : ~0ull;
                batch.node = data.node;
                batch.skin = data.skin;
                batch.ranges.emplace_back(data.range);
            }

            // Apply geometry batches
            for (auto& [key, batch] : batch_map)
            {
                if (flags & GEOMETRY_BATCHER_MERGE_DRAWCALLS_FLAG_BIT)
                {
                    std::sort(batch.ranges.begin(), batch.ranges.end(), [](const FGeometryRanges& a, const FGeometryRanges& b) { return a.beginIndex < b.beginIndex; });
                
                    std::vector<FGeometryRanges> combinedRanges;
                    for (const auto& range : batch.ranges)
                    {
                        if (!combinedRanges.empty())
                        {
                            auto& lastRange = combinedRanges.back();
                            if(lastRange.beginIndex + lastRange.indexCount == range.beginIndex)
                                lastRange.indexCount += range.indexCount;
                        }
                        else
                            combinedRanges.push_back(range);
                    }
                
                    batch.ranges = std::move(combinedRanges);
                }
                
                batches.emplace_back(std::move(batch));
            }
		}
	}
}