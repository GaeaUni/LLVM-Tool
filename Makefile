bootstrap:
	make gen-tooling-debug
	make build-tooling-debug

gen-llvm-debug:
	rm -rf build/llvm/Debug/CMakeCache.txt;
	cmake -S llvm-project/llvm -Bbuild/llvm/Debug -G Ninja -DLLVM_ENABLE_PROJECTS="clang;libcxx;libcxxabi;clang-tools-extra";

gen-tooling-debug: build-llvm-debug
	rm -rf build/Tool/Debug/CMakeCache.txt;
	cmake -GNinja -S packages/call-graph -Bbuild/Tool/Debug -DCMAKE_BUILD_TYPE=Debug

clean-llvm-debug:
	cmake --build build/llvm/Debug --target clean

build-llvm-debug: gen-llvm-debug
	cmake --build build/llvm/Debug --target clangTooling

build-tooling-debug:
	cmake --build build/Tool/Debug --target call-graph --verbose


