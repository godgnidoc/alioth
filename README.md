# The Alioth Project

> Alioth 项目始于一个赛事课题，希望设计一款带有诸多语法糖的现代编程语言。赛事结束时，Alioth 仍有大量工作尚未完成，但限于时间安排，开发陷入停滞。时隔多年我们终于重拾夕日旧梦，当年的语法设计在众多现代编程语言的光辉照耀之下已经不值一提。但我们为自研编程语言所做的工作，我们基于实践对编译原理的理解，这些不应随风而逝。现在，Alioth 项目将带着对美和自由的追求和大家一起继续探索编译原理。

Alioth 项目致力于在最小外部依赖基于C++构建一套编译器前端基础设施。您可以使用 alioth 简化自己的编程语言的设计工作和编译器前端的开发工作。

> 仓库地址: <u>github.com/godgnidoc/alioth.git</u>

Alioth 包含如下功能：

- [READY] 使用 Grammar 属性文法设计您的语言
- [READY] 依据语法规则做语法分析并收集属性树
- [READY] 类似Jinja的模板引擎
- [WIP] 从语法规则推导抽象语法树骨架
- [TODO] LSP 语法服务器

> 上述功能的状态解读如下
> - SOON 核心功能已经可用，但尚未封装对应的命令行接口
> - WIP 核心功能尚在开发
> - TODO 尚未开始开发

## Quick Start 快速开始

`Alioth` 使用 `C++` 构建，你需要一套能够接受 `C++20` 标准的构建工具链。`Alioth` 使用 `vcpkg` 获取依赖并合入构建系统。如果你不打算使用 `vcpkg` 请手动安装如下依赖：

- `fmt` C++20标准字符串格式化框架的原版
- `nlohmann/json` 接口泛型非常棒的 `json` 处理库
- `gtest` Google 推出的 `C++` 测试框架

此外，`Alioth` 使用 `cmake` 作为默认构建系统，对于 `linux` 环境下的简单构建场景，可以使用如下指令：

```bash
#!/bin/bash

cmake --preset x64-linux-debug
cmake --build build/x64-linux-debug
```

`Alioth` 会提供一套核心代码库和一个命令行工具。大部分使用场景下我们需要将二者结合使用。

我们使用命令行工具生成一些骨架代码或静态数据，之后基于 `Alioth` 核心代码库开发我们自己的应用。

## Grammar 属性文法

Alioth 内置了一款名为 Grammar 的属性文法，可用于设计形式语言的词法规则和语法规则，并在产生式中标记综合属性和继承属性。

`Grammar` 文法是自包含的，参见 `grammar/grammar.grammar` 可知如何用 `Grammar` 定义 `Grammar` 文法。

Alioth 可以从 `Grammar` 生成一个 `LR(1)` 语法分析表，并使用一个类似于 `GLR` 语法分析器的并线分析器来分析目标源代码。

在 `Grammar` 中定义词法规则时，可以为单词指定一个或多个所属上下文。语法分析器会在每个状态下依据预期输入的下一个单词所在的上下文创建分析线路分支来并线分析。

利用此设计，可以将一些字面文本一致但语义不同的单词区分到不同的上下文以实现复杂文法设计而不被判定为歧义。

> TODO 介绍 Grammar 文法的编写规则

## Skeleton 骨架推导

Alioth 提供通用的 `ASTNode` 抽象。任何能够被 `Alioth Parser` 解析的语法，都可以使用 `ASTNode` 存储分析结果。

但基于通用语法分析树抽象做进一步语义分析时会遭遇过多的数据类型判断和数据结构调整，不便于实际功能开发。

`Skeleton` 推导算法可以从语法规则推导出每一种语法结构的全部句式可能携带的属性和属性的特征。

基于骨架描述，配合模板引擎即可为目标语言生成专用的语法分析树节点抽象。并兼容 Alioth 中已有的 `ASTNode` 抽象。

> 此模块已完成骨架描述推导，尚未完成目标语言定义代码生成，前置项：模板引用

## Attribute 属性提取

形式语言的抽象语法树可以覆盖完整的源代码文本。但其中包含了许多用于构成语法结构的句子结构，这些节点在语义分析阶段不能提供任何信息，反而会导致语义分析算法需要依照当前句式小心地跳过若干单词才能找到携带信息的节点。

`Grammar` 文法允许在产生式的符号上添加归纳属性和继承属性的标记。语法分析器在生成语法分析树的同时会依据当前句式所携带的标记自动形成一棵属性树。

属性树的数据结构更加干净，所有的节点均包含语义，数据结构更适合用作语义分析的输入。

参见 `grammar/grammar.grammar` 可以找到属性标记的使用例子。
> TODO 解释如何在产生式标注属性

## Template 模板引擎

`Python` 中的 `Jinja` 模板引擎非常强大，令人向往。Alioth 项目希望引入一种模板引擎用于生成客户语言的骨架代码。

但 `Alioth` 项目致力于在最少的外部依赖实现必备功能，我们思前想后认为引入 python 作为功能基础还是不妥。

而 `C++` 社区中的两个 `Jinja` 实现 `Jinja2Cpp` 和 `Jinja2CppLight` 前者功能强大但依赖体量也大。而后者的功能体量可能不够实现复杂的骨架生成。

所以，Alioth 基于内建的 `Grammar` 文法设计了 `Template` 模板语法，参见 `grammar/template.grammar` 可以获得语法定义。

> 得益于 Grammar 的歧义处理能力，`Template` 的关键字被定义得非常干净，对小拇指很友好.  
> 减少了左手shift+左手字符的情况，对没错，说的就是 '%'

下文展开讲解模板语法的主要功能。

### 表达式演算

在 `Template` 中可以将变量或表达式的演算结果插入文本，这是模板引擎的基础功能。

`Template` 支持变量引用、成员解析和下标运算以及管道运算：

> 下文语法高亮使用 jinja 语法

```jinja
字面字符串和字面数字 {{ "this is string" }} {{ -3.1415926e3 }}
引用变量 var {{ var }}
一个复杂的下标和成员混合表达式 {{ array[1][2].member[myIndex] }}
调用函数，支持0或多个参数 {{ group.func(expr1, expr2) }}
```

### 分支判断

在 `Template` 中可以依据某些表达式的演算结果来选择要生成的文本。

> 判定规则计划使用 JavaScript 中的 truthy 规则

分支语句由 `{{ if cond }}` 领起，由 `{{ end if }}` 终止。

分支语句中可以按顺序添加若干个 `{{ else if cond }}` 块，以及最后可选地的一个 `{{ else tnen}}` 块：

```jinja
{{ if mycond }}
这是外层成立分支
{{ else if second_cond }}
这是外层第一个备选分支
{{ else if third_cond }}
这是外层第二个备选分支
    {{if inner }}
    这是内层成立分支
    {{ end if }}
{{ else then }}
这是外层最后的备选分支
{{ end if }}
```

### 容器迭代

在 `Template` 中可以迭代容器，为容器中的每个元素都生成一次指定的文本。在迭代时，要为当前容器元素拟定一个临时变量名。此外，可选地还可以为当前元素在容器中的键和元素拟定变量名以便使用。

```jinja
{{ for value in array }}
...
{{ end for }}

{{ for value, key in object }}
...
{{ end for }}

{{ for value, key, index in something }}
...
{{ end for }}
```

在迭代时，可以为当前迭代上下文命名，之后可以从具名的迭代上下文中获取边界状态用于简便判断：

```jinja
{{ for@here value in array }}
  {{ if first@here }} The first value is {{ value }} {{ end if }}
  {{ if last@here }} The last value is {{ value }} {{ end if }}
  {{ if nonfirst@here }} The value {{ value }} is not first {{ end if }}
  {{ if nonlast@here }} The value {{ value }} is not last {{ end if }}
{{ end for }}
```

### 继承与覆写

在 `Template` 模板中，可以定义一些具有名字的 `block`，当其它模板继承当前模板时，可以选择其中一些块来覆写：

```jinja
这里是模板的模板
{{ block first }}
这是第一个块的默认内容，若不覆写，则应用此处内容
{{ end block }}

一些其他文本

{{ block first }}
这是第一个块的默认内容，若不覆写，则应用此处内容
{{ end block }}
```

上述样例定义了两个块，如果没人覆写的话，其中原本的内容就会被应用。

```jinja
{{ extends "./another.template" }}

{{ overwrite first }}
这里是覆写第一个块的内容
{{ end overwrite }}
```

上述模板使用 `extends` 语句指定了当前模板继承的模板。并在后文中使用 `overwrite` 语句块覆写了其中定义的一个块。另一个未被覆写的块将保持原始定义被输出。

> TODO 目前对 `extends` 模板的语法要求过于严格，overwrite 外部不能编辑任何非空白字符。这样写起来并不方便，计划后续放宽此限制。

### 调用

模板之间可以相互调用，调用模板时默认会传入当前模板正在使用的模型作为输入。您可以指定一个表达式的演算结果作为被调用的模板的输入模型。

```jinja
使用当前模型调用一个外部模板 {{ call "./another.template" }}

使用一个表达式的值调用外部模板 {{ call "./some.template" with my_variable }}
```

### 注释

在语句块之间，可以插入注释块，注释块会在渲染文本时被剔除。

```jinja
{{ -- 这里是注释 }}
{{ -- 注释可以跨行
      这里是第二行}}
```