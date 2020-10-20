# ***服务端创建 Channel***
### 通过 NioServerSocketChannel 举例

>newSocket ***<font color=#FF0000 >[jdk channel]</font>***  
>NioServerSocketChannelConfig() ***<font color=#FF0000 >[tcp 参数配置类]</font>***   
>AbstractNioChannel()  
>>ConfigureBlocking(false)  ***<font color=#FF0000 >[阻塞模式]</font>***   
>>AbstractChannel()  ***<font color=#FF0000 >[创建 id,unsafe,pipeline]</font>***   

NioServerSocketChannel.java （Socket初始过程）
---- 

构造函数 本质上是通过 java NIO SelectorProvider.openServerSocketChannel()方法进行构建

---
```
/**
    * Create a new instance
    */
public NioServerSocketChannel() {
    this(newSocket(DEFAULT_SELECTOR_PROVIDER));
}


private static ServerSocketChannel newSocket(SelectorProvider provider) {
    try {
        /**
            *  Use the {@link SelectorProvider} to open {@link SocketChannel} and so remove condition in
            *  {@link SelectorProvider#provider()} which is called by each ServerSocketChannel.open() otherwise.
            *
            *  See <a href="https://github.com/netty/netty/issues/2308">#2308</a>.
            */
        return provider.openServerSocketChannel();
    } catch (IOException e) {
        throw new ChannelException(
                "Failed to open a server socket.", e);
    }
}
```

SocketChannel 配置过程
----

>NioServerSocketChannelConfig 就是对当前配置的一个抽象
```
// Create a new instance using the given {@link ServerSocketChannel}.
    
public NioServerSocketChannel(ServerSocketChannel channel) {
    super(null, channel, SelectionKey.OP_ACCEPT);
    config = new NioServerSocketChannelConfig(this, javaChannel().socket());
}
```

>super(null, channel, SelectionKey.OP_ACCEPT); 

**AbstractNioChannel 父类构造函数**

***1. 设置 channel 非阻塞***

```
/**
    * Create a new instance
    *
    * @param parent the parent {@link Channel} by which this instance was created. May be {@code null}
    * @param ch   the underlying {@link SelectableChannel} on which it operates
    * @param readInterestOp  the ops to set to receive data from the {@link SelectableChannel}
    */
protected AbstractNioChannel(Channel parent, SelectableChannel ch, int readInterestOp) {
    super(parent);
    this.ch = ch;
    this.readInterestOp = readInterestOp;
    try {
        ch.configureBlocking(false);
    } catch (IOException e) {
        try {
            ch.close();
        } catch (IOException e2) {
            if (logger.isWarnEnabled()) {
                logger.warn(
                        "Failed to close a partially initialized socket.", e2);
            }
        }

        throw new ChannelException("Failed to enter non-blocking mode.", e);
    }
}
```
### AbstractChannel 是对 channel 的抽象（适用于 服务端与客户端） ###

>AbstractChannel.java

>>AbstractNioChannel 作为 AbstractChannel 的衍型类，使用super对channel的 id,unsafe,papiline进行配置
```
protected AbstractChannel(Channel parent) {
    this.parent = parent;
    id = newId();
    unsafe = newUnsafe();
    pipeline = newChannelPipeline();
}
```

