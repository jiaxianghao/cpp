#include "check.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
// Strategy：只有一个操作时，函数对象足够，不必创建类层次。
int quote(int subtotal, const std::function<int(int)>& discount) { return discount(subtotal); }
struct Sender { virtual ~Sender() = default; virtual std::string send() const = 0; };
struct Email : Sender { std::string send() const override { return "email"; } };
// Factory Method：由子类决定创建哪个 Product。它不同于单个 switch 工厂函数。
struct Application {
    virtual ~Application() = default;
    virtual std::unique_ptr<Sender> create_sender() const = 0;
    std::string notify() const { return create_sender()->send(); }
};
struct MailApplication : Application {
    std::unique_ptr<Sender> create_sender() const override { return std::make_unique<Email>(); }
};
struct LegacyPrinter { std::string print_text() const { return "legacy"; } };
struct PrinterAdapter : Sender {
    LegacyPrinter printer;
    std::string send() const override { return printer.print_text(); }
};
struct LoggingSender : Sender {
    std::unique_ptr<Sender> inner;
    explicit LoggingSender(std::unique_ptr<Sender> s) : inner(std::move(s)) {}
    std::string send() const override { return "log:" + inner->send(); }
};
// 同步、仅注册的最小 Observer；回调执行期间不可修改订阅列表。
class Events {
    std::vector<std::function<void(int)>> handlers;
public:
    void subscribe(std::function<void(int)> h) { handlers.push_back(std::move(h)); }
    void publish(int value) const { for (const auto& h : handlers) h(value); }
};
int main() {
    CHECK(quote(100, [](int n) { return n * 9 / 10; }) == 90);
    CHECK(MailApplication{}.notify() == "email");
    CHECK(PrinterAdapter{}.send() == "legacy");
    LoggingSender decorated(std::make_unique<Email>());
    CHECK(decorated.send() == "log:email");
    int sum = 0;
    Events events; // 后构造，先析构；引用捕获的 sum 比 events 活得久。
    events.subscribe([&sum](int n) { sum += n; });
    events.subscribe([&sum](int n) { sum += n * 2; });
    events.publish(3);
    CHECK(sum == 9);
    std::cout << "Strategy / Factory Method / Adapter / Decorator / Observer passed\n";
}
