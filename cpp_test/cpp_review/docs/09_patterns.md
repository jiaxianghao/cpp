# 设计模式：识别变化，再选择结构

模式是反复出现的设计问题的一种协作结构，不是必须集齐的类名。读每个模式时先回答：变化发生在哪里？谁持有谁？通过什么接口替换？额外间接层值得吗？

## 五个可执行实验

运行 `./build/09_patterns`，对应 [源码](../examples/09_patterns.cpp)。每个例子故意缩小业务，只保留角色关系。

|模式|这里的变化点|代码中的角色|代价与不用它的时机|
|---|---|---|---|
|Strategy 策略|计算折扣的算法|quote 是调用方，lambda 是策略|单一规则直接函数即可；std::function 有类型擦除成本|
|Factory Method 工厂方法|框架流程创建哪种发送器|Application 是 Creator，MailApplication 覆盖创建方法，Sender 是 Product|没有可复用框架流程时，普通工厂函数更简单|
|Adapter 适配器|旧接口与新接口不兼容|PrinterAdapter 把 LegacyPrinter 包装成 Sender|双方本来同接口就不需要适配|
|Decorator 装饰器|保持接口同时叠加行为|LoggingSender 拥有一个 Sender|层级多会增加调试与所有权理解成本|
|Observer 观察者|一个事件通知多个接收方|Events 持有回调列表|通知顺序、退订、回调寿命与异常传播需要契约|

源码中装饰器返回带 log 前缀的结果，用于表现包装顺序，并非真实日志系统。Factory Method 采用真正的覆写创建方法，普通 `switch -> make_unique` 函数则是简单工厂，二者不要混为一谈。

Observer 示例限定同步通知、只注册、通知期间不修改订阅列表。回调抛出时会停止后续通知并向外传播。引用捕获的 sum 比 events 活得久。只有当业务需要长期订阅、动态退订、异步通知时，才补充相应生命周期机制。

## 最容易混淆的模式

- Strategy 与 State：两者都可能通过委托改变行为。Strategy 通常由调用者选择算法；State 围绕对象当前状态以及状态转换组织行为。只因存在 if/else 就改成 State 通常没有必要。
- Adapter 与 Decorator：Adapter 解决接口不兼容；Decorator 保持接口并叠加职责。
- Factory Method 与 Abstract Factory：前者是可覆写的创建步骤；后者提供一组相关产品的创建接口。
- Template Method 与 Strategy：前者通过继承固定算法骨架、开放步骤；后者通过组合替换算法。运行时组合需求明确时，Strategy 往往更灵活。
- Observer 与发布订阅：常见发布订阅系统会通过中介或消息通道解耦双方；本例是对象内同步回调，不假定消息持久化、跨进程或异步能力。

## 完整分类地图（用于后续检索，不要求本轮背诵）

|类别|模式|
|---|---|
|创建型|Abstract Factory、Builder、Factory Method、Prototype、Singleton|
|结构型|Adapter、Bridge、Composite、Decorator、Facade、Flyweight、Proxy|
|行为型|Chain of Responsibility、Command、Interpreter、Iterator、Mediator、Memento、Observer、State、Strategy、Template Method、Visitor|

Singleton 会隐藏依赖和共享状态，不能因为“系统里目前只有一个”就默认采用。Visitor 适合类型集合稳定但外部操作扩展较多的情况；新增元素类型可能需要修改所有访问者。Bridge 将两个需要独立变化的方向分离，不能为了演示模式强行创造第二个变化方向。

## 先预测与动手练习

1. quote(100, 九折策略) 返回多少？是否需要继承才能实现 Strategy？
2. 再包一层 LoggingSender 会返回什么？谁负责释放内部对象？
3. 两个观察者分别累计 n 和 2n，发布 3 后结果是多少？如果第一个抛出，第二个是否执行？
4. 增加 SmsSender 与 SmsApplication，验证 Application::notify 不需要修改。
5. 增加一个“满额减固定金额”的 lambda 策略，验证无需改变 quote。
6. 用两段直接函数调用替换一个你认为多余的模式，解释简化后的收益与失去的扩展能力。

过关标准：不看类名也能识别五个模式的角色；能指出当前综合项目只需要 Strategy 的理由；不把设计模式数量当作设计质量。
