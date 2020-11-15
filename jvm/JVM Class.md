# ***Java 类文件结构***


----

## Class 类文件结构
----
- #### 任何一个 Class 文件都对应着唯一的一个类或接口的定义信息，但是反过来说，类或接口并以一定都得定义在文件里（例如类或接口也可以动态生成，直接送入类加载器）

>- Class 文件是一组以8个字节为基础单位的二进制流，各个数据项目严格按照顺序紧凑的排列在文件之中，中间没有添加任何分隔符，这使得整个 Class 文件中存储的内容几乎全部是程序运行的必要数据，没有空隙存在。当遇到需要占用8个字节以上的空间的数据项时，则会按照高位在前的方式分割成若干个8个字节进行存储。
>- 根据《Java 虚拟机规范》的规定，Class文件格式采用一种类似于C语言结构体的伪结构来存储数据，这种伪结构中只有两种数据类型：“无符号数”和“表”。
>>- 无符号属于基本的数据类型，以 u1、u2、u4、u8 来分别代表 1个字节、2个字节、4个字节和8个字节的无符号数，无符号数可以用来描述数字、索引引用、数据值或按照UTF-8编码构成字符串值。
>>- 表是由多个无符号数或者其他表作为数据项构成的复合数据类型，为了便于区分，所有表的命名都习惯地以“info”结尾。表用于描述有层次关系的复合结构的数据，整个 Class 文件本质上也可以视为是一张表。
>>- 无论是无符号数还是表，当需要描述同一类型但数量不定的多个数据时，经常会使用一个前置的容量计数器加若干个连续的数据项的形式，这时候称这一系列的某一类型的数据为某一类型的“集合”。

<br/>
<br/>
<br/>

- ### 魔数与 Class 文件的版本
> 每个 Class 文件的头4个字节被称为魔数（Magic Number）,它的唯一作用时确定这个文件是否为一个能够被虚拟机接受的 Class 文件。值为：0xCAFEBABE
> 
<br/>
<br/>
<br/>

- ### 常量池
> 常量池可以比喻为 Class 文件里的资源仓库，它是 Class 文件结构中与其他项目关联最多的数据，通常也是占用 Class 文件空间最大的数据项目之一，另外，它还是在 Class 文件中第一个出现的表类型数据项目。
>- 由于常量池中常数的数量是不固定的，所以在常量池的入口需要放置一项 u2 类型的数据，代表常量池容量计数值（constant_pool_count）。与 Java 中的语言习惯不同，这个容量计数是从1而不是0开始的，在 Class 文件格式规范制订之时，设计者将第0项常量空出来是有特殊考虑的，这样做的目的在于，如果后面某些指向常量池的索引值的数据在特定情况下需要表达“不引用任何一个常量池项目”的含义，可以把索引值设置为0来表示。
>- Class 文件结构中只有常量池的容量是从1开始，对于其他集合类型，包括接口索引集合、字段表集合、方法表集合等的容量计数都与一般习惯相同，是从0开始的。
>- 常量池中主要存放两大类常量：字面量（Literal）和符号引用（Symbolic References）。字面量比较接近于 Java 语言层面的常量概念，如文本字符串、被声明为 final 的常量值等。而符号引用则属于编译原理方面得概念，主要包括下面几类常量：
>>- 被模块导出或者开放的包（Package）
>>- 类和接口的全限定名（Fully Qualified Name）
>>- 字段的名称和描述符（Descriptor）
>>- 方法的名称和描述符
>>- 方法句柄和方法类型（Method Handle、Method Type、Invoke Dynamic）
>>- 动态调用点和动态常量（Dynamically-Computed Call Site、Dynamically-Computed Constant） 
<br/>
<br/>
<br/>

|类型|名称|数量|
|--|--|--|
u4|magic|1
u2|minor_version|1
u2|major_version|1
u2|constant_pool_conut|1
cp_info|constant_pool|constant_pool_conut-1
u2|access_flags|1
u2|this_class|1
u2|super_class|1
u2|interfaces_count|1
u2|interfaces|interface_count
u2|fields_count|1
field_info|fields|fields_count
u2|methods_count|1
method_info|methods|methods_count
u2|attributes_count|1
attribute_info|attributes|attributes_count

<br/>
<br/>
<br/>

- ### 访问标志
> 在常量池结束之后，紧接着的2个字节代表访问标志（access_flags）,这个标志用于识别一些类或者接口层次的访问信息，包括：这个 Class 是类还是接口；是否定义为 public 类型；是否是abstract;如果是类的话，是否被声明为 final。

|标志名称|标志值|含义|
|--|--|--|
ACC_PUBLIC|0x0001|是否是public类型
ACC_FINAL|0x0010|是否被声明为final,只有类可设置
ACC_SUPER|0x0020|是否允许使用 invokespecial 字节码指令的新语义，JDK1.2之后编译出来的类的这个标志都必须为真。
ACC_INTERFACE|0x0200|标识这是一个接口
ACC_ABSTRACT|0x0400|是否为 abstract 类型，对于接口或者抽象类来说，此标志值为真，其他类型为假
ACC_SYNTHEMIC|0x1000|标识这个类并非是由用户代码产生的
ACC_ANNOTATION|0x2000|标识这是一个注解
ACC_ENUM|0x4000|标识这是一个枚举
ACC_MODULE|0x8000|标识这是一个模块

<br>

> access_flags 中一共有16个标识符可以使用，当前只定义了9个，没有用到的标志位要求一律为零。

- ### 类索引、父类索引与接口索引集合
>- 类索引（this_class）和 父类索引（super_class）都是一个 u2 类型的数据，而接口索引集合（interfaces） 是一组 u2 类型的数据的集合，Class 文件中由这三项数据来确定该类型的继承关系。类索引用于确定这个类的全限定名，父类索引用于确定这个类的全限定名
>- 类索引、父类索引和接口索引集合都按顺序排列在访问标识之后，类索引和父类索引用两个 u2 类型的索引值表示，它们各自指向一个类型为 CONSTANT_Class_info 的类描述符常量，通过 CONSTANT_Class_info 类型的常量中的索引值可以找到定义在 CONSTANT_Utf8_info 类型的常量中的全限定名字符串。


