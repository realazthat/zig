

#be verbose, exit on error
set -exv

wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key|sudo apt-key add -
echo "deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.7 main" | sudo tee -a /etc/apt/sources.list
echo "deb-src http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.7 main" | sudo tee -a /etc/apt/sources.list
sudo apt-get update -qq
sudo apt-get install -y clang-3.7 clang-3.7-doc libclang-common-3.7-dev libclang-3.7-dev libclang1-3.7 libclang1-3.7-dbg libllvm-3.7-ocaml-dev libllvm3.7 libllvm3.7-dbg lldb-3.7 llvm-3.7 llvm-3.7-dev llvm-3.7-doc llvm-3.7-examples llvm-3.7-runtime clang-modernize-3.7 clang-format-3.7 python-clang-3.7 lldb-3.7-dev

