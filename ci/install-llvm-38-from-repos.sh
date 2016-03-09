

#be verbose, exit on error
set -exv

wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key|sudo apt-key add -
echo "deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.8 main" | sudo tee -a /etc/apt/sources.list
echo "deb-src http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.8 main" | sudo tee -a /etc/apt/sources.list
sudo apt-get update -qq
sudo apt-get install -y clang-3.8 clang-3.8-doc libclang-common-3.8-dev libclang-3.8-dev libclang1-3.8 libclang1-3.8-dbg libllvm-3.8-ocaml-dev libllvm3.8 libllvm3.8-dbg lldb-3.8 llvm-3.8 llvm-3.8-dev llvm-3.8-doc llvm-3.8-examples llvm-3.8-runtime clang-modernize-3.8 clang-format-3.8 python-clang-3.8 lldb-3.8-dev
