/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cunistd>
#include <string>

using namespace std;
int main(int argc, char* argv[])
{
    cout << "argc is " << argc << endl;
    for (int i = 0; i < argc; i++) {
        cout << "argv " << i << " is " << argv[i] << endl;
    }

    cout << "begin to sleep 100s" << endl;
    fflush(stdout);
    sleep(100); // 100 : 100s
    return 0;
}