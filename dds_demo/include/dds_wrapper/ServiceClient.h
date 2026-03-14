#ifndef DDS_WRAPPER_SERVICE_CLIENT_H
#define DDS_WRAPPER_SERVICE_CLIENT_H

#include "Publisher.h"
#include "Subscriber.h"
#include "Logger.h"
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <iomanip>
#include <atomic>

namespace dds_wrapper
{

// ServiceClient implements the client side of a ROS-style service.
//
// Publishes Req messages on "<service_name>/request" and listens for
// Resp messages on "<service_name>/response".  Each outgoing request
// receives a unique request_id so that the matching response can be
// identified even when multiple clients share the same topic.
//
// Synchronous usage (blocks until response or timeout):
//   auto result = client->call(req);
//
// Asynchronous usage (returns immediately):
//   client->callAsync(req, [](const Resp& resp){ ... });
//
// IDL convention (required by the template code):
//   - Req  must expose  request_id(const std::string&)  and  client_id(const std::string&)
//   - Resp must expose  request_id() const
//
// Do not construct directly; use DDSManager::createServiceClient().
template<typename Req, typename Resp>
class ServiceClient : public std::enable_shared_from_this<ServiceClient<Req, Resp>>
{
public:
    using ResponseCallback = std::function<void(const Resp&)>;

    // Factory method used internally by DDSManager.
    static std::shared_ptr<ServiceClient<Req, Resp>> create(
        const std::string& service_name,
        std::shared_ptr<Publisher<Req>> pub
    )
    {
        return std::shared_ptr<ServiceClient<Req, Resp>>(
            new ServiceClient<Req, Resp>(service_name, pub)
        );
    }

    ~ServiceClient() = default;

    ServiceClient(const ServiceClient&) = delete;
    ServiceClient& operator=(const ServiceClient&) = delete;
    ServiceClient(ServiceClient&&) = delete;
    ServiceClient& operator=(ServiceClient&&) = delete;

    // Called by DDSManager once to inject the response Subscriber.
    void setSubscriber(std::shared_ptr<Subscriber<Resp>> sub)
    {
        sub_ = sub;
    }

    // True when the server has matched the request topic.
    bool isServerReady() const noexcept
    {
        return pub_ && pub_->isConnected();
    }

    const std::string& getServiceName() const noexcept
    {
        return service_name_;
    }

    const std::string& getClientId() const noexcept
    {
        return client_id_;
    }

    // Synchronous call: blocks until the server responds or the timeout expires.
    // Returns the response, or std::nullopt on timeout.
    std::optional<Resp> call(
        Req& request,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)
    )
    {
        std::string req_id = generateRequestId();
        request.request_id(req_id);
        request.client_id(client_id_);

        // Register a promise before publishing to avoid a race where the
        // response arrives before the promise entry exists.
        auto prom = std::make_shared<std::promise<Resp>>();
        std::future<Resp> fut = prom->get_future();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            pending_sync_[req_id] = prom;
        }

        Logger::getInstance().debug(
            "ServiceClient [" + service_name_ + "] sending sync request_id=" + req_id
        );

        if (!pub_->publish(request))
        {
            std::lock_guard<std::mutex> lock(mutex_);
            pending_sync_.erase(req_id);
            Logger::getInstance().error(
                "ServiceClient [" + service_name_ + "] failed to publish request"
            );
            return std::nullopt;
        }

        if (fut.wait_for(timeout) == std::future_status::ready)
        {
            total_calls_.fetch_add(1);
            return fut.get();
        }

        // Timed out — clean up pending entry.
        {
            std::lock_guard<std::mutex> lock(mutex_);
            pending_sync_.erase(req_id);
        }
        Logger::getInstance().warn(
            "ServiceClient [" + service_name_ + "] timeout waiting for request_id=" + req_id
        );
        timeout_calls_.fetch_add(1);
        return std::nullopt;
    }

    // Asynchronous call: publishes the request and returns immediately.
    // The callback is invoked (from the subscriber thread) when the response arrives.
    void callAsync(Req& request, ResponseCallback callback)
    {
        std::string req_id = generateRequestId();
        request.request_id(req_id);
        request.client_id(client_id_);

        {
            std::lock_guard<std::mutex> lock(mutex_);
            pending_async_[req_id] = std::move(callback);
        }

        Logger::getInstance().debug(
            "ServiceClient [" + service_name_ + "] sending async request_id=" + req_id
        );

        if (!pub_->publish(request))
        {
            std::lock_guard<std::mutex> lock(mutex_);
            pending_async_.erase(req_id);
            Logger::getInstance().error(
                "ServiceClient [" + service_name_ + "] failed to publish async request"
            );
        }
    }

    // Called by the internal Subscriber callback whenever a response arrives.
    // Routes the response to the matching pending promise or async callback.
    void onResponse(const Resp& resp)
    {
        std::string req_id = resp.request_id();

        std::shared_ptr<std::promise<Resp>> prom;
        ResponseCallback async_cb;

        {
            std::lock_guard<std::mutex> lock(mutex_);

            auto sync_it = pending_sync_.find(req_id);
            if (sync_it != pending_sync_.end())
            {
                prom = sync_it->second;
                pending_sync_.erase(sync_it);
            }
            else
            {
                auto async_it = pending_async_.find(req_id);
                if (async_it != pending_async_.end())
                {
                    async_cb = std::move(async_it->second);
                    pending_async_.erase(async_it);
                }
            }
        }

        if (prom)
        {
            total_calls_.fetch_add(1);
            prom->set_value(resp);
            Logger::getInstance().debug(
                "ServiceClient [" + service_name_ + "] sync response received request_id=" + req_id
            );
        }
        else if (async_cb)
        {
            total_calls_.fetch_add(1);
            try
            {
                async_cb(resp);
            }
            catch (const std::exception& e)
            {
                Logger::getInstance().error(
                    "ServiceClient [" + service_name_ + "] async callback exception: " + e.what()
                );
            }
            catch (...)
            {
                Logger::getInstance().error(
                    "ServiceClient [" + service_name_ + "] async callback unknown exception"
                );
            }
            Logger::getInstance().debug(
                "ServiceClient [" + service_name_ + "] async response handled request_id=" + req_id
            );
        }
        // If neither map has the request_id, the response belongs to another client — ignore.
    }

    uint64_t getTotalCalls() const noexcept
    {
        return total_calls_.load();
    }

    uint64_t getTimeoutCalls() const noexcept
    {
        return timeout_calls_.load();
    }

private:
    ServiceClient(
        const std::string& service_name,
        std::shared_ptr<Publisher<Req>> pub
    )
        : service_name_(service_name)
        , pub_(pub)
        , client_id_(generateClientId())
        , total_calls_(0)
        , timeout_calls_(0)
    {
        Logger::getInstance().info(
            "ServiceClient created for service: " + service_name_ +
            " client_id=" + client_id_
        );
    }

    static std::string generateHex64()
    {
        static std::mt19937_64 rng(
            std::random_device{}() ^
            static_cast<uint64_t>(
                std::chrono::steady_clock::now().time_since_epoch().count()
            )
        );
        static std::mutex rng_mutex;

        std::lock_guard<std::mutex> lock(rng_mutex);
        std::ostringstream oss;
        oss << std::hex << std::setfill('0') << std::setw(16) << rng();
        return oss.str();
    }

    static std::string generateRequestId()
    {
        return generateHex64();
    }

    static std::string generateClientId()
    {
        return "client_" + generateHex64();
    }

    std::string service_name_;
    std::shared_ptr<Publisher<Req>> pub_;
    std::shared_ptr<Subscriber<Resp>> sub_;
    std::string client_id_;

    std::mutex mutex_;
    std::map<std::string, std::shared_ptr<std::promise<Resp>>> pending_sync_;
    std::map<std::string, ResponseCallback> pending_async_;

    std::atomic<uint64_t> total_calls_;
    std::atomic<uint64_t> timeout_calls_;
};

} // namespace dds_wrapper

#endif // DDS_WRAPPER_SERVICE_CLIENT_H
