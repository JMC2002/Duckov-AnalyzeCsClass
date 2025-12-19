# AnalyzeCsClass
简单写一个分析CS文件中每个类的成员的代码，方便我写MOD的时候把反编译出来的代码接口喂给GPT，语言标准最好在C++26，不确定C\++23能否编译，应该可以吧。可执行文件会扫描input文件夹下的所有文本文件并输出到output下

input文件夹和output文件夹下默认放了反编译出来的代码以及类结构，可以直接查看

`Export-DuckovDlls.ps1`是一个`powershell`脚本，用于列出DLL中指定命名空间里指定深度的所有命名空间与类

## 编译
下载一个VS2026或者支持C\++26的编译器，打开项目编译即可，可通过修改`AnalyzeCsClass.cpp`中的相关定义来指定输入输出路径以及扫描的文件后缀名（默认为`.cs`）

下面展示与AI配合的工作流，如何让AI快速帮你猜到相关的API：

## 准备文件
### 获取命名空间
- 打开`Export-DuckovDlls.ps1`脚本，修改dll的路径变量`$baseDir`为你需要扫描的DLL路径，修改名称变量`$dllNames`为你需要扫描的DLL名称列表，修改命名空间变量`$namespaceFilters`为你需要扫描的命名空间前缀，修改深度变量`$depth`为你需要扫描的命名空间深度（如前缀填`Duckov.UI`想扫出所有`Duckov.UI.XXX`就填`1`）
- 对于逃离鸭科夫，可以直接下载[outputNamespace](outputNamespace)文件夹下的内容放到你本地，默认展开"Duckov", "Duckov.UI", "Duckov.Utilities"下面的一级命名空间

- 打开`ListRootNamespaces.ps1`脚本，同样修改对应的变量，运行脚本将会列出DLL中所有的根命名空间，方便你确认需要扫描的命名空间前缀
- 对于逃离鸭科夫，可以直接下载[outputRootNamespace](outputRootNamespace)文件夹下的内容放到你本地

### 获取类结构
- 编译运行本项目，将在输出文件夹下生成类结构文件
- 对于逃离鸭科夫，可以直接下载[output](outputClassStructure)文件夹下的内容放到你本地

## 喂给AI
首先描述你的需求背景，比如你在写MOD，想要实现什么功能，比如：
```prompt
我在写一个逃离鸭科夫的MOD（Duckov、不是逃离塔科夫，是一款基于Mono的U3D游戏，最高可以使用C#14，请尽管放心使用相关语法，同时社区中一般会使用harmony2.4.1开发功能），我的MOD想实现XXX功能，首先这是相关DLL的所有命名空间：
（放[outputRootNamespace]的内容）
接下来是比较关键的命名空间下面的子命名空间与类：
（放[outputNamespace]的内容）

你觉得哪些命名空间和类可能对实现需求有帮助？请列出你希望进一步查看的命名空间与类，我会把它们的类结构发给你。
```
答复后，你可以根据AI的建议，挑选一些类结构文件喂给AI，然后进一步问他想看什么：
```prompt
这是我反编译得到的相关的类的结构，是否对你有帮助？你想看这里面的哪些方法的具体实现或者别的什么东西，请告诉我。
（放[output]的内容）
```

然后还可以让类学习你想用的前置库，比如我自己的JmcModLib，我会告诉他：
```prompt
我还会使用一个叫JmcModLib的前置库来开发MOD，其中涉及打印请使用JmcModLib.Utils下的ModLogger，分为Trace/Debug/Info/Warn/Error/Fatal六个级别，请根据需要使用合适的级别打印日志信息，基本用法如下：
```csharp
ModLogger.Trace("消息");
ModLogger.Debug("调试信息");
ModLogger.Info("提示");
ModLogger.Warn("警告", ex);
ModLogger.Error("错误", ex);
ModLogger.Fatal(ex, "致命错误");
```\
其中Warn/Error的第二个参数是可选的异常对象，Fatal 在Debug模式下构建会抛出异常，Release下构建只会打印信息，防止用户运行时崩溃。此外，使用ModLogger会自动附加MOD名称前缀与调用函数名，因此不需要你手动添加。

如果涉及反射，你可以使用JmcModLib.Reflection下的类调用，该类的Get方法自带缓存，不需要你手动缓存，尽量使用委托和泛型版本提升性能，下面是他的文档：
（贴JmcModLib.Reflection的文档）

这些前置做完后，你就可以让AI帮你写代码了；特别地，如果你使用的是AI Studio，有一个branch的功能，可以直接从这里开一个分支出去，这样就不用每个对话都吟唱老半天。
```
## 常见问题
### 报错不允许执行脚本
- 管理员打开终端，运行以下命令：
  ```ps
  Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
  ```

### 报错找不到ilspycmd
- 打开终端，运行以下命令安装`ilspycmd`：
  ```ps
  dotnet tool install -g ilspycmd
  ```
- 执行`ilspycmd --version`确认安装成功

### 运行脚本中文输出乱码
请将脚本文件保存为UTF-8带签名格式