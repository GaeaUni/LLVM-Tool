set(CMAKE_CXX_STANDARD 20)
get_filename_component(TOOL_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)
list(APPEND LLVM_INCLUDE_DIRECTORIES
    ${TOOL_ROOT_PATH}/llvm-project/clang/include
    ${TOOL_ROOT_PATH}/llvm-project/llvm/include
)

if(USE_LLVM_TYPE MATCHES Release)
    list(APPEND LLVM_INCLUDE_DIRECTORIES
        ${TOOL_ROOT_PATH}/build/llvm/Release/tools/clang/include
        ${TOOL_ROOT_PATH}/build/llvm/Release/include
    )
    set(LLVM_PROJECT_PATH ${TOOL_ROOT_PATH}/build/llvm/Release)
else()
    list(APPEND LLVM_INCLUDE_DIRECTORIES
        ${TOOL_ROOT_PATH}/build/llvm/Debug/tools/clang/include
        ${TOOL_ROOT_PATH}/build/llvm/Debug/include
    )
    set(LLVM_PROJECT_PATH ${TOOL_ROOT_PATH}/build/llvm/Debug)
endif()
link_directories(${LLVM_PROJECT_PATH}/lib)

list(APPEND COMMAND_FLAGS
    "-fvisibility-inlines-hidden"
)

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

