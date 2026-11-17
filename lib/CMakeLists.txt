add_library(AssetEngine STATIC
        BinaryWriter.cpp
        BinaryReader.cpp
        FileIO.cpp
        BinaryWriter.h
        BinaryReader.h
        FileIO.h
        ArchiveFormat.h)

target_include_directories(AssetEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(AssetEngine PUBLIC cxx_std_20)