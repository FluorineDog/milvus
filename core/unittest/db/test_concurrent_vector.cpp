// Copyright (C) 2019-2020 Zilliz. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied. See the License for the specific language governing permissions and limitations under the License.

#include <fiu-control.h>
#include <fiu-local.h>
#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "db/SnapshotVisitor.h"
#include "db/Types.h"
#include "db/snapshot/IterateHandler.h"
#include "db/snapshot/Resources.h"
#include "db/utils.h"
#include "dog_segment/SegmentBase.h"
#include "knowhere/index/vector_index/helpers/IndexParameter.h"
#include "segment/SegmentReader.h"
#include "segment/SegmentWriter.h"
#include "src/dog_segment/SegmentBase.h"
#include "utils/Json.h"
#include "dog_segment/ConcurrentVector.h"
#include <random>
#include <vector>
#include <thread>

using std::cin;
using std::cout;
using std::endl;
using namespace milvus::engine;
using namespace milvus::dog_segment;
using std::vector;

TEST(ConcurrentVector, TestABI) {
    ASSERT_EQ(TestABI(), 42);
    assert(true);
}

TEST(ConcurrentVector, TestSingle) {
    auto dim = 8;
    ConcurrentVector<int, 32> c_vec(dim);
    std::default_random_engine e(42);
    int data = 0;
    auto total_count = 0;
    for(int i = 0; i < 10000; ++i) {
        int insert_size = e() % 150;
        vector<int> vec(insert_size * dim);
        for(auto& x: vec) {
            x = data++;
        }
        c_vec.grow_to_at_least(total_count + insert_size);
        c_vec.set_data(total_count, vec.data(), insert_size);
        total_count += insert_size;
    }
    ASSERT_EQ(c_vec.chunk_size(), (total_count + 31) / 32);
    for(int i = 0; i < total_count; ++i) {
        for(int d = 0; d < dim; ++d) {
            auto std_data = d + i * dim;
            ASSERT_EQ(c_vec.get_element(i)[d], std_data);
        }
    }
}



TEST(ConcurrentVector, TestMulti) {
    auto dim = 8;
    constexpr int threads = 16;
    std::vector<int64_t> total_counts(threads);

    ConcurrentVector<int64_t, 32> c_vec(dim);
    std::atomic<int64_t> ack_counter = 0;

    auto executor = [&](int thread_id) {
        std::default_random_engine e(42);
        int64_t data = 0;
        int64_t total_count = 0;
        for(int i = 0; i < 1000; ++i) {
            int insert_size = e() % 150;
            vector<int64_t> vec(insert_size * dim);
            for(auto& x: vec) {
                x = data++ * threads + thread_id;
            }
            auto offset = ack_counter.fetch_add(insert_size);
            c_vec.grow_to_at_least(offset + insert_size);
            c_vec.set_data(total_count, vec.data(), insert_size);
            total_count += insert_size;
        }
        total_counts[thread_id] = total_count;
    };
    std::vector<std::thread> pool;
    for(int i = 0; i < threads; ++i) {
        pool.emplace_back(executor, i);
    }
    for(auto &thread: pool) {
        thread.join();
    }

    std::vector<int64_t> counts(threads);
    auto N = ack_counter.load();
    for(int64_t i = 0; i < N; ++i) {
        for(int d = 0; d < dim; ++d) {
            auto data = c_vec.get_element(i)[d];
            auto thread_id = data % threads;
            auto raw_data =  data / threads;
            auto std_data = counts[thread_id]++;
            ASSERT_EQ(raw_data, std_data);
        }
    }

}
