/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <memory>
#include <iostream>

#include "securec.h"
#include "session.h"
#include "softbus_bus_center.h"
#include "logger.h"
#include "distributed_objectstore_manager.h"

using namespace OHOS::ObjectStore;

static int g_succTestCount = 0;
static int g_failTestCount = 0;

#define TEST_ASSERT_TRUE(ret)  \
    if (ret) {                 \
        LOG_INFO("[test][succ]\n func_[%s]:%d", __func__, __LINE__);    \
        g_succTestCount++;       \
    } else {                   \
        LOG_INFO("[test][error]\n func_[%s]:%d", __func__, __LINE__);    \
        g_failTestCount++;       \
    }


static const char *BUNDLE_NAME = "com.huawei.dosdemo1";
static const char *SESSION_ID = "test";
static char const *GROUP_ID = "";

static int TestObjectStoreManager()
{
    DistributedObjectStoreManager *ptr = DistributedObjectStoreManager::GetInstance();
    if (ptr == nullptr) {
        return 0;
    }

    DistributedObjectStore *store = ptr->Create(BUNDLE_NAME, SESSION_ID);
    TEST_ASSERT_TRUE(store != nullptr);

    DistributedObject *object = store->CreateObject("com.huawei.dosdemo1.model.Student", "objectId");
    TEST_ASSERT_TRUE(store != nullptr);

    int ret = object->PutChar("Test", 'j');
    TEST_ASSERT_TRUE(ret == 0);
    ret = object->PutShort("Test", 255);
    TEST_ASSERT_TRUE(ret == 0);
    ret = object->PutLong("Test", INT32_MAX + 1);
    TEST_ASSERT_TRUE(ret == 0);
    ret = object->PutFloat("Test", 0.01);
    TEST_ASSERT_TRUE(ret == 0);
    ret = object->PutDouble("Test", 1.234);
    TEST_ASSERT_TRUE(ret == 0);
    ret = object->PutBoolean("Test", false);
    TEST_ASSERT_TRUE(ret == 0);
    ret = object->PutString("Test", "objectTestValue");
    TEST_ASSERT_TRUE(ret == 0);

    ret = ptr->Destroy(BUNDLE_NAME, SESSION_ID);
    TEST_ASSERT_TRUE(ret == 0);
    return 0;
}



int main(void)
{
    LOG_INFO("[test]------------------------------------------------------------start");
    char input[100];
    DistributedObject *object;
    int ret;
    DistributedObjectStoreManager *ptr;
    DistributedObjectStore *store;
    while (true) {
        LOG_INFO("[test]--input command c or px or d");
        scanf_s("%s", &input, 100);
        if (strlen(input) == 0) {
            continue;
        }
        if (input[0] == 'c') {
            ptr = DistributedObjectStoreManager::GetInstance();
            if (ptr == nullptr) {
                return 0;
            }
            store = ptr->Create(BUNDLE_NAME, SESSION_ID);
            TEST_ASSERT_TRUE(store != nullptr);
            LOG_INFO("[test]-----start create object");
            object = store->CreateObject("com.huawei.dosdemo1.model.Student", "key1");
            TEST_ASSERT_TRUE(store != nullptr);
            LOG_INFO("[test]-----end create object");
        } else if (input[0] == 'p') {
            if (input[1] == 's' || input[1] == 'a') {
                LOG_INFO("[test]-----input name text");
                std::string name = "";
                std::cin >> name;
                ret = object->PutString("name", name.c_str());
                TEST_ASSERT_TRUE(ret == 0);
            }
            if (input[1] == 'i' || input[1] == 'a') {
                ret = object->PutInt("age", 20);
                TEST_ASSERT_TRUE(ret == 0);
            }
            if (input[1] == 'c' || input[1] == 'a') {
                ret = object->PutChar("testChar", 'a');
                TEST_ASSERT_TRUE(ret == 0);
            }
            if (input[1] == 'h' || input[1] == 'a') {
                ret = object->PutShort("testShort", 2);
                TEST_ASSERT_TRUE(ret == 0);
            }
            if (input[1] == 'l' || input[1] == 'a') {
                ret = object->PutLong("testLong", 1);
                TEST_ASSERT_TRUE(ret == 0);
            }
            if (input[1] == 'f' || input[1] == 'a') {
                ret = object->PutFloat("testFloat", 1.23456);
                TEST_ASSERT_TRUE(ret == 0);
            }
            if (input[1] == 'd' || input[1] == 'a') {
                ret = object->PutDouble("testDouble", 1.234);
                TEST_ASSERT_TRUE(ret == 0);
            }
            if (input[1] == 'b' || input[1] == 'a') {
                ret = object->PutBoolean("testBoolean", false);
                TEST_ASSERT_TRUE(ret == 0);
            }
        }else if (input[0] == 's') {
            ret = store->Sync(object);
            TEST_ASSERT_TRUE(ret == 0);
        } else if  (input[0] == 'd') {
            ret = ptr->Destroy(BUNDLE_NAME, SESSION_ID);
            TEST_ASSERT_TRUE(ret == 0);
        }
    }
    sleep(9000000);
    LOG_INFO("[test]------------------------------------------------------------");
    LOG_INFO("[test]test number: %d, succ = %d. fail = %d", g_failTestCount + g_succTestCount, g_succTestCount, g_failTestCount);
    LOG_INFO("[test]------------------------------------------------------------");
    return g_failTestCount;
}
