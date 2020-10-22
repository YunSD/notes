# ***NioEventLoop 概述***

三个问题
----
---
>- 默认情况下，Netty 服务端启多个线程？何时启动？

>- Netty 是如何解决 JDK 空轮询 BUG?

>- Netty 是如何保证异步串行无锁化?

NioEventLoop 学习流程
----
---
>- NioEventLoop 创建

>- NioEventLoop 启动

>- NioEventLoop 执行逻辑

---
---
---

# NioEventLoop 创建
----
---
>`new NioEventLoopGroup()` **<font color=#FF0000 >[ 线程组，默认 2 * CPU ] </font>**

>>`new ThreadPerTaskExecutor()` **<font color=#FF0000 >[ 线程执行器 ] </font>**

>>`for(){ newChild() }` **<font color=#FF0000 >[ 构造 NioEventLoop ] </font>**

>>`chooserFactory.newChooser()` **<font color=#FF0000 >[ 线程选择器 ] </font>**

---
`EventLoopGroup bossGroup = new NioEventLoopGroup(1);`

MultithreadEventLoopGroup.java
---
>构造方法：
>`DEFAULT_EVENT_LOOP_THREADS` 默认初始线程数为 处理器个数 × 2
```java
    private static final int DEFAULT_EVENT_LOOP_THREADS;

    static {
        DEFAULT_EVENT_LOOP_THREADS = Math.max(1, SystemPropertyUtil.getInt(
                "io.netty.eventLoopThreads", Runtime.getRuntime().availableProcessors() * 2));

        if (logger.isDebugEnabled()) {
            logger.debug("-Dio.netty.eventLoopThreads: {}", DEFAULT_EVENT_LOOP_THREADS);
        }
    }

    /**
     * @see {@link MultithreadEventExecutorGroup#MultithreadEventExecutorGroup(int, Executor, Object...)}
     */
    protected MultithreadEventLoopGroup(int nThreads, Executor executor, Object... args) {
        super(nThreads == 0 ? DEFAULT_EVENT_LOOP_THREADS : nThreads, executor, args);
    }

```
MultithreadEventExecutorGroup.java
---
>`DefaultEventExecutorChooserFactory.INSTANCE` 是创建的 线程选择器

>`MultithreadEventExecutorGroup` 作为 `MultithreadEventLoopGroup` 的父类。

>1. 创建线程选择器

>> ``` executor = new ThreadPerTaskExecutor(newDefaultThreadFactory()); ```

>2. 通过 for 循环创建 NioEventLoop

>3. 创建线程选择器

```java
    public static final DefaultEventExecutorChooserFactory INSTANCE = new DefaultEventExecutorChooserFactory();

    protected MultithreadEventExecutorGroup(int nThreads, Executor executor, Object... args) {
        this(nThreads, executor, DefaultEventExecutorChooserFactory.INSTANCE, args);
    }

    protected MultithreadEventExecutorGroup(int nThreads, Executor executor, EventExecutorChooserFactory  chooserFactory, Object... args) {
        if (nThreads <= 0) {
            throw new IllegalArgumentException(String.format("nThreads: %d (expected: > 0)", nThreads));
        }

        if (executor == null) {
            executor = new ThreadPerTaskExecutor(newDefaultThreadFactory());
        }

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
                if (!success) {
                    for (int j = 0; j < i; j ++) {
                        children[j].shutdownGracefully();
                    }

                    for (int j = 0; j < i; j ++) {
                        EventExecutor e = children[j];
                        try {
                            while (!e.isTerminated()) {
                                e.awaitTermination(Integer.MAX_VALUE, TimeUnit.SECONDS);
                            }
                        } catch (InterruptedException interrupted) {
                            // Let the caller handle the interruption.
                            Thread.currentThread().interrupt();
                            break;
                        }
                    }
                }
            }
        }

        chooser = chooserFactory.newChooser(children);

        final FutureListener<Object> terminationListener = new FutureListener<Object>() {
            @Override
            public void operationComplete(Future<Object> future) throws Exception {
                if (terminatedChildren.incrementAndGet() == children.length) {
                    terminationFuture.setSuccess(null);
                }
            }
        };

        for (EventExecutor e: children) {
            e.terminationFuture().addListener(terminationListener);
        }

        Set<EventExecutor> childrenSet = new LinkedHashSet<EventExecutor>(children.length);
        Collections.addAll(childrenSet, children);
        readonlyChildren = Collections.unmodifiableSet(childrenSet);
    }
```

ThreadperTaskExecutor.java
---
>- 每次执行任务都会创建一个线程实体

```java
    public final class ThreadPerTaskExecutor implements Executor {
        private final ThreadFactory threadFactory;

        public ThreadPerTaskExecutor(ThreadFactory threadFactory) {
            if (threadFactory == null) {
                throw new NullPointerException("threadFactory");
            }
            this.threadFactory = threadFactory;
        }

        @Override
        public void execute(Runnable command) {
            threadFactory.newThread(command).start();
        }
    }
```



DefaultThreadFactory.java
---
>- NioEventLoop 线程命名规则为 nioEventLoopGroup-1(group number)-xx(thread number)

```java
    public DefaultThreadFactory(Class<?> poolType, boolean daemon, int priority) {
        this(toPoolName(poolType), daemon, priority);
    }

    public static String toPoolName(Class<?> poolType) {
        if (poolType == null) {
            throw new NullPointerException("poolType");
        }

        String poolName = StringUtil.simpleClassName(poolType);
        switch (poolName.length()) {
            case 0:
                return "unknown";
            case 1:
                return poolName.toLowerCase(Locale.US);
            default:
                if (Character.isUpperCase(poolName.charAt(0)) && Character.isLowerCase(poolName.charAt(1))) {
                    return Character.toLowerCase(poolName.charAt(0)) + poolName.substring(1);
                } else {
                    return poolName;
                }
        }
    }

    @Override
    public Thread newThread(Runnable r) {
        Thread t = newThread(new DefaultRunnableDecorator(r), prefix + nextId.incrementAndGet());
        try {
            if (t.isDaemon()) {
                if (!daemon) {
                    t.setDaemon(false);
                }
            } else {
                if (daemon) {
                    t.setDaemon(true);
                }
            }

            if (t.getPriority() != priority) {
                t.setPriority(priority);
            }
        } catch (Exception ignored) {
            // Doesn't matter even if failed to set.
        }
        return t;
    }

    //FastThreadLocalThread 是 netty 底层包装的 thread,做了一些优化，另外包装了一个 threadMap
    protected Thread newThread(Runnable r, String name) {
        return new FastThreadLocalThread(threadGroup, r, name);
    }
```

newchild()
---
>- 保存线程执行器 ThreadPerTaskExecutor
>- 创建 MpscQueue
>- 创建 selector

查看 `MultithreadEventExecutorGroup.java` 中的 `newChild(executor, args)` 语句

```java
    @Override
    protected EventLoop newChild(Executor executor, Object... args) throws Exception {
        return new NioEventLoop(this, executor, (SelectorProvider) args[0],
            ((SelectStrategyFactory) args[1]).newSelectStrategy(), (RejectedExecutionHandler) args[2]);
    }

    // NioEventLoop构造器
    NioEventLoop(NioEventLoopGroup parent, Executor executor, SelectorProvider selectorProvider,SelectStrategy strategy, RejectedExecutionHandler rejectedExecutionHandler) {
        // 调用父类的构造器
        super(parent, executor, false, DEFAULT_MAX_PENDING_TASKS, rejectedExecutionHandler);
        if (selectorProvider == null) {
            throw new NullPointerException("selectorProvider");
        }
        if (strategy == null) {
            throw new NullPointerException("selectStrategy");
        }
        provider = selectorProvider;
        // 每一个 selector 都跟一个 NioEventLoop 做唯一的绑定
        selector = openSelector();
        selectStrategy = strategy;
    }

    // NioEventLoop 的父类 SingleThreadEventExecutor 构造器
    protected SingleThreadEventExecutor(EventExecutorGroup parent, Executor executor,
                                        boolean addTaskWakesUp, int maxPendingTasks,
                                        RejectedExecutionHandler rejectedHandler) {
        super(parent);
        this.addTaskWakesUp = addTaskWakesUp;
        this.maxPendingTasks = Math.max(16, maxPendingTasks);
        // 保存线程执行器
        this.executor = ObjectUtil.checkNotNull(executor, "executor");
        // taskQueue 当外部线程执行 netty 任务时，如果判断不是 NioEventLoop 的线程，将放置在 taskQueue 中等待它自身的线程执行。
        taskQueue = newTaskQueue(this.maxPendingTasks);
        rejectedExecutionHandler = ObjectUtil.checkNotNull(rejectedHandler, "rejectedHandler");
    }

    // MpscQueue
    @Override
    protected Queue<Runnable> newTaskQueue(int maxPendingTasks) {
        // This event loop never calls takeTask()
        return PlatformDependent.newMpscQueue(maxPendingTasks);
    }
```

创建线程选择器
---
`chooser = chooserFactory.newChooser(children)`

>给新连接绑定对应的 NioEventLoop,为循环添加.

```java
    public EventExecutorChooser newChooser(EventExecutor[] executors) {
        if (isPowerOfTwo(executors.length)) {
            return new PowerOfTowEventExecutorChooser(executors);
        } else {
            return new GenericEventExecutorChooser(executors);
        }
    }
```
> netty 执行优化，当指定线程数量是 2 的幂，使用`PowerOfTowEventExecutorChooser`。

>> `PowerOfTowEventExecutorChooser` 为 `(index ++) & (length -1)` [ 优化 ] (本质是二进制运算:2的幂-1,尾数为 ~111~，取&高效)

>> `GenericEventExecutorChooser` 为 `abs(index++ % length)` [ 普通 ]





<br>
<br>

# NioEventLoop 启动触发器
>- 服务端启动绑定端口
>- 新连接接入通过 chooser 绑定一个 NioEventLoop
----
---
>- NioEventLoop 启动

>>`bind() -> execute(task)`  **<font color=#FF0000 >[ 入口 ]</font>**  

>>>`startThread() -> doStartThread()` **<font color=#FF0000 >[ 创建线程 ]</font>**  

>>>>`ThreadPerTaskExecutor.execute()`

>>>>> `thread = Thread.currentThread()` (保存当前线程便于判断后续执行者是否是该线程本身，否则会被封装为 Task 置入 taskQueue)

>>>>> `NioEventLoop.run()` **<font color=#FF0000 >[ 启动 ]</font>**  

AbstractBootstrap.java
---
```java
    private static void doBind0(
            final ChannelFuture regFuture, final Channel channel,
            final SocketAddress localAddress, final ChannelPromise promise) {

        // This method is invoked before channelRegistered() is triggered.  Give user handlers a chance to set up
        // the pipeline in its channelRegistered() implementation.
        channel.eventLoop().execute(new Runnable() {
            @Override
            public void run() {
                if (regFuture.isSuccess()) {
                    channel.bind(localAddress, promise).addListener(ChannelFutureListener.CLOSE_ON_FAILURE);
                } else {
                    promise.setFailure(regFuture.cause());
                }
            }
        });
    }
```

SingleThreadEventExecutor.java
---
```java
    @Override
    public void execute(Runnable task) {
        if (task == null) {
            throw new NullPointerException("task");
        }
        // 判断当前执行的线程是否是 NioEventLoop 的线程
        boolean inEventLoop = inEventLoop();
        if (inEventLoop) {
            addTask(task);
        } else {
            startThread();
            addTask(task);
            if (isShutdown() && removeTask(task)) {
                reject();
            }
        }

        if (!addTaskWakesUp && wakesUpForTask(task)) {
            wakeup(inEventLoop);
        }
    }
```

<br>
<br>

# NioEventLoop 执行
---

>- `run() -> for(;;)`

>> `select()` **<font color=#FF0000 >[检查是否存在 IO 事件]</font>**  

>> `processSelectedKeys()` **<font color=#FF0000 >[处理 IO 事件]</font>**  

>> `runAllTask()`  **<font color=#FF0000 >[ 处理异步任务队列]</font>**  

NioEventLoop.java
----
```java
    @Override
    protected void run() {
        for (;;) {
            try {
                switch (selectStrategy.calculateStrategy(selectNowSupplier, hasTasks())) {
                    case SelectStrategy.CONTINUE:
                        continue;
                    case SelectStrategy.SELECT:
                        // 处理 IO 事件 
                        select(wakenUp.getAndSet(false));

                        if (wakenUp.get()) {
                            selector.wakeup();
                        }
                    default:
                        // fallthrough
                }

                cancelledKeys = 0;
                needsToSelectAgain = false;
                final int ioRatio = this.ioRatio;
                // 默认 50
                if (ioRatio == 100) {
                    try {
                        // 执行处理逻辑
                        processSelectedKeys();
                    } finally {
                        // Ensure we always run tasks.
                        runAllTasks();
                    }
                } else {
                    final long ioStartTime = System.nanoTime();
                    try {
                        processSelectedKeys();
                    } finally {
                        // Ensure we always run tasks.
                        final long ioTime = System.nanoTime() - ioStartTime;
                        runAllTasks(ioTime * (100 - ioRatio) / ioRatio);
                    }
                }
            } catch (Throwable t) {
                handleLoopException(t);
            }
            // Always handle shutdown even if the loop processing threw an exception.
            try {
                if (isShuttingDown()) {
                    closeAll();
                    if (confirmShutdown()) {
                        return;
                    }
                }
            } catch (Throwable t) {
                handleLoopException(t);
            }
        }
    }
```

select() 方法执行逻辑
----
>- deadline 以及任务穿插逻辑处理

>- 阻塞式 select

>- 避免 JDK 空轮询 BUG
```java
    switch (selectStrategy.calculateStrategy(selectNowSupplier, hasTasks())) {
        case SelectStrategy.CONTINUE:
            continue;
        case SelectStrategy.SELECT:
            // 处理 IO 事件 
            select(wakenUp.getAndSet(false));

            if (wakenUp.get()) {
                selector.wakeup();
            }
        default:
    }
```
每次执行 select 操作的时候，都会把 `wakenUp` 设置成 false。

select 函数

```java
    private void select(boolean oldWakenUp) throws IOException {
        Selector selector = this.selector;
        try {
            int selectCnt = 0;
            // 计算当前时间
            long currentTimeNanos = System.nanoTime();
            // 当前时间 + 截止时间 NioEventLoop 底层有一个定时队列 delayNanos计算当前任务队列第一个任务的截止时间
            // selectDeadLineNanos 当前执行操作的时间不能超过这个截止时间 
            long selectDeadLineNanos = currentTimeNanos + delayNanos(currentTimeNanos);
            for (;;) {
                // 判断当前有没有超时，如果当前超时并且一次也没有 seletc ,就进行一个非阻塞的 select 操作
                long timeoutMillis = (selectDeadLineNanos - currentTimeNanos + 500000L) / 1000000L;
                if (timeoutMillis <= 0) {
                    if (selectCnt == 0) {
                        selector.selectNow();
                        selectCnt = 1;
                    }
                    break;
                }
                // 未到截止时间，继续判断当前任务队列里有没有任务，也就是 taskQueue
                // 这个时候把 wakenUp 设置为 true
                if (hasTasks() && wakenUp.compareAndSet(false, true)) {
                    selector.selectNow();
                    selectCnt = 1;
                    break;
                }
                // 截止时间未到，并且任务队列为空，那么就进行一个阻塞式的select操作
                // timeoutMillis 本次 select 的最大时间，默认为 1s
                // select 阻塞式
                int selectedKeys = selector.select(timeoutMillis);
                // 每次阻塞后都会将这个 selectCnt ++ ，表示以及轮询 selectCnt 次
                selectCnt ++;

                // selectedKeys:轮询到事件
                // oldWakenUp：select 是否需要唤醒
                // wakenUp.get()：执行 select 操作的时候已经被外部线程唤醒
                // hasTasks：异步队列有任务
                // hasScheduledTasks：定时队列中有任务
                // 满足其上任一条件，都会中止 select 操作
                if (selectedKeys != 0 || oldWakenUp || wakenUp.get() || hasTasks() || hasScheduledTasks()) {
                    break;
                }
                if (Thread.interrupted()) {
                    selectCnt = 1;
                    break;
                }

                // 已经进行了一次阻塞式的 select 操作
                long time = System.nanoTime();
                // 这里是为了判断之前的 select 阻塞操作到底是否执行
                if (time - TimeUnit.MILLISECONDS.toNanos(timeoutMillis) >= currentTimeNanos) {
                    // 如果 确实阻塞，将 selectCnt 置为 1
                    selectCnt = 1;
                } else if (SELECTOR_AUTO_REBUILD_THRESHOLD > 0 && selectCnt >= SELECTOR_AUTO_REBUILD_THRESHOLD) {
                    // 发生的空轮询，并且次数超过了 SELECTOR_AUTO_REBUILD_THRESHOLD：512
                    // 调用 rebuildSelector ，避免下一次继续发生
                    // 逻辑：将老的 selectKeys 注册到新的 Selector 上面，那么在新的 Selector 上的阻塞操作就有可能不会发生空轮询
                    rebuildSelector();
                    selector = this.selector;

                    //重新执行 selectNow
                    selector.selectNow();
                    selectCnt = 1;
                    break;
                }
                currentTimeNanos = time;
            }

        } catch (CancelledKeyException e) {
        }
    }
```

```java
    public void rebuildSelector() {
        // 重新创建 Selector
        final Selector oldSelector = selector;
        final Selector newSelector;

        if (oldSelector == null) {
            return;
        }

        try {
            newSelector = openSelector();
        } catch (Exception e) {
            logger.warn("Failed to create a new Selector.", e);
            return;
        }

        int nChannels = 0;
        for (;;) {
            try {
                // 拿到 oldSelector 上面所有的 key 与 attachment
                for (SelectionKey key: oldSelector.keys()) {
                    Object a = key.attachment();
                    try {
                        if (!key.isValid() || key.channel().keyFor(newSelector) != null) {
                            continue;
                        }
                        // 拿到 key 的注册事件
                        int interestOps = key.interestOps();
                        // 取消原先 key 的事件
                        key.cancel();
                        // 注册到重新创建的新的 selector 上面，包括注册的事件以及 netty 重新封装的 attachment
                        SelectionKey newKey = key.channel().register(newSelector, interestOps, a);
                        if (a instanceof AbstractNioChannel) {
                            // Update SelectionKey 重新分配 channel 的 selectionKey
                            ((AbstractNioChannel) a).selectionKey = newKey;
                        }
                        nChannels ++;
                    } catch (Exception e) {
                        ...
                    }
                }
            } catch (ConcurrentModificationException e) {
                continue;
            }

            break;
        }

        selector = newSelector;
        ...
    }


```


`processSelectedKeys()` 执行逻辑
----
>- selected keySet 优化

>- `processSelectdKeysOptimized()`

NioEventLoop `openSelector()` 方法
```java
    private Selector openSelector() {
        final Selector selector;
        try {
            // 创建 jdk 底层 selector
            selector = provider.openSelector();
        } catch (IOException e) {
            throw new ChannelException("failed to open a new selector", e);
        }

        // 是否需要优化，默认需要优化
        if (DISABLE_KEYSET_OPTIMIZATION) {
            return selector;
        }

        // SelectedSelectionKeySet 看起来是个 Set，其实是由数组实现的
        final SelectedSelectionKeySet selectedKeySet = new SelectedSelectionKeySet();

        //  通过反射获取 SelectorImpl
        Object maybeSelectorImplClass = AccessController.doPrivileged(new PrivilegedAction<Object>() {
            @Override
            public Object run() {
                try {
                    return Class.forName(
                            "sun.nio.ch.SelectorImpl",
                            false,
                            PlatformDependent.getSystemClassLoader());
                } catch (ClassNotFoundException e) {
                    return e;
                } catch (SecurityException e) {
                    return e;
                }
            }
        });

        // 判断拿到的对象是否是 selector 的实现
        if (!(maybeSelectorImplClass instanceof Class) ||
                // ensure the current selector implementation is what we can instrument.
                !((Class<?>) maybeSelectorImplClass).isAssignableFrom(selector.getClass())) {
            if (maybeSelectorImplClass instanceof Exception) {
                Exception e = (Exception) maybeSelectorImplClass;
                logger.trace("failed to instrument a special java.util.Set into: {}", selector, e);
            }
            //如果不是这个类的实现，则会直接返回一个 selector
            return selector;
        }

        // 如果是
        final Class<?> selectorImplClass = (Class<?>) maybeSelectorImplClass;

        // 通过反射获取 maybeSelectorImplClass 中的 selectedKeys 与 publicSelectedKeys
        Object maybeException = AccessController.doPrivileged(new PrivilegedAction<Object>() {
            @Override
            public Object run() {
                try {
                    // 默认都是 HashSet 实现
                    Field selectedKeysField = selectorImplClass.getDeclaredField("selectedKeys");
                    Field publicSelectedKeysField = selectorImplClass.getDeclaredField("publicSelectedKeys");

                    selectedKeysField.setAccessible(true);
                    publicSelectedKeysField.setAccessible(true);

                    //精华点：将selector设置到优化后的keySet
                    selectedKeysField.set(selector, selectedKeySet);
                    publicSelectedKeysField.set(selector, selectedKeySet);
                    return null;
                } catch (NoSuchFieldException e) {
                    ...
                }
            }
        });

        if (maybeException instanceof Exception) {
            selectedKeys = null;
            Exception e = (Exception) maybeException;
            logger.trace("failed to instrument a special java.util.Set into: {}", selector, e);
        } else {
            //keySet保存
            selectedKeys = selectedKeySet;
            logger.trace("instrumented a special java.util.Set into: {}", selector);
        }

        return selector;
    }

```
NioEventLoop `processSelectedKeys()` 方法
---
```java
    private void processSelectedKeys() {
        if (selectedKeys != null) {
            // 默认情况下 selectedKeys 不为空，则进行优化后的处理
            // selectedKeys.flip() 将底层的数组返回
            processSelectedKeysOptimized(selectedKeys.flip());
        } else {
            processSelectedKeysPlain(selector.selectedKeys());
        }
    }

    private void processSelectedKeysOptimized(SelectionKey[] selectedKeys) {
        // 遍历 selectedKeys 数组 获取 SelectionKey
        for (int i = 0;; i ++) {
            final SelectionKey k = selectedKeys[i];
            if (k == null) {
                break;
            }
            // null out entry in the array to allow to have it GC'ed once the Channel close
            // 置空
            selectedKeys[i] = null;

            // 获取 attachment ，也就是 netty 封装的 AbstractChannel
            final Object a = k.attachment();

            if (a instanceof AbstractNioChannel) {
                // 判断 unsafe 的 key是否合法，合法的话会处理 channel 的连接、读、写、accept事件
                processSelectedKey(k, (AbstractNioChannel) a);
            } else {
                @SuppressWarnings("unchecked")
                NioTask<SelectableChannel> task = (NioTask<SelectableChannel>) a;
                processSelectedKey(k, task);
            }

            if (needsToSelectAgain) {
                // null out entries in the array to allow to have it GC'ed once the Channel close
                // See https://github.com/netty/netty/issues/2363
                for (;;) {
                    i++;
                    if (selectedKeys[i] == null) {
                        break;
                    }
                    selectedKeys[i] = null;
                }

                selectAgain();
                // Need to flip the optimized selectedKeys to get the right reference to the array
                // and reset the index to -1 which will then set to 0 on the for loop
                // to start over again.
                //
                // See https://github.com/netty/netty/issues/1523
                selectedKeys = this.selectedKeys.flip();
                i = -1;
            }
        }
    }
```
