FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# 添加 LLVM 19 官方 APT 源
RUN apt-get update && apt-get install -y --no-install-recommends \
    wget gnupg ca-certificates \
    && rm -rf /var/lib/apt/lists/* \
    && wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key \
    | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc \
    && echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-19 main" \
    > /etc/apt/sources.list.d/llvm.list

# 安装构建依赖
RUN apt-get update && apt-get install -y --no-install-recommends \
    clang-19 clang++-19 lld-19 \
    llvm-19 llvm-19-dev libclang-19-dev \
    build-essential libstdc++-11-dev \
    ripgrep \
    && rm -rf /var/lib/apt/lists/*

# 创建无版本号 symlink，供 FindCCompiler 的 PATH 扫描使用
RUN ln -sf /usr/bin/clang-19 /usr/local/bin/clang \
    && ln -sf /usr/bin/clang++-19 /usr/local/bin/clang++ \
    && ln -sf /usr/bin/llvm-config-19 /usr/local/bin/llvm-config

ENV CLANGXX_BIN=clang++-19
ENV LLVM_CONFIG_BIN=llvm-config-19
ENV CC_BIN=/usr/local/bin/clang

WORKDIR /workspace

COPY . .

# 修复 Windows 换行符 + LLVM Triple / StringRef API 兼容（不修改本地源文件）
RUN sed -i 's/\r$//' build_ir.sh test/run_tests.sh test/present/run_demo.sh 2>/dev/null \
    && sed -i 's/module\.setTargetTriple(target_triple)/module.setTargetTriple(target_triple.str())/' \
       backend/codegen/llvm_codegen.cpp \
    && sed -i 's/lookupTarget(target_triple, /lookupTarget(target_triple.str(), /' \
       backend/codegen/llvm_codegen.cpp \
    && sed -i 's/createTargetMachine(target_triple,/createTargetMachine(target_triple.str(),/' \
       backend/codegen/llvm_codegen.cpp \
    && sed -i 's/ctx\.Module()\.setTargetTriple(llvm::Triple("x86_64-unknown-linux-gnu"))/ctx.Module().setTargetTriple("x86_64-unknown-linux-gnu")/' \
       backend/irgen/irgen.cpp \
    && ./build_ir.sh

CMD ["/bin/bash"]
