

# Distributed Object User Manual (Small-System Devices)

The distributed data object management framework is an object-oriented in-memory data management framework. It provides application developers with basic data object management capabilities, such as creating, querying, deleting, modifying, and subscribing to in-memory objects. Featuring the distributed capability, this framework supports data object collaboration between multiple devices for the same application in the super device scenario.

## Basic Concepts

The distributed data object management framework consists of the following components:

 •	Data object: an instance of the data object model class used to store the runtime data of an application.

 •	Data object store: a data object management class used to insert, query, delete, and subscribe to data objects by using the data object store APIs.

## Limitations and Constraints

Small-system devices can be connected to large-system devices. Currently, only unidirectional object synchronization links are available for synchronizing distributed objects from small-system devices to large-system devices.

| Type| Description|
| ----------- | --------------------------------------- |
| int32_t     | 32-bit integer, corresponding to the **int** type of large-system devices.|
| int16_t     | 16-bit integer, corresponding to the **short** type of large-system devices.|
| int64_t     | Long integer.|
| char        | Character, corresponding to the **char** type of large-system devices.|
| float       | Single-precision floating-point, corresponding to the **float** type of large-system devices.|
| double      | Double-precision floating point, corresponding to the **double** type of large-system devices.|
| boolean     | Boolean, corresponding to the **boolean** type of large-system devices.|
| std::string | String, corresponding to the **java.lang.String** type of large-system devices.|
| int8_t      | 8-bit integer, corresponding to the **byte** type of large-system devices.|
## Development Guidelines
### Available APIs
•	**OHOS::ObjectStore::DistributedObjectStoreManager** in the **distributed_objectstore_manager.h** file is used to create and destroy a distributed object store.

| API| Description|
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| static DistributedObjectStoreManager* GetInstance()          | Obtains a **DistributedObjectStoreManager** instance.|
| DistributedObjectStore *Create(const std::string &bundleName, const std::string &sessionId) | Creates a distributed object store. <br>**bundleName** must be the same as **bundleName** of the HAP package for the large-system device. <br>**sessionId** must be the same as the **sessionId** used when the large-system device creates the distributed object store.|
| uint32_t Destroy(const std::string &bundleName, const std::string &sessionId) | Destroys a distributed object store. <br>**bundleName** must be the same as **bundleName** of the HAP package for the large-system device. <br>**sessionId** must be the same as the **sessionId** used when the large-system device creates the distributed object store.|

•	**OHOS::ObjectStore::DistributedObjectStore** in the **distributed_objectstore.h** file is used to create and synchronize a distributed object.

| API| Description|
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| DistributedObject *CreateObject(const std::string &classPath, const std::string &key) | Creates a distributed object. <br>**classPath** must be the same as the **clazz** path when the large-system device creates the distributed object, for example, **com.testobject.dosdemo1.model.Student**. <br>**key** must be the same as the **key** used when the large-system device creates the distributed object. A class can have multiple objects, and **key** uniquely identifies an object in the class.|
| uint32_t Sync(DistributedObject *object)                     | Synchronizes a distributed object.|
| void Close()                                                 | Closes a store. This API is automatically called in **DistributedObjectStoreManager.destroy**.|

•	**OHOS::ObjectStore::DistributedObject** in the **distributed_object.h** file is used to manage object attributes.

| API| Description|
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| uint32_t PutChar(const std::string &key, char value)         | Adds or updates an attribute of the **char** type. <br>**key** indicates the attribute name, which corresponds to the object name identified by **@Field** on the large-system device. <br>**value** indicates the new value of the attribute.|
| uint32_t PutInt(const std::string &key, int32_t value)       | Adds or updates an attribute of the **int32_t** type. <br>**key** indicates the attribute name, which corresponds to the object name identified by **@Field** on the large-system device. <br>**value** indicates the new value of the attribute.|
| uint32_t PutShort(const std::string &key, int16_t value)     | Adds or updates an attribute of the **int16_t** type. <br>**key** indicates the attribute name, which corresponds to the object name identified by **@Field** on the large-system device. <br>**value** indicates the new value of the attribute.|
| uint32_t PutLong(const std::string &key, int64_t value)      | Adds or updates an attribute of the **int64_t** type. <br>**key** indicates the attribute name, which corresponds to the object name identified by **@Field** on the large-system device. <br>**value** indicates the new value of the attribute.|
| uint32_t PutFloat(const std::string &key, float value)       | Adds or updates an attribute of the **float** type. <br>**key** indicates the attribute name, which corresponds to the object name identified by **@Field** on the large-system device. <br>**value** indicates the new value of the attribute.|
| uint32_t PutDouble(const std::string &key, double value)     | Adds or updates an attribute of the **double** type. <br>**key** indicates the attribute name, which corresponds to the object name identified by **@Field** on the large-system device. <br>**value** indicates the new value of the attribute.|
| uint32_t PutBoolean(const std::string &key, bool value)      | Adds or updates an attribute of the **bool** type. <br>**key** indicates the attribute name, which corresponds to the object name identified by **@Field** on the large-system device. <br>**value** indicates the new value of the attribute.|
| uint32_t PutString(const std::string &key, const std::string &value) | Adds or updates an attribute of the **std::string** type. <br>**key** indicates the attribute name, which corresponds to the object name identified by **@Field** on the large-system device. <br>**value** indicates the new value of the attribute.|
| uint32_t PutByte(const std::string &key, const int8_t &value) | Adds or updates an attribute of the **int8_t** type. <br>**key** indicates the attribute name, which corresponds to the object name identified by **@Field** on the large-system device. <br>**value** indicates the new value of the attribute.|
### How to Develop
 #### 1. Create a project.
  Configure the **build.gn** file.
•    Add the following configuration to the **public_deps** node:

```
public_deps = [
      "//foundation/distributeddatamgr/objectstore:objectstore_sdk"
    ]

```

•    Set the C++ version to **C++17**.

```
 static_library("objectstoremgr") {
 configs -= [ "//build/lite/config:language_cpp" ]
  cflags = [ "-fPIC" ]
   cflags_cc = cflags
   cflags_cc += [ "-std=c++17" ]
 configs += [ ":objectStore_config" ]
```
  #### 2. Obtain a **DistributedObjectStoreManager** instance.
  ```
DistributedObjectStoreManager *ptr = DistributedObjectStoreManager::GetInstance();
if (ptr == nullptr) {
	return 0;
}
  ```
 #### 3. Create a distributed object store.
```
DistributedObjectStore *store = ptr->Create(BUNDLE_NAME, SESSION_ID);
TEST_ASSERT_TRUE(store != nullptr);
```
#### 4. Create a distributed object.

```
 DistributedObject *object = store->CreateObject("com.testobject.dosdemo1.model.Student", "key1");
 TEST_ASSERT_TRUE(object != nullptr);
```
#### 5. Modify the data object at the local device.
```c
// Change the value of name to myTestName.
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
#### 6. Synchronize the data object to a mobile phone.

```
store->Sync(object);
```

#### 7. Close the distributed object store.
After using the distributed object store, call **Destroy** to close it. After the store is closed, all data objects in the store will be destroyed.
```
ptr->Destroy(BUNDLE_NAME, SESSION_ID);
```
