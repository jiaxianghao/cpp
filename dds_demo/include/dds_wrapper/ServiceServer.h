#ifndef DDS_WRAPPER_SERVICE_SERVER_H
#define DDS_WRAPPER_SERVICE_SERVER_H

#include "Publisher.h"
#include "Subscriber.h"
#include "Logger.h"
#include <functional>
#include <memory>
#include <string>
#include <atomic>

namespace dds_wrapper
{

// ServiceServer implements the server side of a ROS-style service.
//
// It listens on topic  "<service_name>/request"  for incoming Req messages,
// calls the user-supplied Handler, then publishes the Resp back on topic
// "<service_name>/response".  The request_id field is copied from the request
// into the response so that clients can correlate the two.
//
// IDL convention (required by the template code):
//   - Req must expose  request_id() const  and  request_id(const std::string&)
//   - Resp must expose request_id(const std::string&)
//
// Do not construct directly; use DDSManager::createServiceServer().
template<typename Req, typename Resp>
class ServiceServer
{
public:
    using Handler = std::function<Resp(const Req&)>;

    // Factory method used internally by DDSManager.
    static std::shared_ptr<ServiceServer<Req, Resp>> create(
        const std::string& service_name,
        std::shared_ptr<Publisher<Resp>> pub,
        std::shared_ptr<Subscriber<Req>> sub
    )
    {
        return std::shared_ptr<ServiceServer<Req, Resp>>(
            new ServiceServer<Req, Resp>(service_name, pub, sub)
        );
    }

    ~ServiceServer() = default;

    ServiceServer(const ServiceServer&) = delete;
    ServiceServer& operator=(const ServiceServer&) = delete;
    ServiceServer(ServiceServer&&) = delete;
    ServiceServer& operator=(ServiceServer&&) = delete;

    // True when at least one client has matched the request topic.
    bool isReady() const noexcept
    {
        return sub_ && sub_->isConnected();
    }

    const std::string& getServiceName() const noexcept
    {
        return service_name_;
    }

    uint64_t getTotalHandled() const noexcept
    {
        return total_handled_.load();
    }

    // Called by DDSManager after create() to register the handler.
    // The handler is wrapped in a lambda that also handles the
    // request_id copy and response publication.
    void setHandler(Handler handler, std::shared_ptr<Publisher<Resp>> pub)
    {
        sub_->setCallback(
            [this, handler, pub](const Req& req)
            {
                try
                {
                    Resp resp = handler(req);
                    resp.request_id(req.request_id());
                    pub->publish(resp);
                    total_handled_.fetch_add(1);
                    Logger::getInstance().debug(
                        "ServiceServer [" + service_name_ +
                        "] handled request_id=" + std::string(req.request_id())
                    );
                }
                catch (const std::exception& e)
                {
                    Logger::getInstance().error(
                        "ServiceServer [" + service_name_ +
                        "] handler exception: " + e.what()
                    );
                }
                catch (...)
                {
                    Logger::getInstance().error(
                        "ServiceServer [" + service_name_ + "] unknown handler exception"
                    );
                }
            }
        );
    }

private:
    ServiceServer(
        const std::string& service_name,
        std::shared_ptr<Publisher<Resp>> pub,
        std::shared_ptr<Subscriber<Req>> sub
    )
        : service_name_(service_name)
        , pub_(pub)
        , sub_(sub)
        , total_handled_(0)
    {
        Logger::getInstance().info("ServiceServer created for service: " + service_name_);
    }

    std::string service_name_;
    std::shared_ptr<Publisher<Resp>> pub_;
    std::shared_ptr<Subscriber<Req>> sub_;
    std::atomic<uint64_t> total_handled_;
};

} // namespace dds_wrapper

#endif // DDS_WRAPPER_SERVICE_SERVER_H
