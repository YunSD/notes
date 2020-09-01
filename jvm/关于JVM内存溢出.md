title: "空对象的计算方式" 

1. 一个字节 = 8个二进制位。
> 指针压缩。
>>  指针压缩：8字节 = 64位， 64位计算机有48（6字节）与16位（2字节保留位）组成。
---
>对象的组成
>> 对象头区域:header
>>>Mark Word  
>>>类型指针：基于哪个类生成的对象  
>>>数组长度
<br>
>>实例数据区域
>>>实例数据：Instance Data
<br>
>>对齐填充区域
>>>对齐填充 Padding  
>>>8字节对齐,永远8的整数倍，段页式
<br>

```
public class EmptyObject {
    public static void main(String[] args) {
        EmptyObject emptyObject = new EmptyObject();
        //开启指针压缩 16B = 8B + 4B(KClass Pointer) + 0 + 4B(补)
        //未开启指针压缩 16B = 8B +8B(KClass Pointer) + 0 + 0
        System.out.println(ClassLayout.parseInstance(emptyObject).toPrintable());
    }

}
```

>如果未开启指针压缩，对象头中存在填充对齐
>jvm 查看详情 java -XX:+PrintFlagsFinal -version grep ThreadStack
>栈帧计算 栈大小*1024/772
```
public class EmptyObject {
    private int deep = 0;
    private void test(){
        deep++;
        test();
    }
    public static void main(String[] args) {
        EmptyObject emptyObject = new EmptyObject();
        try{
            emptyObject.test();
        }catch (Throwable t){
            System.out.println(emptyObject.deep);
        }
    }

}
```
设置栈大小 -Xss160k 最小160