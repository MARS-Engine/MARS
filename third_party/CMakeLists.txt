add_library(vkbootstrap STATIC)
set_property(TARGET vkbootstrap PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(vkbootstrap PRIVATE vkbootstrap/VkBootstrap.h vkbootstrap/VkBootstrap.cpp)
target_include_directories(vkbootstrap PUBLIC vkbootstrap)
target_link_libraries(vkbootstrap ${Vulkan_LIBRARIES})
target_include_directories(vkbootstrap PUBLIC ${Vulkan_INCLUDE_DIR})

add_library(tinyobjloader STATIC)
set_property(TARGET tinyobjloader PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(tinyobjloader PRIVATE TinyObjLoader/tiny_obj_loader.h TinyObjLoader/tiny_obj_loader.cc)
target_include_directories(tinyobjloader PUBLIC tinyobjloader)

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE vma)