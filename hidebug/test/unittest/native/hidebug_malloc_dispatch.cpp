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

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <sys/mman.h>
#include <unistd.h>
#include <thread>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "hidebug/hidebug.h"
#include "hidebug/hidebug_type.h"
#include "securec.h"
#pragma clang optimize off

bool g_isInit = false;
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
    if (temp == nullptr) {
        printf("malloc failed\n");
        return;
    }
    *temp = 8;
    int* temp2 = (int*)malloc(sizeof(int));
    if (temp2 == nullptr) {
        printf("malloc failed\n");
        return;
    }
    *temp2 = 10;
    int ret = *temp2 + *temp;
    printf("ret = %d\n", ret);
    free(temp);
    free(temp2);
}
void TestMmap()
{
    char* mapPtr = nullptr;
    const size_t bufferSize = 100;  // 100 : the size of memory
    mapPtr = (char*)mmap(nullptr, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mapPtr == MAP_FAILED) {
        printf("mmap failed\n");
        return;
    }
    int len = snprintf_s(mapPtr, bufferSize, bufferSize - 1, "%s", "hi, this is test mmap");
    if (len < 0) {
        printf("snprintf_s failed\n");
    }
    printf("mapPtr = %s\n", mapPtr);
    munmap(mapPtr, bufferSize);
}

void TestCalloc()
{
    int* ptr = (int*)calloc(5, sizeof(int));  // 5 : the length of array
    if (ptr == nullptr) {
        printf("calloc failed\n");
        return;
    }
    for (size_t i = 0; i < 5; ++i) {  // 5 : the length of array
        ptr[i] = i * i;
        printf("ptr[%zu] = %d\n", i, ptr[i]);
    }
    free(ptr);
}

void TestRealloc()
{
    int* ptr = (int*)malloc(5 * sizeof(int)); // 5 : the length of array
    if (ptr == nullptr) {
        printf("malloc failed\n");
        return;
    }
    for (size_t i = 0; i < 5; ++i) { // 5 : the length of array
        ptr[i] = i * i;
        printf("ptr[%zu] = %d\n", i, ptr[i]);
    }
    int* newPtr = (int*)realloc(ptr, 10 * sizeof(int)); // 10 : the length of array
    if (newPtr == nullptr) {
        printf("realloc failed\n");
        free(ptr);
        return;
    }
    for (size_t i = 0; i < 10; ++i) { // 10 : the length of array
        newPtr[i] = i * i;
        printf("newPtr[%zu] = %d\n", i, newPtr[i]);
    }
    free(newPtr);
}

void TestMutilThread(int num)
{
    std::cout << "TestMutilThread num = " << num << ", thread_id is " << std::this_thread::get_id() << std::endl;
    TestMalloc();
    TestMmap();
    TestCalloc();
    TestRealloc();
}

class TestCustomClass {
   private:
    int temp_;

   public:
    TestCustomClass()
    {
        temp_ = 10;
        printf("TestCustomClass constructor\n");
    }
    ~TestCustomClass()
    {
        printf("TestCustomClass destructor\n");
    }
};

void TestNewFunc()
{
    int* ptr = new int(10);
    if (ptr == nullptr) {
        printf("new failed\n");
        return;
    }
    printf("ptr = %d\n", *ptr);
    delete ptr;
    ptr = nullptr;
    int* ptr2 = new int[10]; // 10 : the length of array
    if (ptr2 == nullptr) {
        printf("new failed\n");
        return;
    }
    for (size_t i = 0; i < 10; ++i) { // 10 : the length of array
        ptr2[i] = i * i;
    }
    for (size_t i = 0; i < 10; ++i) { // 10 : the length of array
        printf("ptr2[%zu] = %d\n", i, ptr2[i]);
    }
    delete[] ptr2;
    ptr2 = nullptr;
    std::string* str = new std::string("hello, world");
    if (str == nullptr) {
        printf("new failed\n");
        return;
    }
    printf("str = %s\n", str->c_str());
    delete str;
    str = nullptr;
    TestCustomClass* test = new TestCustomClass();
    if (test == nullptr) {
        printf("new failed\n");
        return;
    }
    delete test;
    test = nullptr;
    std::vector<int>* vec = new std::vector<int>(10); // 10 : the length of vector
    if (vec == nullptr) {
        printf("new failed\n");
        return;
    }
    for (size_t i = 0; i < vec->size(); ++i) {
        (*vec)[i] = i * i;
    }
    for (size_t i = 0; i < vec->size(); ++i) {
        printf("vec[%zu] = %d\n", i, (*vec)[i]);
    }
    delete vec;
    vec = nullptr;
    std::map<int, int>* map = new std::map<int, int>();
    if (map == nullptr) {
        printf("new failed\n");
        return;
    }
    for (size_t i = 0; i < 10; ++i) { // 10 : test 10 times
        (*map)[i] = i * i;
    }
    for (size_t i = 0; i < 10; ++i) { // 10 : test 10 times
        printf("map[%zu] = %d\n", i, (*map)[i]);
    }
    delete map;
    map = nullptr;
}

static void* MyNestedMalloc(size_t size)
{
    if (g_isInit) {
        HiDebug_MallocDispatch* original = (HiDebug_MallocDispatch*)OH_HiDebug_GetDefaultMallocDispatchTable();
        return original->malloc(size);
    }
    HiDebug_MallocDispatch* original = (HiDebug_MallocDispatch*)OH_HiDebug_GetDefaultMallocDispatchTable();
    printf("test MyNestedMalloc----\n");
    g_isInit = true;
    TestMalloc();
    g_isInit = false;
    return original->malloc(size);
}

HiDebug_MallocDispatch* InitNestedCustomMalloc()
{
    HiDebug_MallocDispatch* original = (HiDebug_MallocDispatch*)OH_HiDebug_GetDefaultMallocDispatchTable();
    HiDebug_MallocDispatch* current = (HiDebug_MallocDispatch*)original->malloc(sizeof(HiDebug_MallocDispatch));
    memset_s(current, sizeof(HiDebug_MallocDispatch), 0, sizeof(HiDebug_MallocDispatch));
    current->malloc = MyNestedMalloc;
    OH_HiDebug_SetMallocDispatchTable(current);
    return current;
}

int main(int argc, char* argv[])
{
    HiDebug_MallocDispatch* current = InitCustomMalloc();
    // test malloc
    TestMalloc();
    TestMmap();
    TestCalloc();
    TestRealloc();
    // test muti-threads
    std::vector<std::thread> vecThreads;
    for (size_t i = 0; i < 10; i++) { // 10 : test 10 threads
        vecThreads.push_back(std::thread(TestMutilThread, i));
    }
    for (auto& thread : vecThreads) {
        thread.join();
    }
    // test new operation
    TestNewFunc();
    DesCustomMalloc(current);

    // nested call
    HiDebug_MallocDispatch* nestedCurrent = InitNestedCustomMalloc();
    int *nestedPtr = (int*)malloc(sizeof(int));
    if (nestedPtr == nullptr) {
        printf("malloc failed\n");
        return 0;
    }
    *nestedPtr = 8;
    free(nestedPtr);
    nestedPtr = nullptr;
    DesCustomMalloc(nestedCurrent);
    return 0;
}
#pragma clang optimize on