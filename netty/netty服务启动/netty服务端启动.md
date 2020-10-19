## **Netty 服务段启动**

#### **创建服务端Channel**

#### 初始化服务端Channel

#### 注册Seletor

#### 端口绑定

---

> **bind() [用户代码入口]**
>
> >**initAndRegister() [初始化并注册]**
> >
> >> **newChannel() [创建服务端Channel]**

#### 对照代码：

```
ChannelFuture f = b.bind(8888).sync();
```

AbstractBootstrap.java

```
public ChannelFuture bind(SocketAddress localAddress) {
    validate();
    if (localAddress == null) {
        throw new NullPointerException("localAddress");
    }
    return doBind(localAddress);
}
```

```
private ChannelFuture doBind(final SocketAddress localAddress) {
    final ChannelFuture regFuture = initAndRegister();
    final Channel channel = regFuture.channel();
    ...
}
```

```
final ChannelFuture initAndRegister() {
    Channel channel = null;
    try {
        channel = channelFactory.newChannel();
        init(channel);
    } catch (Throwable t) {
    ...
}

```
ReflectiveChannelFactory.java
```
@Override
public T newChannel() {
    try {
        return clazz.newInstance();
    } catch (Throwable t) {
        throw new ChannelException("Unable to create Channel from class " + clazz, t);
    }
}
```