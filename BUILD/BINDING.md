# SpoutBinding — C ABI DLL 构建与 Java FFM 绑定指南

> 请注意：本文档由 AI 生成

本文档详细说明如何将 `SpoutBinding`（位于 `SPOUTSDK/SpoutBinding/`）编译为 Windows x86 DLL，并使用 `jextract`（JDK 25+ 的 Project Panama 工具）生成 Java Foreign Function & Memory（FFM）绑定。

---

## 目录

- [1. 概述](#1-概述)
- [2. 环境要求](#2-环境要求)
- [3. 编译 SpoutBinding DLL](#3-编译-spoutbinding-dll)
  - [3.1 方案 A：Windows 原生编译（MSVC）](#31-方案-awindows-原生编译msvc)
  - [3.2 方案 B：Linux 交叉编译（MinGW-w64）](#32-方案-blinux-交叉编译mingw-w64)
- [4. jextract：从 C 头文件生成 Java FFM 绑定](#4-jextract从-c-头文件生成-java-ffm-绑定)
  - [4.1 获取 jextract](#41-获取-jextract)
  - [4.2 运行 jextract](#42-运行-jextract)
  - [4.3 输出文件说明](#43-输出文件说明)
- [5. 在 Java 中使用 SpoutBinding](#5-在-java-中使用-spoutbinding)
- [6. 完整工作流示例](#6-完整工作流示例)
- [7. 故障排除](#7-故障排除)

---

## 1. 概述

`SpoutBinding` 是一个轻量级的 **C ABI（`extern "C"`）包装库**，只暴露三个函数：

| C 函数 | 作用 |
|---|---|
| `SpoutHandle spCreateSpout(void)` | 创建 Spout 发送者对象 |
| `int spSendFrameBufferObject(SpoutHandle, unsigned int fbo, unsigned int w, unsigned int h)` | 发送 FBO 的 `GL_COLOR_ATTACHMENT0` 纹理 |
| `void spReleaseSpout(SpoutHandle)` | 释放 Spout 对象 |

内部包装了 Spout SDK 的 `SpoutSender` C++ 类。通过 `__declspec(dllexport)` 导出符号，任何支持加载 C ABI 动态库的语言（Java、Python、C#、Rust 等）均可调用。

---

## 2. 环境要求

### 编译 DLL

| 依赖 | 说明 |
|---|---|
| **CMake** ≥ 3.15 | 构建系统 |
| **C++ 编译器** | MSVC（Windows）或 MinGW-w64（Linux 交叉编译） |
| **Windows SDK** | 包含 DirectX 11 头文件（d3d11.h, dxgi.h 等） |
| **OpenGL** | 链接 opengl32.lib / libopengl32.a |

> 注意：Spout 仅支持 Windows 目标平台。Linux 上需使用 MinGW-w64 交叉编译。

### 生成 Java 绑定

| 依赖 | 说明 |
|---|---|
| **JDK 25+** | 包含 Project Panama FFM API |
| **jextract** | 独立工具，从 [jdk.java.net/jextract](https://jdk.java.net/jextract/) 下载 |

---

## 3. 编译 SpoutBinding DLL

### 3.1 方案 A：Windows 原生编译（MSVC）

#### 步骤 1：配置 CMake

在项目根目录打开 **"x86 Native Tools Command Prompt for VS 2022"**（确保是 x86 / 32-bit 环境），运行：

```bat
mkdir build_msvc_x86
cd build_msvc_x86
cmake .. ^
    -G "Visual Studio 17 2022" -A Win32 ^
    -DSPOUT_BUILD_BINDING=ON ^
    -DSPOUT_BUILD_LIBRARY=OFF ^
    -DSPOUT_BUILD_SPOUTDX=OFF
```

#### 步骤 2：编译

```bat
cmake --build . --target SpoutBinding --config Release
```

#### 步骤 3：产物

```
build_msvc_x86/bin/
├── SpoutBinding.dll          ← 运行时 DLL（~800 KB）
└── SpoutBinding.lib          ← 导入库
```

> `bin/` 目录下还会生成 `Spout.dll`（因为 `SpoutBinding` 依赖 `Spout_static` 静态库，不会单独生成）。

---

### 3.2 方案 B：Linux 交叉编译（MinGW-w64）

#### 步骤 1：安装 MinGW-w64

```bash
# Ubuntu / Debian
sudo apt install mingw-w64 cmake make

# Fedora
sudo dnf install mingw32-gcc-c++ cmake make

# Arch Linux
sudo pacman -S mingw-w64-gcc cmake make
```

#### 步骤 2：配置 CMake（使用预置工具链文件）

项目包含两个 MinGW 工具链文件：

| 架构 | 工具链文件 | 编译器 | 输出 |
|---|---|---|---|
| **x86-64（64-bit）** | `cmake/toolchain-mingw-w64.cmake` | `x86_64-w64-mingw32-g++` | 推荐 |
| x86（32-bit） | `cmake/toolchain-mingw-w32.cmake` | `i686-w64-mingw32-g++` | 仅限旧系统 |

编译 **64-bit** 版本（推荐）：

```bash
cd /path/to/Spout2
rm -rf build
cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw-w64.cmake \
    -DSPOUT_BUILD_BINDING=ON \
    -DSPOUT_BUILD_LIBRARY=OFF \
    -DSPOUT_BUILD_SPOUTDX=OFF \
    -G "Unix Makefiles"
```

#### 步骤 3：编译

```bash
cmake --build build --target SpoutBinding --verbose
```

#### 步骤 4：产物

```
build/bin/
└── libSpoutBinding.dll          ← 运行时 DLL（~920 KB, x86-64 PE）

build/lib/
└── libSpoutBinding.dll.a        ← MinGW 导入库
```

> 注意：MinGW 生成的 DLL 默认带 `lib` 前缀（`libSpoutBinding.dll`），而 MSVC 生成 `SpoutBinding.dll`。两者本质相同，调用时注意库名即可。

---

## 4. jextract：从 C 头文件生成 Java FFM 绑定

### 4.1 获取 jextract

jextract **不包含在 JDK 中**，需单独下载：

- 访问 [https://jdk.java.net/jextract/](https://jdk.java.net/jextract/)
- 选择与 JDK 版本匹配的构建（如 `jextract-25`）
- 下载对应平台的压缩包（`linux-x64`, `windows-x64`, `macos-x64` 等）

解压：

```bash
# Linux
tar -xzf openjdk-25-jextract+*-linux-x64_bin.tar.gz
# 解压到目录如 jextract-25/

# Windows
# 解压 ZIP 文件到任意目录
```

验证：

```bash
./jextract-25/bin/jextract --version
# 输出示例: jextract 25 | JDK version 25+37-3491 | LibClang version clang 13.0.0
```

### 4.2 运行 jextract

```bash
# 创建输出目录
mkdir -p java-bindings

# 运行 jextract
/path/to/jextract-25/bin/jextract \
    --output java-bindings \
    --header-class-name SpoutBinding \
    --target-package com.spout.binding \
    -l libSpoutBinding \
    -I SPOUTSDK/SpoutBinding \
    SPOUTSDK/SpoutBinding/SpoutBinding.h
```

**参数说明：**

| 参数 | 值 | 说明 |
|---|---|---|
| `--output` | `java-bindings` | 生成的 `.java` 文件输出目录 |
| `--header-class-name` | `SpoutBinding` | 生成的入口类名 |
| `--target-package` | `com.spout.binding` | Java 包名 |
| `-l` | `libSpoutBinding` | 指定加载的共享库名（不含 `.dll` 扩展名） |
| `-I` | `SPOUTSDK/SpoutBinding` | 头文件搜索路径 |
| 最后一个参数 | `SpoutBinding.h` | 要解析的 C 头文件 |

> **关于 `-l` 参数的库名解析：**
>
> | 平台 | `-l libSpoutBinding` 实际加载 | 说明 |
> |---|---|---|
> | Windows (MSVC) | `SpoutBinding.dll` | `System.mapLibraryName` 去掉 `lib` 前缀 |
> | Windows (MinGW) | `libSpoutBinding.dll` | MinGW 保留 `lib` 前缀 |
> | Linux (测试) | `liblibSpoutBinding.so` | 仅用于语法验证 |
>
> 如果库名不匹配，可改为 `-l :SpoutBinding.dll`（用 `:` 前缀指定精确路径/文件名）。

### 4.3 输出文件说明

运行成功后生成：

```
java-bindings/
└── com/
    └── spout/
        └── binding/
            ├── SpoutBinding.java              ← 主绑定类（含三个 API 方法）
            └── SpoutBinding$shared.java       ← 共享类型定义
```

**生成的 Java API 签名：**

```java
package com.spout.binding;

public class SpoutBinding {

    // 类型映射: void* → MemorySegment
    public static final AddressLayout SpoutHandle = ...;

    // 1. 创建 Spout 发送者
    public static MemorySegment spCreateSpout();

    // 2. 发送 FBO (返回 1=成功, 0=失败)
    public static int spSendFrameBufferObject(
        MemorySegment handle,  // SpoutHandle
        int fbo,               // GLuint (unsigned int)
        int width,             // unsigned int
        int height             // unsigned int
    );

    // 3. 释放 Spout 发送者
    public static void spReleaseSpout(MemorySegment handle);
}
```

---

## 5. 在 Java 中使用 SpoutBinding

### 5.1 编译

```bash
javac -d out \
    java-bindings/com/spout/binding/SpoutBinding.java \
    java-bindings/com/spout/binding/SpoutBinding\$shared.java \
    YourApp.java
```

### 5.2 运行

```bash
java --enable-native-access=ALL-UNNAMED \
     -Djava.library.path=/path/to/build/bin \
     -cp out \
     YourApp
```

> **`--enable-native-access=ALL-UNNAMED`** 是 Project Panama FFM 必需的 JVM 标志。

### 5.3 最小示例

```java
package com.spout.binding;

import java.lang.foreign.MemorySegment;

public class SpoutBindingTest {

    public static void main(String[] args) {
        System.out.println("SpoutBinding Java FFM Test");

        // 1. 创建 Spout 发送者
        MemorySegment spout = SpoutBinding.spCreateSpout();
        if (spout.equals(MemorySegment.NULL)) {
            System.err.println("FAIL: spCreateSpout returned NULL");
            System.exit(1);
        }
        System.out.println("OK: spCreateSpout()");

        // 2. 发送 FBO
        //    注意：此处假设 OpenGL 上下文已创建，
        //    FBO 已绑定且 GL_COLOR_ATTACHMENT0 上挂载了纹理
        int fbo = 0;      // 0 = 默认 framebuffer
        int w   = 1920;
        int h   = 1080;
        int result = SpoutBinding.spSendFrameBufferObject(spout, fbo, w, h);
        System.out.println("SendFBO: " + (result == 1 ? "成功" : "失败"));

        // 3. 释放
        SpoutBinding.spReleaseSpout(spout);
        System.out.println("OK: spReleaseSpout()");
    }
}
```

---

## 6. 完整工作流示例

以下是在 **Ubuntu Linux 交叉编译 + jextract 生成绑定** 的完整流程：

```bash
# ===== Step 1: 编译 DLL =====
cd /workspaces/Spout2

cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw-w32.cmake \
    -DSPOUT_BUILD_BINDING=ON \
    -DSPOUT_BUILD_LIBRARY=OFF \
    -DSPOUT_BUILD_SPOUTDX=OFF

cmake --build build --target SpoutBinding

# 验证 DLL 导出符号
i686-w64-mingw32-objdump -p build/bin/libSpoutBinding.dll | grep -A 20 "Ordinal"
# 应看到: spCreateSpout, spReleaseSpout, spSendFrameBufferObject

# ===== Step 2: 下载 jextract =====
curl -sL "https://download.java.net/java/early_access/jextract/25/2/openjdk-25-jextract+2-4_linux-x64_bin.tar.gz" \
    -o /tmp/jextract.tar.gz
tar -xzf /tmp/jextract.tar.gz -C /tmp/

/tmp/jextract-25/bin/jextract --version
# => jextract 25 | JDK version 25+37-3491 | LibClang version clang 13.0.0

# ===== Step 3: 生成 Java 绑定 =====
mkdir -p build/java-bindings
/tmp/jextract-25/bin/jextract \
    --output build/java-bindings \
    --header-class-name SpoutBinding \
    --target-package com.spout.binding \
    -l libSpoutBinding \
    -I SPOUTSDK/SpoutBinding \
    SPOUTSDK/SpoutBinding/SpoutBinding.h

# ===== Step 4: 编译 Java 代码 =====
javac -d build/java-classes \
    build/java-bindings/com/spout/binding/*.java

# ===== Step 5 (仅 Windows): 运行时测试 =====
# 将 DLL 和 class 文件复制到 Windows 机器后执行：
# java --enable-native-access=ALL-UNNAMED \
#      -Djava.library.path=. \
#      -cp build/java-classes \
#      com.spout.binding.SpoutBindingTest
```

---

## 7. 故障排除

### 7.1 CMake 配置失败 — "Spout is not supported outside of MS Windows"

**原因：** 未使用 MinGW 工具链，CMake 检测到当前 OS 不是 Windows。

**解决：** 确保传入了 `-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw-w32.cmake`。

### 7.2 jextract 找不到 `d3d11.h` 或 `windows.h`

**原因：** MinGW 头文件路径未包含在 include 搜索路径中。

**解决：** 手动指定 MinGW include 目录：

```bash
jextract \
    -I /usr/i686-w64-mingw32/include \
    -I SPOUTSDK/SpoutBinding \
    ...
```

### 7.3 Java 运行时 `UnsatisfiedLinkError: Can't find libSpoutBinding`

**原因：** `System.mapLibraryName("libSpoutBinding")` 解析的库名与实际文件名不匹配。

**解决：** 改用 `-l :SpoutBinding.dll`（精确文件名，以 `:` 开头），或在 `SpoutBinding.java` 中修改 `SYMBOL_LOOKUP` 中的库名。

### 7.4 Java 运行时 `SymbolLookup failed for spSendFrameBufferObject`

**原因：** DLL 中未导出该符号。

**解决：** 用 `objdump -p libSpoutBinding.dll | grep -A 100 "Export"` 检查导出表。
确认 `SpoutBinding.h` 中函数有 `SPOUTBINDING_API` 修饰，且 `.cpp` 编译时定义了 `SPOUTBINDING_EXPORTS`。

---

## 附录

### A. 关键文件列表

| 文件 | 作用 |
|---|---|
| `SPOUTSDK/SpoutBinding/SpoutBinding.h` | C ABI 头文件（纯 C，jextract 可解析） |
| `SPOUTSDK/SpoutBinding/SpoutBinding.cpp` | 实现（包装 SpoutSender） |
| `SPOUTSDK/SpoutBinding/CMakeLists.txt` | CMake 构建定义 |
| `cmake/toolchain-mingw-w32.cmake` | MinGW x86 交叉编译工具链 |

### B. 类型映射说明

| C 类型 | Java FFM 类型 | 说明 |
|---|---|---|
| `void*` (SpoutHandle) | `MemorySegment` | 不透明指针 |
| `unsigned int` (GLuint / fbo, width, height) | `int` | C `unsigned int` 映射为 Java `int` |
| `int` (返回值) | `int` | 1=成功, 0=失败 |

### C. 参考链接

- [Spout SDK 官方文档](https://spoutgl-site.netlify.app/)
- [Project Panama / jextract 官网](https://jdk.java.net/jextract/)
- [OpenJDK FFM API 文档](https://openjdk.org/jeps/454)
