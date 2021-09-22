

# 分布式对象用户手册（小型设备）

分布式数据对象管理框架是一款面向对象的内存数据管理框架，向应用开发者提供内存对象的创建、查询、删除、修改、订阅等基本数据对象的管理能力，同时具备分布式能力，满足超级终端场景下，相同应用多设备间的数据对象协同需求。 

## 基本概念

分布式数据对象管理框架的两个主要组件： 

 •     数据对象：数据对象模型类的实例，用于存储应用程序的运行时数据。

 •     数据对象仓库：数据对象的管理类，通过数据对象仓库的接口完成数据对象的插入、查找、删除、订阅等操作。 

## 约束与限制

•	小型设备支持和大型设备连接，对象同步链路暂时为单向，支持小型设备向大型设备同步分布式对象。

| 类型名称    | 类型描述                                  |
| ----------- | ----------------------------------------- |
| int32_t     | 32位整型，对应大型设备int类型             |
| int16_t     | 16位整型，对应大型设备short类型           |
| int64_t     | 封装长整型                                |
| char        | 字符型，对应大型设备char类型              |
| float       | 单精度浮点型，对应大型设备float类型       |
| double      | 双精度浮点型，对应大型设备double类型      |
| boolean     | 布尔型，对应大型设备boolean类型           |
| std::string | 字符串型,对应大型设备java.lang.String类型 |
| int8_t      | 8位整型，对应大型设备byte类型             |
## 开发指导
### 接口说明
•	对象仓库管理
distributed_objectstore_manager.h中OHOS::ObjectStore::DistributedObjectStoreManager用于创建，销毁分布式对象仓库。

| 接口名称                                                     | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| static DistributedObjectStoreManager* GetInstance()          | 获取DistributedObjectStoreManager实例                        |
| DistributedObjectStore *Create(const std::string &bundleName, const std::string &sessionId) | 创建分布式对象仓库<br>bundleName和大型设备hap包的bundleName一致<br>sessionId和大型设备构建分布式对象仓库时的sessionId一致 |
| uint32_t Destroy(const std::string &bundleName, const std::string &sessionId) | 销毁分布式对象仓库<br>bundleName和大型设备hap包的bundleName一致<br>sessionId和大型设备构建分布式对象仓库时的sessionId一致 |

•	对象仓库
distributed_objectstore.h中OHOS::ObjectStore::DistributedObjectStore用于创建，同步分布式对象仓库。

| 接口名称                                                     | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| DistributedObject *CreateObject(const std::string &classPath, const std::string &key) | 创建分布式对象<br>classPath和大型设备创建对象时clazz的路径相同，如com.testobject.dosdemo1.model.Student<br>key和大型设备创建分布式对象时key相同，同一个class中可以标识一个对象（一个class可以创建多个对象） |
| uint32_t Sync(DistributedObject *object)                     | 同步分布式对象                                               |
| void Close()                                                 | 关闭仓库，无需调用，在DistributedObjectStoreManager.destroy时自动调用 |

•	对象

distributed_object.h中OHOS::ObjectStore::DistributedObject用于管理对象属性。

| 接口名称                                                     | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| uint32_t PutChar(const std::string &key, char value)         | 增加或更新类型为char的属性<br>key 标识属性名，和大型设备测用@Field标识的对象名对应<br>value是刷新的内容 |
| uint32_t PutInt(const std::string &key, int32_t value)       | 增加或更新类型为int32_t的属性<br>key 标识属性名，和大型设备测用@Field标识的对象名对应<br>value是刷新的内容 |
| uint32_t PutShort(const std::string &key, int16_t value)     | 增加或更新类型为int16_t的属性<br>key 标识属性名，和大型设备测用@Field标识的对象名对应<br>value是刷新的内容 |
| uint32_t PutLong(const std::string &key, int64_t value)      | 增加或更新类型为int64_t的属性<br>key 标识属性名，和大型设备测用@Field标识的对象名对应<br>value是刷新的内容 |
| uint32_t PutFloat(const std::string &key, float value)       | 增加或更新类型为float的属性<br>key 标识属性名，和大型设备测用@Field标识的对象名对应<br>value是刷新的内容 |
| uint32_t PutDouble(const std::string &key, double value)     | 增加或更新类型为double的属性<br>key 标识属性名，和大型设备测用@Field标识的对象名对应<br>value是刷新的内容 |
| uint32_t PutBoolean(const std::string &key, bool value)      | 增加或更新类型为bool的属性<br>key 标识属性名，和大型设备测用@Field标识的对象名对应<br>value是刷新的内容 |
| uint32_t PutString(const std::string &key, const std::string &value) | 增加或更新类型为std::string的属性<br>key 标识属性名，和大型设备测用@Field标识的对象名对应<br>value是刷新的内容 |
| uint32_t PutByte(const std::string &key, const int8_t &value) | 增加或更新类型为int8_t的属性<br>key 标识属性名，和大型设备测用@Field标识的对象名对应<br>value是刷新的内容 |
### 开发步骤
 #### 1.配置工程
  配置build.gn文件
•    在public_deps节点中添加以下配置：

```
public_deps = [
      "//foundation/distributeddatamgr/objectstore:objectstore_sdk"
    ]

```

•    设置用c++版本为c++17：

```
 static_library("objectstoremgr") {
 configs -= [ "//build/lite/config:language_cpp" ]
  cflags = [ "-fPIC" ]
   cflags_cc = cflags
   cflags_cc += [ "-std=c++17" ]
 configs += [ ":objectStore_config" ]
```
  #### 2.获取分布式对象仓库管理类
  ```
DistributedObjectStoreManager *ptr = DistributedObjectStoreManager::GetInstance();
if (ptr == nullptr) {
	return 0;
}
  ```
 #### 3.创建对象仓库
```
DistributedObjectStore *store = ptr->Create(BUNDLE_NAME, SESSION_ID);
TEST_ASSERT_TRUE(store != nullptr);
```
#### 4.创建数据对象

```
 DistributedObject *object = store->CreateObject("com.testobject.dosdemo1.model.Student", "key1");
 TEST_ASSERT_TRUE(object != nullptr);
```
#### 5.修改数据对象到本地
```c
//更新name属性的值为 myTestName
int ret = object->PutString("name", "myTestName");
TEST_ASSERT_TRUE(ret == 0);
ret = object->PutInt("age", 18);
TEST_ASSERT_TRUE(ret == 0);
ret = object->PutLong("testLong", 1);
TEST_ASSERT_TRUE(ret == 0);
ret = object->PutDouble("testDouble", 1.234);
TEST_ASSERT_TRUE(ret == 0);
ret = object->PutFloat("testFloat", 1.23456);
TEST_ASSERT_TRUE(ret == 0);
ret = object->PutShort("testShort", 2);
TEST_ASSERT_TRUE(ret == 0);
ret = object->PutBoolean("testBoolean", false);
TEST_ASSERT_TRUE(ret == 0);
ret = object->PutChar("testChar", 'a');
TEST_ASSERT_TRUE(ret == 0);
```
#### 6.同步数据对象到手机

```
store->Sync(object);
```

  #### 7.关闭数据对象仓库
数据对象仓库使用完毕后，需要调用Destroy接口进行关闭，关闭后所有的数据对象都会被销毁。
```
ptr->Destroy(BUNDLE_NAME, SESSION_ID);
```