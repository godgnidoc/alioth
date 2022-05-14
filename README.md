<h1>The Alioth Project</h1>

The Alioth project aims to create the native programming language which is named Alioth, providing morden high level features without the support of VM.

![](doc/img/icon_with_text.png)

# 0. Index

- [0. Index](#0-index)
- [1. Getting Started](#1-getting-started)
  - [1.1. Building Requirements](#11-building-requirements)
  - [1.2. Building Project](#12-building-project)

# 1. Getting Started

Currently the alioth compiler is developed on the support of the LLVM-Core project which is well known as the compiler facility. To reduce the amount of hand-written code, many scripts are used to generate framework source code, thuse, we are not able to build this project on the platform other than linux.

## 1.1. Building Requirements

To build this project, you would need the following softwares installed properly.

1. The latest version of GNU gcc toolchain (which is at least version 11.2.0).
2. The latest version of GNU make (which is at least version 4.3).
3. The latest version of NodeJs distribution (which is at least version 16.13.1).
4. The latest version of GNU Bison (which is at least version 3.8.2).
5. The latest version of flex (which is at least version 2.6.4).
6. The latest version of LLVM (which is at least version 8.0.0).

## 1.2. Building Project

Make sure all those requirements are satisfied, then run the following command.

~~~bash
#!/bin/bash

make
~~~