/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include "hidebug/hidebug.h"

// Global flag to control thread execution
std::atomic<bool> g_running(true);

// Child thread function: calls the collection interface
void CollectorThread()
{
    std::cout << "Collector thread started" << std::endl;
    OH_HiDebug_ResourceType profilerType = OH_HiDebug_ResourceType::OH_RES_TYPE_NATIVE;
    OH_HiDebug_ResProfilerConfig config = {};
    uint32_t fileSize = 256; // 256 bytes
    uint32_t maxDuration = 3600; // 1 hour
    uint32_t maxStackDepth = 30; // 30 frames
    uint32_t sampleInterval = 384; // 384 Bytes
    uint32_t statisticsInterval = 10; // 10 seconds
    const int thirty = 30;

    config.filterSize = fileSize;
    config.maxDuration = maxDuration;
    config.maxStackDepth = maxStackDepth;
    config.sampleInterval = sampleInterval;
    config.statisticsInterval = statisticsInterval;

    OH_HiDebug_StartProfiler(profilerType, &config);
    std::cout << "Collector thread: collecting data..." << std::endl;
        
    std::this_thread::sleep_for(std::chrono::seconds(thirty));
    OH_HiDebug_StopProfiler();
    std::cout << "Collector thread stopped" << std::endl;
}

// Main thread function: continuously allocates and frees memory
void MemoryAllocationThread()
{
    std::cout << "Main thread: starting memory allocation test" << std::endl;
    
    const int allocationSize = 1024 * 1024; // 1MB
    const int maxAllocations = 100;
    const int thirty = 30;
    const int oneHundred = 100;
    const int twoHundredFiftySix = 256;
    std::vector<char*> memoryBlocks;
    
    // Run for 30 seconds
    auto startTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::seconds(thirty);
    
    while (std::chrono::steady_clock::now() - startTime < duration) {
        // Randomly allocate memory
        if (memoryBlocks.size() < maxAllocations) {
            char* block = new char[allocationSize];
            // Fill data to ensure memory is actually used
            for (int i = 0; i < allocationSize; i++) {
                block[i] = static_cast<char>(i % twoHundredFiftySix);
            }
            memoryBlocks.push_back(block);
            std::cout << "Allocated block " << memoryBlocks.size() << " of size "
                << allocationSize << " bytes" << std::endl;
        }
        
        // Brief sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(oneHundred));
    }
    
    // Free all remaining memory
    for (auto block : memoryBlocks) {
        delete[] block;
    }
    memoryBlocks.clear();
    
    std::cout << "Main thread: memory allocation test completed" << std::endl;
    g_running = false;
}

int main()
{
    std::cout << "Starting memory profiling test..." << std::endl;
    
    // Start child thread to call the collection interface
    std::thread collector(CollectorThread);
    
    // Main thread performs memory allocation and deallocation
    MemoryAllocationThread();
    
    // Wait for child thread to finish
    if (collector.joinable()) {
        collector.join();
    }
    
    std::cout << "Memory profiling test completed." << std::endl;
    return 0;
}
