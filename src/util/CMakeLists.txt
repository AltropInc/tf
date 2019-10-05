add_library (tf_util
    Enum.cpp
    StrScan.cpp
    StrPool.cpp
    Intrinsics.cpp
    FixedMemPool.cpp
)

# target_include_directories (Hello PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# add_executable (UtilTest Enum.cpp)
# target_link_libraries (helloDemo LINK_PUBLIC Hello)