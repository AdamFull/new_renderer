#pragma once

#include "BaseStructures.h"

namespace engine
{
	namespace render
	{
        //enum EGeometryBatcherFlagBits
        //{
        //    GEOMETRY_BATCHER_COUNT_ON_MATERIAL_FLAG_BIT = 1 << 0,
        //    GEOMETRY_BATCHER_MERGE_DRAWCALLS_FLAG_BIT = 1 << 1,
        //    GEOMETRY_BATCHER_ALL_FLAG_BIT = 
        //                                    GEOMETRY_BATCHER_COUNT_ON_MATERIAL_FLAG_BIT | 
        //                                    GEOMETRY_BATCHER_MERGE_DRAWCALLS_FLAG_BIT
        //};
        //
		//namespace batcher
		//{
        //    struct BatchKey
        //    {
        //        u32 flags{};
        //        u64 vboId{};
        //        u64 materialId{};
        //        u32 node{};
        //
        //        bool operator==(const BatchKey& other) const
        //        {
        //            bool is_same = vboId == other.vboId && node == other.node;
        //            return is_same && ((flags & GEOMETRY_BATCHER_COUNT_ON_MATERIAL_FLAG_BIT) ? materialId == other.materialId : true);
        //        }
        //    };
        //
        //    struct BatchKeyHash
        //    {
        //        u64 operator()(const BatchKey& key) const
        //        {
        //            u64 hash = std::hash<u64>()(key.vboId) ^ std::hash<u32>()(key.node);
        //
        //            if (key.flags & GEOMETRY_BATCHER_COUNT_ON_MATERIAL_FLAG_BIT)
        //                hash ^= std::hash<u64>()(key.materialId);
        //
        //            return hash;
        //        }
        //    };
		//}
        //
        //inline void rebatchGeometry(const utl::vector<FGeometryBatch>& batches, utl::vector<FGeometryBatch>& rebatched, u32 flags = GEOMETRY_BATCHER_ALL_FLAG_BIT)
        //{
        //    std::unordered_map<batcher::BatchKey, FGeometryBatch, batcher::BatchKeyHash> batch_map;
        //
        //    for (const auto& batch : batches)
        //    {
        //        batcher::BatchKey key{ flags, batch.vboID, batch.materialID, batch.nodeID };
        //        auto& nbatch = batch_map[key];
        //
        //        nbatch.vboID = batch.vboID;
        //        nbatch.materialID = (flags & GEOMETRY_BATCHER_COUNT_ON_MATERIAL_FLAG_BIT) ? batch.materialID : ~0ull;
        //        nbatch.nodeID = batch.nodeID;
        //
        //        for(const auto& range : batch.ranges)
        //            nbatch.ranges.emplace_back(range);
        //    }
        //
        //    // Apply geometry batches
        //    for (auto& [key, batch] : batch_map)
        //    {
        //        if (flags & GEOMETRY_BATCHER_MERGE_DRAWCALLS_FLAG_BIT)
        //        {
        //            std::sort(batch.ranges.begin(), batch.ranges.end(), [](const FGeometryRanges& a, const FGeometryRanges& b) { return a.beginIndex < b.beginIndex; });
        //
        //            utl::vector<FGeometryRanges> combinedRanges;
        //            for (const auto& range : batch.ranges)
        //            {
        //                if (!combinedRanges.empty())
        //                {
        //                    auto& lastRange = combinedRanges.back();
        //                    if (lastRange.beginIndex + lastRange.indexCount == range.beginIndex)
        //                        lastRange.indexCount += range.indexCount;
        //                }
        //                else
        //                    combinedRanges.push_back(range);
        //            }
        //
        //            batch.ranges = std::move(combinedRanges);
        //        }
        //
        //        rebatched.emplace_back(std::move(batch));
        //    }
        //}
        //
		//inline void createGeometryBatches(const utl::vector<FGeometryDrawData>& geometry_list, utl::vector<FGeometryBatch>& batches, u32 flags = GEOMETRY_BATCHER_ALL_FLAG_BIT)
		//{
        //    std::unordered_map<batcher::BatchKey, FGeometryBatch, batcher::BatchKeyHash> batch_map;
        //
        //    for (const auto& data : geometry_list)
        //    {
        //        batcher::BatchKey key{ flags, data.vboID, data.materialID, data.nodeID};
        //        auto& batch = batch_map[key];
        //
        //        batch.vboID = data.vboID;
        //        batch.materialID = (flags & GEOMETRY_BATCHER_COUNT_ON_MATERIAL_FLAG_BIT) ? data.materialID : ~0ull;
        //        batch.nodeID = data.nodeID;
        //        batch.ranges.emplace_back(data.range);
        //    }
        //
        //    // Apply geometry batches
        //    utl::vector<FGeometryBatch> uni_batches{};
        //    for (auto& [key, batch] : batch_map)
        //    {
        //        if (flags & GEOMETRY_BATCHER_MERGE_DRAWCALLS_FLAG_BIT)
        //        {
        //            std::sort(batch.ranges.begin(), batch.ranges.end(), [](const FGeometryRanges& a, const FGeometryRanges& b) { return a.beginIndex < b.beginIndex; });
        //        
        //            utl::vector<FGeometryRanges> combinedRanges;
        //            for (const auto& range : batch.ranges)
        //            {
        //                if (!combinedRanges.empty())
        //                {
        //                    auto& lastRange = combinedRanges.back();
        //                    if(lastRange.beginIndex + lastRange.indexCount == range.beginIndex)
        //                        lastRange.indexCount += range.indexCount;
        //                }
        //                else
        //                    combinedRanges.push_back(range);
        //            }
        //        
        //            batch.ranges = std::move(combinedRanges);
        //        }
        //        
        //        uni_batches.emplace_back(std::move(batch));
        //    }
        //
        //    // Sort by vbo and material
        //    std::sort(uni_batches.begin(), uni_batches.end(), [](const FGeometryBatch& a, const FGeometryBatch& b)
        //        { 
        //            if (a.vboID == b.vboID)
        //                return a.materialID < b.materialID;
        //
        //            return a.vboID < b.vboID;
        //        });
        //
        //    if (flags & GEOMETRY_BATCHER_COUNT_ON_MATERIAL_FLAG_BIT)
        //    {
        //        u64 currentMaterial{ ~0ull };
        //        for (auto& uni_batch : uni_batches)
        //        {
        //            if (!batches.empty() &&
        //                batches.back().vboID == uni_batch.vboID &&
        //                batches.back().nodeID == uni_batch.nodeID &&
        //                currentMaterial + 1 == uni_batch.materialID)
        //            {
        //                batches.back().ranges.insert(batches.back().ranges.end(), uni_batch.ranges.begin(), uni_batch.ranges.end());
        //                currentMaterial = uni_batch.materialID;
        //            }
        //            else
        //            {
        //                batches.emplace_back(std::move(uni_batch));
        //                batches.back().materialID = uni_batch.materialID;
        //                currentMaterial = uni_batch.materialID;
        //            }
        //        }
        //    }
        //    else
        //    {
        //        batches = std::move(uni_batches);
        //    }
		//}
	}
}