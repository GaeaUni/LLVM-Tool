bootstrap:
	brew install ninja
	brew install cmake
	brew install pre-commit
	pre-commit install
	make gen-tooling-debug
	make build-tooling-debug

gen-llvm-debug:
	rm -rf build/llvm/Debug/CMakeCache.txt;
	cmake -S llvm-project/llvm -Bbuild/llvm/Debug -G Ninja -DLLVM_ENABLE_PROJECTS="clang;libcxx;libcxxabi;clang-tools-extra";

gen-llvm-release:
	rm -rf build/llvm/Release/CMakeCache.txt;
	cmake -DCMAKE_BUILD_TYPE=Release -S llvm-project/llvm -Bbuild/llvm/Release -G Ninja -DLLVM_ENABLE_PROJECTS="clang;libcxx;libcxxabi;clang-tools-extra";

clean-llvm-debug:
	cmake --build build/llvm/Debug --target clean

clean-llvm-release:
	cmake --build build/llvm/Release --target clean

build-llvm-debug: gen-llvm-debug
	cmake --build build/llvm/Debug --target clangTooling --verbose

build-llvm-release: gen-llvm-release
	cmake --build build/llvm/Release --target clangTooling --verbose

gen-tooling-debug: build-llvm-release
	rm -rf build/Tool/Debug/CMakeCache.txt
	cmake -DUSE_LLVM_TYPE=Release -GNinja -S packages/call-graph -Bbuild/Tool/Debug -DCMAKE_BUILD_TYPE=Debug

build-tooling-debug:
	cmake --build build/Tool/Debug --target call-graph --verbose


