# ***JDK 调优工具***

- [***JDK 调优工具***](#jdk-调优工具)
  - [- GC日志详解](#--gc日志详解)
  - [JDK 内置指令](#jdk-内置指令)
      - [Jps-虚拟机进程状态工具](#jps-虚拟机进程状态工具)
      - [Jmap-虚拟机内存映射工具](#jmap-虚拟机内存映射工具)
      - [Jstack-堆栈跟踪](#jstack-堆栈跟踪)
      - [JInfo-配置信息](#jinfo-配置信息)
      - [Jstat-虚拟机统计信息监视工具](#jstat-虚拟机统计信息监视工具)
  - [GC日志详解](#gc日志详解)
----

## JDK 内置指令

#### Jps-虚拟机进程状态工具

&ensp;&ensp;&ensp;&ensp;**命令名称：虚拟机进程状态工具(Java Virtual Machine Process Status Tool)。**

&ensp;&ensp;&ensp;&ensp;**命令描述：该命令可以用来查看当前的 java 进程。**

&ensp;&ensp;&ensp;&ensp;**命令格式：`jps [options] [hostid] `**

&ensp;&ensp;&ensp;&ensp;**`[options]` 选项 ：**
- **`-q`：** 抑制类名，JAR 文件名和传递给该 main 方法的参数的输出，仅生成本地VM标识符的列表。
- **`-m`：** 输出传递给 main 方法的参数。对于嵌入式 JVM null。
- **`-l`：** 输出完全的包名，应用主类名，JAR 的完全路径名。
- **`-v`：** 输出 JVM 参数。
- **`-V`：** 输出通过 flag 文件传递到 JVM 中的参数(.hotspotrc 文件或 -XX:Flags= 所指定的文件。 
- **`-Joption`：** 传递参数到 JVM,例如: -J-Xms512m。

&ensp;&ensp;&ensp;&ensp;**`[hostid]` 主机标识符 ：**`[protocol:][[//]hostname][:port][/servername]`

&ensp;&ensp;&ensp;&ensp;**示例部分：**
1. **`jsp`:** 列出在本地主机上检测到的 JVM。
   ```java
    >jps
    18027 Java2Demo.JAR 
    18032 jps 
    18005 jstat
   ```
2. **`jps -l remote.domain`:** 列出在远程主机上的检测到的 JVM。
   ```java
   >jps -l remote.domain:2002
    3002 /opt/jdk1.7.0/demo/jfc/Java2D/Java2Demo.JAR 
    2857 sun.tools.jstatd.jstatd
   ```


#### Jmap-虚拟机内存映射工具

&ensp;&ensp;&ensp;&ensp;**命令名称：虚拟机内存映射(Java Virtual Machine Memory Map Tool)。**

&ensp;&ensp;&ensp;&ensp;**命令描述：输出指定进程或核心文件或远程调试服务器的共享对象内存映射或堆内存的详细信息。**

&ensp;&ensp;&ensp;&ensp;**命令格式：**
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;**`jmap [options] pid `：**为其打印堆栈跟踪的进程 ID。该进程必须是 Java 进程。要获取机器上运行的 Java 进程的列表，可以使用 jps。
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;**`jmap [options] executable core `**
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;**`jmap [options] [server-id@] remote-hostname-or-IP `**

&ensp;&ensp;&ensp;&ensp;**`[options]` 选项(互斥) ：**
- **`<no option>`：** 如果不使用任何选项，jmap 会打印共享对象映射。对于目标 VM 中加载的每个共享库，将打印起始地址，映射的大小以及共享库文件的完整路径。这类似于 Solaris pmap 实用程序。
- **`-dump:[live,]format=b,file=<filename>`：** 将 Java 堆以 hprof 二进制格式转储到文件名。该 live 选项是可选的。如果指定，则仅转储堆中的活动对象。要浏览堆转储，可以使用 jhat（Java 堆分析工具）读取生成的文件 **(也可以设置内存溢出自动导出 dump 文件,`-XX:+HeapDumpOnOutOfMemoryError`与 `-XX:HeapDumpPath` )。**
- **`-finalizerinfo`：** 在等待完成的对象上打印信息。
- **`-heap`：** 打印堆的摘要。打印使用的 GC 算法，堆配置以及按代打印堆的使用情况。
- **`-histo[:live]`：** 打印堆的直方图。对于每个 Java 类，将打印对象数量，以字节为单位的内存大小以及完全限定的类名称。VM 内部类名称以 “*” 前缀打印。如果指定了 live 子选项，则仅计算活动对象。
- **`-permstat`：** 打印 Java 堆中永久代的类加载器的统计信息。对于每个类加载器，将打印其名称，活动性，地址，父类加载器以及已加载的类的数量和大小。此外，还会打印内部字符串的数量和大小。
- **`-F`：** 如果 pid 没有响应，则与`jmap -dump`或`jmap -histo`选项一起使用。此模式不支持 live 子选项。
- **`-H`：** 打印帮助信息。
- **`-help`：** 打印帮助信息。
- **`-J<flag>`：** 将 `<flag>` 传递到运行 jmap 的 Java 虚拟机。

&ensp;&ensp;&ensp;&ensp;**示例部分：**
1. **`jmap -histo pid `:** 打印堆的直方图。
   ```java
    >jmap -histo pid > ./log.txt

    file output:

    num     #instances      #bytes   class name
    ----------------------------------------------
    1:         48284       23506488  [B
    2:        218282       21864792  [C
    3:        388746        9329904  java.util.LinkedList$Node
    4:         28002        8499104  [I
    ...
   ```
2. **`jmap -dump format=b,file=<filename> pid`:** 转储 Java 堆到 filename 文件。
   ```java
    >jmap -dump format=b,file=<filename> pid
    Dumping heap to File<filename.hprof>...
    Heap dump file created
   ```

#### Jstack-堆栈跟踪

&ensp;&ensp;&ensp;&ensp;**命令名称：Java 堆栈跟踪(Stack Trace)。**

&ensp;&ensp;&ensp;&ensp;**命令描述：为给定的 Java 进程或核心文件或远程调试服务器打印 Java 线程的 Java 堆栈跟踪。**

&ensp;&ensp;&ensp;&ensp;**命令格式：**
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;**`jstack [options] pid `**
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;**`jstack [options] executable core `**
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;**`jstack [options] [server-id@] remote-hostname-or-IP `**

&ensp;&ensp;&ensp;&ensp;**`[options]` 选项(互斥) ：**
- **`-F`：** 当 `jstack [-l] pid` 没有响应时，强制执行堆栈转储。
- **`-l`：** 长时间列出。打印有关锁的其他信息，例如拥有的 `java.util.concurrent` 拥有的同步器列表。
- **`-m`：** 打印混合模式（Java 和本机 C/C++ 框架）堆栈跟踪。
- **`-h`：** 打印帮助信息。
- **`-help`：** 打印帮助信息。


#### JInfo-配置信息

&ensp;&ensp;&ensp;&ensp;**命令名称：Java 配置信息(Configuration Info)。**

&ensp;&ensp;&ensp;&ensp;**命令描述：打印给定Java进程或核心文件或远程调试服务器的 Java 配置信息。**

&ensp;&ensp;&ensp;&ensp;**命令格式：**
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;**`jinfo [options] pid `**
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;**`jinfo [options] executable core `**
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;**`jinfo [options] [server-id@] remote-hostname-or-IP `**

&ensp;&ensp;&ensp;&ensp;**`[options]` 选项(互斥) ：**
- **`<no option>`：** 显示命令行标志以及系统属性。
- **`-flag name`：** 打印给定命令行标志的名称和值。
- **`-flag [+|-]name`：** 启用或禁用给定的布尔命令行标志。
- **`-flag name=value`：** 将给定的命令行标志设置为指定的值。
- **`-flags`：** 打印传递给 JVM 的命令行标志。
- **`-sysprops`：** 将 Java System 属性打印为键值对形式。
- **`-h`：** 打印帮助信息。
- **`-help`：** 打印帮助信息。

&ensp;&ensp;&ensp;&ensp;**示例部分：**
1. **`jmap -flags pid `:** 打印指定进程为 pid 的 Java 配置信息
   ```java
    >jmap -histo pid

    file output:

    Attaching to process ID 26860, please wait...
    Debugger attached successfully.
    Server compiler detected.
    JVM version is 25.65-b01
    Non-default VM flags: -XX:CICompilerCount=4 -XX:InitialHeapSize=268435456 -XX:MaxHeapSize=734003200 -XX:MaxNewSize=244318208 -XX:MinHeapDeltaBytes=524288 -XX:NewSize=89128960 -XX:OldSize=179306496 
    ...
   ```


#### Jstat-虚拟机统计信息监视工具

&ensp;&ensp;&ensp;&ensp;**命令名称：虚拟机统计信息监视工具(Virtual Machine Statistics Monitoring Tool)。**

&ensp;&ensp;&ensp;&ensp;**命令描述：显示 HotSpot 虚拟机的性能统计数据。**

&ensp;&ensp;&ensp;&ensp;**命令格式：**
&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;**`jstat [ generalOption | outputOptions vmid [interval[s|ms] [count]] ]`**
&ensp;&ensp;&ensp;&ensp;**参数 ：**
- **`generalOption`:** 单个常规命令行选项( `-help` 或 `-options` )。
- **`outputOptions`:** 一个或多个输出选项，由单个`statOption`以及`-t`，`-h`和`-J`选项中的任何一个组成 。
- **`vmid`:** 虚拟机标识符，一个字符串，指示目标Java虚拟机（JVM）。通用语法是:`[protocol:][//]lvmid[@hostname[:port]/servername]`。
- **`interval[s|ms]`:** 采样间隔，单位为秒(s)或毫秒(ms)。默认单位是毫秒。必须为正整数。如果指定，jstat 将在每个间隔产生其输出。
- **`count`:** 要显示的样本数。默认值为无穷大；也就是说，jstat 将显示统计信息，直到目标 JVM 终止或 jstat 命令终止为止。必须为正整数。

&ensp;&ensp;&ensp;&ensp;**`[statOption]` 选项 ：**
- **`class`:** 有关类加载器的行为的统计信息。
- **`compiler`:** HotSpot 即时编译器行为的统计信息。
- **`gc`:** 垃圾收集堆行为的统计信息。
- **`gccapacity`:** 各分代及其相应空间的容量统计。
- **`gccause`:** 垃圾回收统计信息的摘要(等同`-gcutil`)，以及最近和当前（如果适用）的垃圾回收事件的原因。
- **`gcnew`:** 	新生代行为的统计信息
- **`gcnewcapacity`:** 新生代大小及其相应空间的统计信息。
- **`gcold`:** 	有关老年代和永久代的行为的统计信息。
- **`gcoldcapacity`:** 	老年代的统计数据。
- **`gcpermcapacity`:** 永久代大小的统计信息。
- **`gcutil`:** 垃圾收集统计信息摘要。
- **`printcompilation`:** HotSpot 编译方法统计信息。

&ensp;&ensp;&ensp;&ensp;**示例部分([详见 Oracle官网](https://docs.oracle.com/javase/7/docs/technotes/tools/share/jstat.html))：**
1. **`jstat -gcutil pid 250 7 `:** 以 250ms 的间隔进行 7 个采样，并显示`-gcutil`选项指定的输出。
   ```java
    >jstat -gcutil pid 250 7

    file output:

    S0     S1     E      O      P     YGC    YGCT    FGC    FGCT     GCT
    12.44   0.00  27.20   9.49  96.70    78    0.176     5    0.495    0.672
    12.44   0.00  62.16   9.49  96.70    78    0.176     5    0.495    0.672
    12.44   0.00  83.97   9.49  96.70    78    0.176     5    0.495    0.672
     0.00   7.74   0.00   9.51  96.70    79    0.177     5    0.495    0.673
     0.00   7.74  23.37   9.51  96.70    79    0.177     5    0.495    0.673
     0.00   7.74  43.82   9.51  96.70    79    0.177     5    0.495    0.673
     0.00   7.74  58.11   9.51  96.71    79    0.177     5    0.495    0.673
   ```
2. **`jstat -gcnew -h3 pid 250`:** 以 250ms 的间隔进行 7 个采样，并显示`-gcutil`选项指定的输出，另外，它使用`-h3` 选项在每三行数据之后输出列标题。
   ```java
    >jstat -gcnew -h3 pid 250 S0C

    file output:

    S0C    S1C    S0U    S1U   TT  MTT   DSS      EC       EU     YGC    YGCT
    64.0   64.0    0.0   31.7  31  31   32.0    512.0    178.6    249    0.203
    64.0   64.0    0.0   31.7  31  31   32.0    512.0    355.5    249    0.203
    64.0   64.0   35.4    0.0   2  31   32.0    512.0     21.9    250    0.204
    S0C    S1C    S0U    S1U   TT  MTT   DSS      EC       EU     YGC    YGCT
    64.0   64.0   35.4    0.0   2  31   32.0    512.0    245.9    250    0.204
    64.0   64.0   35.4    0.0   2  31   32.0    512.0    421.1    250    0.204
    64.0   64.0    0.0   19.0  31  31   32.0    512.0     84.4    251    0.204
    S0C    S1C    S0U    S1U   TT  MTT   DSS      EC       EU     YGC    YGCT
    64.0   64.0    0.0   19.0  31  31   32.0    512.0    306.7    251    0.204
   ```
3. **`jstat -gcoldcapacity -t pid 250 3`:** 以 250ms 的间隔进行 3 个采样，并显示`-gcoldcapacity`选项指定的输出，另外，它使用`-t` 选项在列头生成所述的每个样本的时间标记。
   ```java
    >jstat -gcoldcapacity -t 21891 250 3

    file output:

    Timestamp   OGCMN       OGCMX        OGC          OC        YGC    FGC   FGCT    GCT
    150.1       1408.0      60544.0      11696.0      11696.0   194    80    2.874   3.799
    150.4       1408.0      60544.0      13820.0      13820.0   194    81    2.938   3.863
    150.7       1408.0      60544.0      13820.0      13820.0   194    81    2.938   3.863
   ```


## GC日志详解

&ensp;&ensp;&ensp;&ensp;对于 JAVA 应用我们可以通过一些配置把程序运行过程中的 GC 日志全部打印出来，然后分析 GC 日志得到关键性指标，分析 GC 原因，调优 JVM 参数。

```java
// 打印GC日志方法，在JVM参数里增加参数，%t 代表时间 
 ‐Xloggc:./gc‐%t.log ‐XX:+PrintGCDetails ‐XX:+PrintGCDateStamps  ‐XX:+PrintGCTimeStamps ‐XX:+PrintGCCause 
 ‐XX:+UseGCLogFileRotation ‐XX:NumberOfGCLogFiles=10 ‐XX:GCLogFileSize=100M 
```

&ensp;&ensp;&ensp;&ensp; **JVM 参数汇总查看命令:**
1. **java -XX:+PrintFlagsInitial** ： 表示打印出所有参数选项的默认值 
2. **java -XX:+PrintFlagsFinal** ： 表示打印出所有参数选项在运行程序时生效的值

