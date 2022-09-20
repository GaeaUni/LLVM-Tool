bootstrap:
	brew install ninja
	brew install cmake
	brew install pre-commit
	pre-commit install
	make build-lldb-tests-debug

gen-llvm-debug:
	cmake -DCMAKE_BUILD_TYPE=Debug -S llvm-project/llvm -Bbuild/llvm/Debug -G Ninja -DLLDB_BUILD_FRAMEWORK=1 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DLLVM_ENABLE_PROJECTS="clang;lldb;clang-tools-extra" -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi";

gen-llvm-release:
	cmake -DCMAKE_BUILD_TYPE=Release -S llvm-project/llvm -Bbuild/llvm/Release -G Ninja -DLLDB_BUILD_FRAMEWORK=1 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DLLVM_ENABLE_PROJECTS="clang;lldb;clang-tools-extra" -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi";

clean-llvm:
	rm -rf build;

# clang Tooling
build-llvm-debug: gen-llvm-debug
	cmake --build build/llvm/Debug --target clangTooling --verbose

build-llvm-release: gen-llvm-release
	cmake --build build/llvm/Release --target clangTooling --verbose

gen-tooling-debug: build-llvm-release
	rm -rf build/Tool/Debug/CMakeCache.txt
	cmake -DUSE_LLVM_TYPE=Release -GNinja -S packages/call-graph -Bbuild/Tool/Debug -DCMAKE_BUILD_TYPE=Debug

build-tooling-debug:
	cmake --build build/Tool/Debug --target call-graph --verbose

# lldb
build-lldb-debug:
	if [ ! -d build/llvm/Debug ]; then make gen-llvm-debug;fi
	cmake --build build/llvm/Debug --target lldb --verbose

build-lldb-release:
	if [ ! -d build/llvm/Debug ]; then make gen-llvm-release;fi
	cmake --build build/llvm/Release --target lldb --verbose

# tests
gen-lldb-tests-debug:build-lldb-debug
	cmake -DCMAKE_BUILD_TYPE=Debug -S packages/lldb-plugin -Bbuild/LLDBPlugin -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=1

build-lldb-tests-debug:
	if [ ! -d build/LLDBPlugin ]; then make gen-lldb-tests-debug;fi
	cmake --build build/LLDBPlugin --target LLDBPluginTests --verbose

