cmake_minimum_required(VERSION 3.16)
project(hanlu)

set(CMAKE_CXX_STANDARD 17)
add_library(hanlu
        frameworks/jskitsimpl/src/adaptor/js_distributedobjectstore.cpp
        frameworks/innerkitsimpl/src/adaptor/distributed_object_store_impl.cpp
        frameworks/jskitsimpl/src/adaptor/js_watcher.cpp
        frameworks/jskitsimpl/src/adaptor/js_distributedobject.cpp
        frameworks/jskitsimpl/src/adaptor/js_object_wrapper.cpp
        frameworks/jskitsimpl/src/adaptor/js_module_init.cpp)
include_directories(./interfaces/innerkits)
include_directories(./frameworks/jskitsimpl/include/adaptor)
include_directories(./frameworks/innerkitsimpl/include/adaptor)
include_directories(./frameworks/innerkitsimpl/include/common)
include_directories(./base/include)
