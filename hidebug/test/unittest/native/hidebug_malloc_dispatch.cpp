/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#include "hidebug/hidebug.h"
#include "hidebug/hidebug_type.h"
#include "securec.h"
#pragma clang optimize off

static void* MyMalloc(size_t size)
{
    HiDebug_MallocDispatch* original = (HiDebug_MallocDispatch*)OH_HiDebug_GetDefaultMallocDispatchTable();
    printf("test my_malloc---\n");
    return original->malloc(size);
}

static void MyFree(void* ptr)
{
    HiDebug_MallocDispatch* original = (HiDebug_MallocDispatch*)OH_HiDebug_GetDefaultMallocDispatchTable();
    printf("test my_free----\n");
    original->free(ptr);
}

static void* MyMmap(void* addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    HiDebug_MallocDispatch* original = (HiDebug_MallocDispatch*)OH_HiDebug_GetDefaultMallocDispatchTable();
    printf("test my_mmap----\n");
    return original->mmap(addr, len, prot, flags, fd, offset);
}

static int MyMunmap(void* addr, size_t len)
{
    HiDebug_MallocDispatch* original = (HiDebug_MallocDispatch*)OH_HiDebug_GetDefaultMallocDispatchTable();
    printf("test my_munmap----\n");
    return original->munmap(addr, len);
}

static void* MyCalloc(size_t nmemb, size_t size)
{
    HiDebug_MallocDispatch* original = (HiDebug_MallocDispatch*)OH_HiDebug_GetDefaultMallocDispatchTable();
    printf("test my_calloc----\n");
    return original->calloc(nmemb, size);
}

static void* MyRealloc(void* ptr, size_t size)
{
    HiDebug_MallocDispatch* original = (HiDebug_MallocDispatch*)OH_HiDebug_GetDefaultMallocDispatchTable();
    printf("test my_realloc----\n");
    return original->realloc(ptr, size);
}

static void* MyAlignedAlloc(size_t alignment, size_t size)
{
    HiDebug_MallocDispatch* original = (HiDebug_MallocDispatch*)OH_HiDebug_GetDefaultMallocDispatchTable();
    printf("test my_aligned_alloc----\n");
    return original->aligned_alloc(alignment, size);
}

HiDebug_MallocDispatch* InitCustomMalloc()
{
    HiDebug_MallocDispatch* original = (HiDebug_MallocDispatch*)OH_HiDebug_GetDefaultMallocDispatchTable();
    HiDebug_MallocDispatch* current = (HiDebug_MallocDispatch*)original->malloc(sizeof(HiDebug_MallocDispatch));
    memset_s(current, sizeof(HiDebug_MallocDispatch), 0, sizeof(HiDebug_MallocDispatch));
    current->malloc = MyMalloc;
    current->free = MyFree;
    current->mmap = MyMmap;
    current->munmap = MyMunmap;
    current->calloc = MyCalloc;
    current->realloc = MyRealloc;
    current->aligned_alloc = MyAlignedAlloc;
    OH_HiDebug_SetMallocDispatchTable(current);
    return current;
}

void DesCustomMalloc(HiDebug_MallocDispatch* current)
{
    HiDebug_MallocDispatch* original = (HiDebug_MallocDispatch*)OH_HiDebug_GetDefaultMallocDispatchTable();
    original->free(current);
    OH_HiDebug_RestoreMallocDispatchTable();
}

void TestMalloc()
{
    int* temp = (int*)malloc(sizeof(int));
    *temp = 8;
    int* temp2 = (int*)malloc(sizeof(int));
    *temp2 = 10;
    int ret = *temp2 + *temp;
    printf("ret = %d\n", ret);
    free(temp);
    free(temp2);
}
void TestMmap()
{
    char* p_map = nullptr;
    p_map = (char*)mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sprintf(p_map, "%s", "hi, this is test mmap");
    printf("p_map = %s\n", p_map);
    munmap(p_map, 100);
}

void TestCalloc()
{
    int* ptr = (int*)calloc(5, sizeof(int));
    for (size_t i = 0; i < 5; ++i) {
        ptr[i] = i * i;
        printf("ptr[%zu] = %d\n", i, ptr[i]);
    }
    free(ptr);
}

void TestRealloc()
{
    int* ptr = (int*)malloc(5 * sizeof(int));
    for (size_t i = 0; i < 5; ++i) {
        ptr[i] = i * i;
        printf("ptr[%zu] = %d\n", i, ptr[i]);
    }
    int* newPtr = (int*)realloc(ptr, 10 * sizeof(int));
    for (size_t i = 0; i < 10; ++i) {
        newPtr[i] = i * i;
        printf("newPtr[%zu] = %d\n", i, newPtr[i]);
    }
    free(newPtr);
}

void TestAlignedAlloc()
{
    size_t alignment = 16;
    size_t size = 8 * sizeof(int);
    int* ptr = (int*)aligned_alloc(alignment, size);
    for (size_t i = 0; i < size / sizeof(int); ++i) {
        ptr[i] = i * i;
        printf("ptr[%zu] = %d\n", i, ptr[i]);
    }
    free(ptr);
}

int main(int argc, char* argv[])
{
    HiDebug_MallocDispatch* current = InitCustomMalloc();
    TestMalloc();
    TestMmap();
    TestCalloc();
    TestRealloc();
    TestAlignedAlloc();
    DesCustomMalloc(current);
    return 0;
}
#pragma clang optimize on