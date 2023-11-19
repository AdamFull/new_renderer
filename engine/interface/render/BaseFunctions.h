#pragma once

#include <interface/render/BaseStructures.h>
#include <unordered_map>
#include <algorithm>

namespace engine
{
	namespace render
	{
		namespace batcher
		{
            struct BatchKey
            {
                size_t vboId{};
                size_t materialId{};
                int32_t skin{};

                bool operator==(const BatchKey& other) const
                {
                    return vboId == other.vboId && materialId == other.materialId && skin == other.skin;
                }
            };

            struct BatchKeyHash
            {
                size_t operator()(const BatchKey& key) const
                {
                    return std::hash<size_t>()(key.vboId) ^ std::hash<size_t>()(key.materialId) ^ std::hash<int32_t>()(key.skin);
                }
            };
		}

		inline std::vector<FGeometryBatch> createGeometryBatches(const std::vector<FGeometryDrawData>& geometry_list)
		{
            std::unordered_map<batcher::BatchKey, FGeometryBatch, batcher::BatchKeyHash> batch_map;

            for (const auto& data : geometry_list)
            {
                batcher::BatchKey key{ data.vboId, data.materialId, data.skin };
                auto& batch = batch_map[key];

                batch.vboId = data.vboId;
                batch.materialId = data.materialId;
                batch.skin = data.skin;
                batch.ranges.push_back(data.range);
            }

            std::vector<FGeometryBatch> batches{};
            for (auto& [key, batch] : batch_map)
            {
                std::sort(batch.ranges.begin(), batch.ranges.end(), [](const FGeometryRanges& a, const FGeometryRanges& b) { return a.beginIndex < b.beginIndex; });
            
                std::vector<FGeometryRanges> combinedRanges;
                for (const auto& range : batch.ranges)
                {
                    if (!combinedRanges.empty() && combinedRanges.back().beginIndex + combinedRanges.back().indexCount == range.beginIndex)
                        combinedRanges.back().indexCount += range.indexCount;
                    else
                        combinedRanges.push_back(range);
                }
            
                batch.ranges = std::move(combinedRanges);
                batches.emplace_back(std::move(batch));
            }

            return batches;
		}
	}
}