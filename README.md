# tiny-chat

## Getting Started

1. clone 项目到本地
```bash
git clone https://github.com/Mer1997/tiny-chat.git
```

2. 项目结构介绍
```bash
➜  tiny-chat git:(main) ✗ tree -I 'build'
.
|-- 00_hello_world
|   |-- CMakeLists.txt
|   `-- server.cc
|-- CMakeLists.txt
|-- LICENSE
|-- README.md
```
每个目录作为单独的模块互不影响，例如 **00_hello_world** 目录编译后会得到 00_server
新建目录必须以数字开头，并包含 00_hello_world 的 CMakeLists.txt 文件
```bash
cp -r 00_hello_world 01_echo_server_and_client
touch 01_echo_server_and_client/client.cc
```

3. 编译和运行
运行
```bash
cd tiny-chat
cmake -Bbuild
cmake --build build
```
即可编译所有子目录下的所有二进制文件, 命名方式为 "数字+文件名"
所有的二进制文件都放在 build/bin 目录下，直接运行即可
```bash
./build/bin/00_server
```
