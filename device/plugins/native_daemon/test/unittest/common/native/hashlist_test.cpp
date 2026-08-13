/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#include "hashlist_test.h"

#include <string>

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
class HashListTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    const std::size_t size_ {20};
};

void HashListTest::SetUpTestCase(void) {}
void HashListTest::TearDownTestCase(void) {}
void HashListTest::SetUp() {}
void HashListTest::TearDown() {}

HWTEST_F(HashListTest, size, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    ASSERT_TRUE(hashList.empty());
    ASSERT_FALSE(hashList.IsFull());
    ASSERT_EQ(hashList.capacity(), size_);
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
        EXPECT_EQ(hashList.size(), curSize + 1);
    }
}

HWTEST_F(HashListTest, empty, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    EXPECT_TRUE(hashList.empty());
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
        EXPECT_FALSE(hashList.empty());
    }
}

HWTEST_F(HashListTest, count, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    std::size_t temp {125};
    EXPECT_EQ(hashList.count(temp), 0u);
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
        EXPECT_EQ(hashList.count(curSize), 1u);
    }
}

HWTEST_F(HashListTest, begin, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = 0; count < size_; ++count) {
        auto first = hashList.begin();
        EXPECT_EQ(*first, count);
        hashList.pop_front();
    }
}

HWTEST_F(HashListTest, cbegin, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = 0; count < size_; ++count) {
        const auto first = hashList.cbegin();
        EXPECT_EQ(*first, count);
        hashList.pop_front();
    }
}

HWTEST_F(HashListTest, end, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = size_ - 1; count > 0; --count) {
        auto last = hashList.end();
        last--;
        EXPECT_EQ(*last, count);
        hashList.pop_back();
    }
}

HWTEST_F(HashListTest, cend, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = size_ - 1; count > 0; --count) {
        const auto last = hashList.cend();
        auto temp = last;
        temp--;
        EXPECT_EQ(*temp, count);
        hashList.pop_back();
    }
}

HWTEST_F(HashListTest, rbegin, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = size_ - 1; count > 0; --count) {
        auto last = hashList.rbegin();
        EXPECT_EQ(*last, count);
        hashList.pop_back();
    }
}

HWTEST_F(HashListTest, crbegin, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = size_ - 1; count > 0; --count) {
        const auto last = hashList.crbegin();
        EXPECT_EQ(*last, count);
        hashList.pop_back();
    }
}

HWTEST_F(HashListTest, rend, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = 0; count < size_; ++count) {
        auto first = hashList.rend();
        --first;
        EXPECT_EQ(*first, count);
        hashList.pop_front();
    }
}

HWTEST_F(HashListTest, crend, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = 0; count < size_; ++count) {
        const auto first = hashList.rend();
        auto temp = first;
        --temp;
        EXPECT_EQ(*temp, count);
        hashList.pop_front();
    }
}

HWTEST_F(HashListTest, front, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = 0; count < size_; ++count) {
        auto first1 = hashList.front();
        const auto first2 = hashList.front();
        EXPECT_EQ(first1, count);
        EXPECT_EQ(first2, count);
        hashList.pop_front();
    }
}

HWTEST_F(HashListTest, back, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = size_ - 1; count > 0; --count) {
        auto last1 = hashList.back(true);
        auto last2 = hashList.front();
        EXPECT_EQ(last1, count);
        EXPECT_EQ(last2, count);
    }
}

HWTEST_F(HashListTest, subscription, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = 0; count < size_; ++count) {
        auto val1 = hashList[count];
        auto val2 = hashList.front();
        EXPECT_EQ(val1, val2);
    }
}

HWTEST_F(HashListTest, find, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = 0; count < (size_ + size_); ++count) {
        auto itr = hashList.find(count);
        if (count < size_) {
            EXPECT_EQ(*itr, count);
        } else {
            EXPECT_TRUE(itr == hashList.end());
        }
    }
}

HWTEST_F(HashListTest, push_front, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    constexpr std::size_t size_ {20};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_front(curSize, curSize);
        std::size_t tmp = hashList.front();
        EXPECT_EQ(tmp, curSize);
    }
}

HWTEST_F(HashListTest, push_back, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    constexpr std::size_t size_ {20};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
        std::size_t tmp = hashList.back();
        EXPECT_EQ(tmp, curSize);
    }
}

HWTEST_F(HashListTest, pop_front, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = 0; count < size_; ++count) {
        std::size_t tmp = hashList.front();
        EXPECT_EQ(tmp, count);
        EXPECT_EQ(hashList.size(), size_ - count);
        hashList.pop_front();
    }
    EXPECT_TRUE(hashList.empty());
}

HWTEST_F(HashListTest, pop_back, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_front(curSize, curSize);
    }
    for (std::size_t count = 0; count < size_; ++count) {
        std::size_t tmp = hashList.back();
        EXPECT_EQ(tmp, count);
        EXPECT_EQ(static_cast<std::size_t>(hashList.size()), size_ - count);
        hashList.pop_back();
    }
    EXPECT_TRUE(hashList.empty());
}

HWTEST_F(HashListTest, erase, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {size_};
    for (std::size_t curSize = 0; curSize < size_; ++curSize) {
        hashList.push_back(curSize, curSize);
    }
    for (std::size_t count = 0; count < size_; ++count) {
        std::size_t tmp = hashList.front();
        EXPECT_EQ(tmp, count);
        EXPECT_EQ(static_cast<std::size_t>(hashList.size()), size_ - count);
        hashList.erase(count);
    }
    EXPECT_TRUE(hashList.empty());
}

HWTEST_F(HashListTest, IteratorComparisonAndBoundary, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> first {3};
    HashList<std::size_t, std::size_t> second {3};
    auto emptyIt = first.begin();
    EXPECT_TRUE(emptyIt == first.end());
    ++emptyIt;
    EXPECT_FALSE(emptyIt == first.end());
    emptyIt--;
    EXPECT_TRUE(emptyIt.GetNode() == nullptr);

    first.push_back(1, 10);
    first.push_back(2, 20);
    second.push_back(1, 10);
    auto head = first.begin();
    auto next = head;
    next++;
    EXPECT_TRUE(head < next);
    EXPECT_FALSE(next < head);
    EXPECT_FALSE(head < second.begin());
    EXPECT_FALSE((HashList<std::size_t, std::size_t>::Iterator() < head));
    EXPECT_FALSE((HashList<std::size_t, std::size_t>::Iterator() == head));
    EXPECT_EQ(*head, 10u);
}

HWTEST_F(HashListTest, ReverseIteratorComparisonAndBoundary, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> first {3};
    HashList<std::size_t, std::size_t> second {3};
    auto emptyIt = first.rbegin();
    EXPECT_TRUE(emptyIt == first.rend());
    ++emptyIt;
    EXPECT_FALSE(emptyIt == first.rend());
    emptyIt--;
    EXPECT_TRUE(emptyIt.GetNode() == nullptr);

    first.push_back(1, 10);
    first.push_back(2, 20);
    second.push_back(1, 10);
    auto tail = first.rbegin();
    auto prev = tail;
    prev++;
    EXPECT_TRUE(tail < prev);
    EXPECT_FALSE(prev < tail);
    EXPECT_FALSE(tail < second.rbegin());
    EXPECT_FALSE((HashList<std::size_t, std::size_t>::ReverseIterator() < tail));
    EXPECT_FALSE((HashList<std::size_t, std::size_t>::ReverseIterator() == tail));
    EXPECT_EQ(*tail, 20u);
}

HWTEST_F(HashListTest, DuplicateKeysMoveNodesAndUpdateValues, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {2};
    hashList.push_back(1, 10);
    hashList.push_back(2, 20);
    EXPECT_TRUE(hashList.IsFull());

    hashList.push_back(3, 30);
    EXPECT_EQ(hashList.count(2), 0u);
    EXPECT_EQ(hashList.back(), 30u);

    hashList.push_front(1, 11);
    EXPECT_EQ(hashList.front(), 11u);
    hashList.push_back(1, 12);
    EXPECT_EQ(hashList.front(), 12u);
    EXPECT_EQ(hashList.back(), 30u);

    std::size_t movedValue = 40;
    hashList.push_front(4, std::move(movedValue));
    EXPECT_EQ(hashList.front(), 40u);
    hashList.push_back(4, static_cast<std::size_t>(41));
    EXPECT_EQ(hashList.front(), 41u);
    EXPECT_EQ(hashList.back(), 12u);
}

HWTEST_F(HashListTest, ReservePopAndEraseEdges, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> emptyList {1};
    emptyList.pop_front();
    emptyList.pop_back();
    EXPECT_TRUE(emptyList.erase(99) == emptyList.end());

    HashList<std::size_t, std::size_t> hashList;
    EXPECT_EQ(hashList.reserve(0), 0);
    EXPECT_EQ(hashList.reserve(4), 4);
    EXPECT_EQ(hashList.reserve(2), -1);
    hashList.push_back(1, 10);
    hashList.push_back(2, 20);
    hashList.push_back(3, 30);
    auto eraseRet = hashList.erase(hashList.find(2));
    EXPECT_FALSE(eraseRet == hashList.end());
    EXPECT_EQ(*eraseRet, 30u);
    EXPECT_EQ(hashList.size(), 2u);
}

HWTEST_F(HashListTest, IteratorNullAndSingleElementBoundaries, TestSize.Level0)
{
    using List = HashList<std::size_t, std::size_t>;
    using Node = LinkNode<std::size_t, std::size_t>;
    EXPECT_EQ(Node::GetLinkNode(static_cast<std::size_t*>(nullptr)), nullptr);
    EXPECT_EQ(Node::GetLinkNode(static_cast<Link*>(nullptr)), nullptr);

    List::Iterator nullIterator(static_cast<Node*>(nullptr), static_cast<List*>(nullptr));
    EXPECT_EQ(nullIterator.GetNode(), nullptr);
    const Node* nullNode = nullptr;
    const List* nullList = nullptr;
    List::Iterator constNullIterator(nullNode, nullList);
    EXPECT_EQ(constNullIterator.GetNode(), nullptr);

    HashList<std::size_t, std::size_t> hashList {2};
    EXPECT_TRUE(hashList.cbegin() == hashList.cend());
    EXPECT_TRUE(hashList.crbegin() == hashList.crend());

    hashList.push_back(1, 10);
    auto forward = hashList.begin();
    EXPECT_EQ(*forward, 10u);
    ++forward;
    EXPECT_EQ(forward.GetNode(), nullptr);
    forward++;
    EXPECT_EQ(forward.GetNode(), nullptr);

    auto reverse = hashList.rbegin();
    EXPECT_EQ(*reverse, 10u);
    ++reverse;
    EXPECT_EQ(reverse.GetNode(), nullptr);
    reverse++;
    EXPECT_EQ(reverse.GetNode(), nullptr);
}

HWTEST_F(HashListTest, ReverseIteratorRendDecrementBoundaries, TestSize.Level0)
{
    HashList<std::size_t, std::size_t> hashList {2};
    hashList.push_back(1, 10);
    hashList.push_back(2, 20);

    auto fromRend = hashList.rend();
    --fromRend;
    ASSERT_NE(fromRend.GetNode(), nullptr);
    EXPECT_EQ(*fromRend, 10u);

    auto post = hashList.rend();
    auto old = post--;
    EXPECT_EQ(old.GetNode(), nullptr);
    ASSERT_NE(post.GetNode(), nullptr);
    EXPECT_EQ(*post, 10u);

    auto beforeRbegin = hashList.rbegin();
    --beforeRbegin;
    EXPECT_EQ(beforeRbegin.GetNode(), nullptr);
}
} // end of namespace NativeDaemon
} // end of namespace Developtools
} // end of namespace OHOS
