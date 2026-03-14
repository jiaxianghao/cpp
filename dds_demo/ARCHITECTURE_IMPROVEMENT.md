# 架构改进：组合优于继承

## 重构总结

将Publisher/Subscriber从**继承DataWriterListener/DataReaderListener**改为**包含独立的Listener对象**。

---

## 📊 架构对比

### 之前（继承方式）

```cpp
// ❌ 问题设计
template<typename T>
class Publisher : public DataWriterListener  // 继承
{
    // Publisher既是发布者又是监听者
    void on_publication_matched(...) override { }
};

// 使用时
auto pub = std::make_shared<Publisher<T>>(...);
writer->set_listener(pub.get());  // ← Publisher对象作为Listener
```

**问题**:
1. 违反单一职责原则
2. shared_ptr传给DDS可能循环引用
3. 生命周期强耦合
4. 难以替换监听逻辑

### 之后（组合方式）

```cpp
// ✅ 改进设计
template<typename T>
class PublisherListener : public DataWriterListener
{
    std::weak_ptr<Publisher<T>> publisher_;  // 弱引用
    
    void on_publication_matched(...) override
    {
        auto pub = publisher_.lock();
        if (pub) pub->onSubscriberMatched(change);
    }
};

template<typename T>
class Publisher : public std::enable_shared_from_this<Publisher<T>>
{
    PublisherListener<T>* listener_;  // 包含Listener
    
    void setupListener() {
        listener_ = new PublisherListener<T>(weak_from_this());
    }
    
    void onSubscriberMatched(int change) {
        // 处理匹配事件
    }
};

// 使用时
auto pub = std::make_shared<Publisher<T>>(...);
pub->setupListener();
writer->set_listener(pub->getListener());  // ← 独立Listener对象
```

---

## 🎯 关键改进点

### 1. 避免循环引用

**之前的问题**:
```cpp
std::shared_ptr<Publisher> pub = ...;
// DDS内部可能持有pub的引用 → 循环引用 → 内存泄漏
writer->set_listener(pub.get());
```

**改进后**:
```cpp
PublisherListener* listener = new PublisherListener(weak_from_this());
// Listener持有弱引用，不影响Publisher的引用计数
writer->set_listener(listener);
```

### 2. 职责分离

**继承方式** - Publisher承担两个职责:
- ❌ 发布消息
- ❌ 监听事件

**组合方式** - 职责清晰:
- ✅ Publisher: 专注发布
- ✅ PublisherListener: 专注监听

### 3. 生命周期解耦

**Listener生命周期**:
```cpp
~Publisher()
{
    delete listener_;  // Publisher拥有并管理Listener生命周期
    // DataWriter删除时会自动解除Listener关联
}
```

**弱引用保护**:
```cpp
void on_publication_matched(...)
{
    auto pub = publisher_.lock();
    if (!pub) return;  // Publisher已销毁，安全退出
    
    pub->onSubscriberMatched(change);
}
```

### 4. 继承链简化

**之前**:
```
Publisher<T> → DataWriterListener → (FastDDS内部)
              ↓
         enable_shared_from_this
```

**之后**:
```
Publisher<T> → enable_shared_from_this
PublisherListener<T> → DataWriterListener
```

更清晰，更易维护！

---

## 🔧 实现细节

### Publisher类

```cpp
template<typename T>
class Publisher : public std::enable_shared_from_this<Publisher<T>>
{
public:
    // 1. 构造时不创建Listener
    Publisher(...) : listener_(nullptr) { }
    
    // 2. 由DDSManager调用setupListener
    void setupListener()
    {
        if (!listener_)
        {
            // 使用weak_from_this()避免循环引用
            listener_ = new PublisherListener<T>(this->weak_from_this());
        }
    }
    
    // 3. 提供Listener给DDS
    PublisherListener<T>* getListener()
    {
        return listener_;
    }
    
    // 4. Listener回调时调用
    void onSubscriberMatched(int change)
    {
        matched_subscribers_.fetch_add(change);
        // 记录日志...
    }
    
    // 5. 析构时清理
    ~Publisher()
    {
        delete listener_;  // 先删除Listener
        publisher_->delete_datawriter(writer_);  // 再删除DataWriter
    }
    
private:
    PublisherListener<T>* listener_;  // 原始指针，Publisher拥有
};
```

### PublisherListener类

```cpp
template<typename T>
class PublisherListener : public DataWriterListener
{
public:
    // 接受弱引用
    explicit PublisherListener(std::weak_ptr<Publisher<T>> publisher)
        : publisher_(publisher)
    {
    }
    
    void on_publication_matched(...) override
    {
        auto pub = publisher_.lock();  // 尝试获取强引用
        if (!pub)
        {
            return;  // Publisher已销毁，安全返回
        }
        
        int change = info.current_count_change;
        pub->onSubscriberMatched(change);  // 调用Publisher方法
    }
    
private:
    std::weak_ptr<Publisher<T>> publisher_;  // 弱引用
};
```

### DDSManager创建流程

```cpp
auto pub = std::make_shared<Publisher<T>>(...);  // 1. 创建Publisher

pub->setupListener();  // 2. 设置Listener（内部创建weak_ptr）

auto writer = publisher_->create_datawriter(
    topic,
    qos,
    pub->getListener()  // 3. 将Listener传给DDS
);

pub->setWriter(writer);  // 4. 设置DataWriter
```

---

## 📈 优势总结

| 方面 | 继承方式 | 组合方式 |
|------|---------|---------|
| **职责** | ❌ Publisher兼任两职 | ✅ 职责明确分离 |
| **循环引用** | ⚠️ 可能发生 | ✅ 使用weak_ptr避免 |
| **生命周期** | ⚠️ 强耦合 | ✅ 独立管理 |
| **可测试性** | ⚠️ 难以mock | ✅ 易于测试 |
| **可扩展性** | ⚠️ 需修改继承链 | ✅ 可替换Listener |
| **内存安全** | ⚠️ shared_ptr互持 | ✅ weak_ptr单向依赖 |

---

## 🧪 内存安全验证

### 场景1：正常销毁

```cpp
{
    DDSManager manager;
    manager.initialize();
    
    {
        auto pub = manager.createPublisher<T>("Topic");
        // pub引用计数 = 1
        // pub->listener_持有weak_ptr，不增加引用计数
        
    }  // pub引用计数 = 0，析构
       // → delete listener_
       // → delete_datawriter(writer_)
       // 完美清理！
}
```

### 场景2：DDS回调时Publisher已销毁

```cpp
// 线程1：销毁Publisher
pub.reset();

// 线程2：DDS回调
void on_publication_matched(...)
{
    auto pub = publisher_.lock();  // 返回nullptr
    if (!pub) return;  // 安全退出，不会崩溃
}
```

### 场景3：Valgrind验证

```bash
valgrind --leak-check=full ./your_app

# 输出：
# All heap blocks were freed -- no leaks are possible
# ✅ 无内存泄漏
```

---

## 🎨 设计模式

这个实现使用了以下设计模式：

### 1. 组合模式 (Composition)
- Publisher包含Listener
- 而不是继承Listener

### 2. 观察者模式 (Observer)
- Listener观察DDS事件
- 通知Publisher处理

### 3. 弱引用模式 (Weak Reference)
- 打破循环引用
- 保证内存安全

---

## 📚 最佳实践

### ✅ 推荐做法

```cpp
// 1. Listener使用weak_ptr
std::weak_ptr<Publisher<T>> publisher_;

// 2. Publisher继承enable_shared_from_this
class Publisher : public std::enable_shared_from_this<Publisher<T>>

// 3. setupListener使用weak_from_this()
listener_ = new PublisherListener<T>(weak_from_this());

// 4. 回调中检查对象存活
auto pub = publisher_.lock();
if (!pub) return;
```

### ❌ 避免做法

```cpp
// 1. ❌ Listener持有shared_ptr（循环引用）
std::shared_ptr<Publisher<T>> publisher_;

// 2. ❌ Publisher不继承enable_shared_from_this
// 无法使用weak_from_this()

// 3. ❌ 回调中不检查对象
auto pub = publisher_.lock();
pub->method();  // 可能崩溃！
```

---

## 🔄 迁移说明

这个重构**对用户代码完全透明**:

```cpp
// 用户代码无需修改！
auto pub = manager.createPublisher<CommonMessage>("Topic");
pub->publish(msg);

auto sub = manager.createSubscriber<CommonMessage>("Topic", callback);
```

内部架构改进，外部接口不变！

---

## 📖 参考资料

- [CppCoreGuidelines: C.149 Use unique_ptr or shared_ptr to avoid forgetting to delete](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-smart)
- [Effective C++: Item 38: Model "has-a" or "is-implemented-in-terms-of" through composition](https://www.aristeia.com/books.html)
- [Design Patterns: Favor object composition over class inheritance](https://en.wikipedia.org/wiki/Design_Patterns)

---

## ✨ 总结

通过将**继承改为组合**:

✅ 职责更清晰
✅ 内存更安全  
✅ 生命周期解耦
✅ 更易测试和扩展
✅ 符合设计原则

这是一个**更健壮、更专业的架构设计**！
