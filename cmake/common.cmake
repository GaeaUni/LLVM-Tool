set(CMAKE_CXX_STANDARD 20)
get_filename_component(TOOL_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)
list(APPEND LLVM_INCLUDE_DIRECTORIES
    ${TOOL_ROOT_PATH}/llvm-project/clang/include
    ${TOOL_ROOT_PATH}/llvm-project/llvm/include
)

MESSAGE("haha ${CMAKE_CURRENT_BINARY_DIR}")
if(CMAKE_BUILD_TYPE MATCHES Release)
    list(APPEND LLVM_INCLUDE_DIRECTORIES
        ${TOOL_ROOT_PATH}/build/llvm/Release/tools/clang/include
        ${TOOL_ROOT_PATH}/build/llvm/Release/include
    )
    link_directories(${TOOL_ROOT_PATH}/build/llvm/Release/lib)
else()
    list(APPEND LLVM_INCLUDE_DIRECTORIES
        ${TOOL_ROOT_PATH}/build/llvm/Debug/tools/clang/include
        ${TOOL_ROOT_PATH}/build/llvm/Debug/include
    )
    link_directories(${TOOL_ROOT_PATH}/build/llvm/Debug/lib)
endif()

set(LLVM_LINK_LIBRARIES
    clangTooling
    LLVMDemangle
    LLVMSupport
    LLVMCore
    LLVMBinaryFormat
    LLVMBitReader
    LLVMBitstreamReader
    LLVMFrontendOpenMP
    LLVMTransformUtils
    LLVMAggressiveInstCombine
    LLVMInstCombine
    LLVMScalarOpts
    LLVMAnalysis
    LLVMMC
    LLVMMCParser
    LLVMObject
    LLVMOption
    LLVMRemarks
    LLVMDebugInfoCodeView
    LLVMDebugInfoDWARF
    LLVMProfileData
    LLVMTextAPI
    clangBasic
    clangLex
    clangParse
    clangAST
    clangASTMatchers
    clangSema
    clangAnalysis
    clangEdit
    clangRewrite
    clangDriver
    clangSerialization
    clangFrontend
    clangToolingCore
    clangToolingInclusions
    clangFormat
    z
    curses
)

