# ***注册 Selector***

>AbstractChannel.register(channel) ***<font color=#FF0000 >[入口]</font>***  
>this.evenLoop = evenLoop ***<font color=#FF0000 >[绑定线程]</font>***   
>register0()  ***<font color=#FF0000 >[实际注册]</font>***  
>>doRegister(false)  ***<font color=#FF0000 >[JDK 底层注册]</font>***  
>>invokeHandlerAddedIfNedded()
>>fireChannelRegistered() ***<font color=#FF0000 >[传播事件]</font>***   

AbstractChannel.java
---- 
```

```


注：以 NioEventLoopGroup 为例

到 AbstractChannel 中的 register() 方法调用链十分漫长
1. 起始在初始化阶段调用 ServerBootstrap.group()方法，将 NioEventLoopGroup 传入。
2. ServerBootstrap的group方法中，将 boosGroup 作为参数传递到其父类 AbstractBootstrap的group()方法中，保存至 EventLoopGroup group 属性。
3. 调用 ServerBootstrap 的 bind 方法进行启动，进入 AbstractBootstrap 的 initAndRegister方法（初始化并注册），该方法内执行了以下语句进行配置

    ```
    ChannelFuture regFuture = config().group().register(channel);
    ```
    1. config() 调用了 ServerBootstrap 中的 config() 方法返回一个通过当前对象构建的ServerBootstrapConfig。
        ```
         private final ServerBootstrapConfig config = new ServerBootstrapConfig(this);
        ```
    2. group() 调用的是 ServerBootstrapConfig 从父类 AbstractBootstrapConfig 中继承的 group 方法
        ```
        public final EventLoopGroup group() {
            return bootstrap.group();
        }
        ```
    3. 该 bootstrap 本质上是 ServerBootstrap，调用的 group() 也是 ServerBootstrap 从 AbstractBootstrap 中的 group() 方法
        ```
        public final EventLoopGroup group() {
            return group;
        }
        ```
    4. 该 group 属性 也就是 在之前起始阶段配置的 NioEventLoopGroup 。
    5. 接下来调用的 register()方法也就是 NioEventLoopGroup 从父类 MultithreadEventLoopGroup 中继承的 register() 方法。
        ```
        @Override
        public ChannelFuture register(Channel channel) {
            return next().register(channel);
        }
        ```
    6. 在 MultithreadEventLoopGroup 执行的 next() 方法 如下
        ```
        @Override
        public EventLoop next() {
            return (EventLoop) super.next();
        }
        ```
    7. super.next() 执行的 next()在 MultithreadEventLoopGroup的父类 MultithreadEventExecutorGroup 中，方法是从 chooser[EventExecutorChooserFactory.EventExecutorChooser] 中取出下一个 EventExecutor 对象。代码如下：
        ```
        @Override
        public EventExecutor next() {
            return chooser.next();
        }
        ```
    8. 该 chooser 是 MultithreadEventExecutorGroup 构造方法中进行初始化的。初始化是由 NioEventLoopGroup 的构造器中执行 MultithreadEventLoopGroup 的构造器，而 MultithreadEventLoopGroup 的构造器又执行了 MultithreadEventExecutorGroup 的构造器。该构造方法代码如下：
        ```
        protected MultithreadEventExecutorGroup(int nThreads, Executor executor,
                    EventExecutorChooserFactory chooserFactory, Object... args) {
        ···

        children = new EventExecutor[nThreads];

        for (int i = 0; i < nThreads; i ++) {
            boolean success = false;
            try {
                children[i] = newChild(executor, args);
                success = true;
            } catch (Exception e) {
                // TODO: Think about if this is a good exception type
                throw new IllegalStateException("failed to create a child event loop", e);
            } finally {
                ···
            }
        }
        chooser = chooserFactory.newChooser(children);
        ···
        }
        ```
    9. chooser 是 chooserFactory.newChooser(children) 语句进行初始化，而 children 是在 EventExecutor 事件执行器对象 ，内容是由 newChild(executor, args) 进行填充。这里的 newChild 方法，本质上就是 NioEventLoopGroup 的 newChild 方法，该方法返回了一个由该组构建的 NioEventLoop 实例，并向上转型为 EventLoop。
    10. 最终 config().group().register(channel) 代码执行的 register 方法是 SingleThreadEventLoop 的 register 方法。代码如下：
        ```
        @Override
        public ChannelFuture register(Channel channel) {
            return register(new DefaultChannelPromise(channel, this));
        }

        @Override
        public ChannelFuture register(final ChannelPromise promise) {
            ObjectUtil.checkNotNull(promise, "promise");
            promise.channel().unsafe().register(this, promise);
            return promise;
        }
        ```
    11. 启动 promise.channel().unsafe().register(this, promise) 语句的 register 执行的是 AbstractChannel 的 register 方法，也就是最后交由 AbstractChannel 进行对 evenloop 的管理，也就是做 selector 的注册过程



