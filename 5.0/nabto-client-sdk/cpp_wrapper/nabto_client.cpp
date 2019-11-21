#include "nabto_client.hpp"
#include <nabto/nabto_client.h>
#include <nabto/nabto_client_experimental.h>

namespace nabto {
namespace client {

class BufferOut : public Buffer {
 public:
    BufferOut(size_t capacity)
        : data_(capacity)
    {
    }
    BufferOut(const uint8_t* data, size_t dataLength)
    {
        data_ = std::vector<uint8_t>(data, data + dataLength);
    }

    ~BufferOut() {
    }
    virtual uint8_t* data()
    {
        return data_.data();
    }
    virtual size_t size()
    {
        return data_.size();
    }
    void resize(size_t n) {
        data_.resize(n);
    }
 private:
    std::vector<uint8_t> data_;
};

const char* Status::getDescription() const {
    return nabto_client_error_get_message(errorCode_);
}

const Status& Status::OK = Status(0);

bool Status::ok() const {
    return errorCode_ == 0;
}

class FutureBufferImpl : public FutureBuffer, public std::enable_shared_from_this<FutureBufferImpl>
{
 public:
    FutureBufferImpl(NabtoClient* context, std::shared_ptr<BufferOut> data, std::shared_ptr<size_t> transferred)
        : future_(nabto_client_future_new(context)), data_(data), transferred_(transferred)
    {
    }
    FutureBufferImpl(NabtoClientFuture* future, std::shared_ptr<BufferOut> data, std::shared_ptr<size_t> transferred)
        : future_(future), data_(data), transferred_(transferred)
    {
    }
    ~FutureBufferImpl()
    {
        if (!ended_) {
            auto c = std::make_shared<FutureBufferImpl>(future_, data_, transferred_);
            c->callback(std::make_shared<CallbackFunction>([](Status){ /* do nothing */ }));
        } else {
            nabto_client_future_free(future_);
        }
    }

    std::shared_ptr<Buffer> waitForResult()
    {
        nabto_client_future_wait(future_);
        ended_ = true;
        return getResult();
    }
    static void doCallback(NabtoClientFuture* future, NabtoClientError ec, void* data)
    {
        FutureBufferImpl* self = (FutureBufferImpl*)data;
        self->ended_ = true;
        self->cb_->run(Status(ec));
        self->selfReference_ = nullptr;
    }
    void callback(std::shared_ptr<FutureCallback> cb)
    {
        cb_ = cb;
        selfReference_ = shared_from_this();
        nabto_client_future_set_callback(future_,
                                         &doCallback,
                                         this);
    }
    std::shared_ptr<Buffer> getResult() {
        auto ec = nabto_client_future_error_code(future_);
        if (ec) {
            throw NabtoException(ec);
        }
        data_->resize(*transferred_);
        return data_;
    }
    NabtoClientFuture* getFuture() {
        return future_;
    }
  private:
    NabtoClientFuture* future_;
    std::shared_ptr<BufferOut> data_;
    std::shared_ptr<size_t> transferred_;
    std::shared_ptr<FutureBufferImpl> selfReference_;
    std::shared_ptr<FutureCallback> cb_;
    bool ended_ = false;
};

class FutureMdnsResultImpl : public FutureMdnsResult, public std::enable_shared_from_this<FutureMdnsResultImpl>
{
 public:
    FutureMdnsResultImpl(NabtoClient* context, std::shared_ptr<MdnsResult> result)
        : future_(nabto_client_future_new(context)), result_(result)
    {
    }
    FutureMdnsResultImpl(NabtoClientFuture* future, std::shared_ptr<MdnsResult> result)
        : future_(future), result_(result)
    {
    }
    ~FutureMdnsResultImpl()
    {
        if (!ended_) {
            auto c = std::make_shared<FutureMdnsResultImpl>(future_, result_);
            c->callback(std::make_shared<CallbackFunction>([](Status){ /* do nothing */ }));
        } else {
            nabto_client_future_free(future_);
        }
    }

    std::shared_ptr<MdnsResult> waitForResult()
    {
        nabto_client_future_wait(future_);
        ended_ = true;
        return getResult();
    }
    static void doCallback(NabtoClientFuture* future, NabtoClientError ec, void* data)
    {
        FutureMdnsResultImpl* self = (FutureMdnsResultImpl*)data;
        self->ended_ = true;
        self->cb_->run(Status(ec));
        self->selfReference_ = nullptr;
    }

    void callback(std::shared_ptr<FutureCallback> cb)
    {
        cb_ = cb;
        selfReference_ = shared_from_this();
        nabto_client_future_set_callback(future_,
                                         &doCallback,
                                         this);
    }
    std::shared_ptr<MdnsResult> getResult() {
        auto ec = nabto_client_future_error_code(future_);
        if (ec) {
            throw NabtoException(ec);
        }
        return result_;
    }
    NabtoClientFuture* getFuture() {
        return future_;
    }

  private:
    NabtoClientFuture* future_;
    std::shared_ptr<MdnsResult> result_;
    std::shared_ptr<FutureMdnsResultImpl> selfReference_;
    std::shared_ptr<FutureCallback> cb_;
    bool ended_ = false;
};

class FutureConnectionEventImpl : public FutureConnectionEvent, public std::enable_shared_from_this<FutureConnectionEventImpl>
{
 public:
    FutureConnectionEventImpl(NabtoClient* context, int* result)
        : future_(nabto_client_future_new(context)), result_(result)
    {
    }
    FutureConnectionEventImpl(NabtoClientFuture* future, int* result)
        : future_(future), result_(result)
    {
    }
    ~FutureConnectionEventImpl()
    {
        if (!ended_) {
            auto c = std::make_shared<FutureConnectionEventImpl>(future_, result_);
            c->callback(std::make_shared<CallbackFunction>([](Status){ /* do nothing */ }));
        } else {
            nabto_client_future_free(future_);
        }
    }

    ConnectionEvent waitForResult()
    {
        nabto_client_future_wait(future_);
        ended_ = true;
        return getResult();
    }
    static void doCallback(NabtoClientFuture* future, NabtoClientError ec, void* data)
    {
        FutureConnectionEventImpl* self = (FutureConnectionEventImpl*)data;
        self->ended_ = true;
        self->cb_->run(Status(ec));
        self->selfReference_ = nullptr;
    }

    void callback(std::shared_ptr<FutureCallback> cb)
    {
        cb_ = cb;
        selfReference_ = shared_from_this();
        nabto_client_future_set_callback(future_,
                                         &doCallback,
                                         this);
    }
    ConnectionEvent getResult() {
        auto ec = nabto_client_future_error_code(future_);
        if (ec) {
            throw NabtoException(ec);
        }
        return ConnectionEvent(*result_);
    }
    NabtoClientFuture* getFuture() {
        return future_;
    }
  private:
    NabtoClientFuture* future_;
    int* result_;
    std::shared_ptr<FutureConnectionEventImpl> selfReference_;
    std::shared_ptr<FutureCallback> cb_;
    bool ended_ = false;
};

class FutureVoidImpl : public FutureVoid, public std::enable_shared_from_this<FutureVoidImpl> {
 public:
    FutureVoidImpl(NabtoClient* context)
        : future_(nabto_client_future_new(context))
    {
    }

    FutureVoidImpl(NabtoClient* context,  std::shared_ptr<Buffer> data)
        : future_(nabto_client_future_new(context)), data_(data)
    {
    }

    FutureVoidImpl(NabtoClientFuture* future, std::shared_ptr<Buffer> data)
        : future_(future), data_(data)
    {
    }
    FutureVoidImpl(NabtoClientFuture* future)
        : future_(future)
    {
    }
    ~FutureVoidImpl()
    {
        if (!ended_) {
            auto c = std::make_shared<FutureVoidImpl>(future_, data_);
            c->callback(std::make_shared<CallbackFunction>([](Status){ /* do nothing */ }));
        } else {
            nabto_client_future_free(future_);
        }
    }
    // waitForResult for result.
    void waitForResult() {
        nabto_client_future_wait(future_);
        ended_ = true;
        return getResult();
    }

    static void doCallback(NabtoClientFuture* future, NabtoClientError ec, void* data)
    {
        FutureVoidImpl* self = (FutureVoidImpl*)data;
        self->ended_ = true;
        self->cb_->run(Status(ec));
        self->selfReference_ = nullptr;
    }

    //bool waitFor(int milliseconds) = 0;
    void callback(std::shared_ptr<FutureCallback> cb)
    {
        cb_ = cb;
        selfReference_ = shared_from_this();
        nabto_client_future_set_callback(future_,
                                         &doCallback,
                                         this);
    }
    void getResult() {
        auto ec = nabto_client_future_error_code(future_);
        if (ec) {
            throw NabtoException(ec);
        }
    }

    NabtoClientFuture* getFuture() {
        return future_;
    }

 private:
    NabtoClientFuture* future_;
    std::shared_ptr<Buffer> data_;
    std::shared_ptr<FutureVoidImpl> selfReference_;
    std::shared_ptr<FutureCallback> cb_;
    bool ended_ = false;
};


class MdnsResultImpl : public MdnsResult {
 public:
    MdnsResultImpl(NabtoClientMdnsResult* result)
        : result_(result)
    {
    }
    ~MdnsResultImpl() {
        nabto_client_experimental_mdns_result_free(result_);
    }
    virtual std::string getAddress()
    {
        const char* str;
        auto ec = nabto_client_experimental_mdns_result_get_address(result_, &str);
        if (ec) {
            throw NabtoException(ec);
        }
        return std::string(str);
    }

    virtual int getPort()
    {
        uint16_t port;
        auto ec = nabto_client_experimental_mdns_result_get_port(result_, &port);
        if (ec) {
            throw NabtoException(ec);
        }
        return port;
    }

    virtual std::string getDeviceId()
    {
        const char* str;
        auto ec = nabto_client_experimental_mdns_result_get_device_id(result_, &str);
        if (ec) {
            throw NabtoException(ec);
        }
        return std::string(str);
    }

    virtual std::string getProductId()
    {
        const char* str;
        auto ec = nabto_client_experimental_mdns_result_get_product_id(result_, &str);
        if (ec) {
            throw NabtoException(ec);
        }
        return std::string(str);
    }
 private:
    NabtoClientMdnsResult* result_;
};

class MdnsResolverImpl : public MdnsResolver {
 public:
    MdnsResolverImpl(NabtoClient* context)
        : context_(context)
    {
        resolver_ = nabto_client_experimental_mdns_resolver_new(context);
    }
    ~MdnsResolverImpl() {
        nabto_client_experimental_mdns_resolver_free(resolver_);
    }
    virtual std::shared_ptr<FutureMdnsResult> getResult()
    {
        auto result = nabto_client_experimental_mdns_result_new(context_);
        auto resultWrapper = std::make_shared<MdnsResultImpl>(result);
        auto future = std::make_shared<FutureMdnsResultImpl>(context_, resultWrapper);
        nabto_client_experimental_mdns_resolver_get_result(resolver_, future->getFuture(), result);
        return future;
    }
 private:
    NabtoClientMdnsResolver* resolver_;
    NabtoClient* context_;
};

class CoapImpl : public Coap {
 public:
    CoapImpl(NabtoClient* context, NabtoClientCoap* coap)
        : context_(context)
    {
        request_ = coap;
    }
    ~CoapImpl() {
        nabto_client_coap_free(request_);
    };

    static std::shared_ptr<CoapImpl> create(NabtoClient* context, NabtoClientConnection* connection, const std::string& method, const std::string& path)
    {
        auto request_ = nabto_client_coap_new(connection, method.c_str(), path.c_str());
        if (!request_) {
            return nullptr;
        }
        return std::make_shared<CoapImpl>(context, request_);
    }

    void setRequestPayload(int contentFormat, std::shared_ptr<Buffer> payload)
    {
        NabtoClientError ec;
        ec = nabto_client_coap_set_request_payload(request_, contentFormat, payload->data(), payload->size());
        if (ec) {
            throw NabtoException(ec);
        }
    }

    std::shared_ptr<FutureVoid> execute()
    {
        auto future = std::make_shared<FutureVoidImpl>(context_);
        nabto_client_coap_execute(request_, future->getFuture());
        return future;
    }

    int getResponseStatusCode()
    {
        NabtoClientError ec;
        uint16_t statusCode;
        ec = nabto_client_coap_get_response_status_code(request_, &statusCode);
        if (ec) {
            throw NabtoException(ec);
        }
        return statusCode;
    }
    int getResponseContentFormat() {
        NabtoClientError ec;
        uint16_t contentFormat;
        ec = nabto_client_coap_get_response_content_format(request_, &contentFormat);
        if (ec == NABTO_CLIENT_EC_NO_DATA) {
            return -1;
        } else if (ec) {
            throw NabtoException(ec);
        }
        return contentFormat;
    }
    std::shared_ptr<Buffer> getResponsePayload() {
        void* payload;
        size_t payloadLength;
        NabtoClientError ec = nabto_client_coap_get_response_payload(request_, &payload, &payloadLength);
        if (ec != NABTO_CLIENT_EC_OK) {
            return nullptr;
        }
        auto ret = std::make_shared<BufferOut>(reinterpret_cast<const uint8_t*>(payload), payloadLength);
        return ret;
    }

 private:
    NabtoClientCoap* request_;
    NabtoClient* context_;
};


class StreamImpl : public Stream {
 public:
    StreamImpl(NabtoClientConnection* connection, NabtoClient* context)
        : context_(context)
    {
        stream_ = nabto_client_stream_new(connection);
    }
    ~StreamImpl() {
        nabto_client_stream_free(stream_);
    }
    std::shared_ptr<FutureVoid> open(uint32_t contentType)
    {
        auto future = std::make_shared<FutureVoidImpl>(context_);
        nabto_client_stream_open(stream_, future->getFuture(), contentType);
        return future;
    }
    std::shared_ptr<FutureBuffer> readAll(size_t n)
    {
        auto data = std::make_shared<BufferOut>(n);
        auto transferred = std::make_shared<size_t>();
        auto future = std::make_shared<FutureBufferImpl>(context_,data, transferred);
        nabto_client_stream_read_all(stream_, future->getFuture(), data->data(), data->size(), transferred.get());
        return future;
    }
    std::shared_ptr<FutureBuffer> readSome(size_t max)
    {
        auto data = std::make_shared<BufferOut>(max);
        auto transferred = std::make_shared<size_t>();
        auto future = std::make_shared<FutureBufferImpl>(context_, data, transferred);
        nabto_client_stream_read_some(stream_, future->getFuture(), data->data(), data->size(), transferred.get());
        return future;
    }
    std::shared_ptr<FutureVoid> write(std::shared_ptr<Buffer> data)
    {
        auto future = std::make_shared<FutureVoidImpl>(context_, data);
        nabto_client_stream_write(stream_, future->getFuture(), data->data(), data->size());
        return future;
    }
    std::shared_ptr<FutureVoid> close()
    {
        auto future = std::make_shared<FutureVoidImpl>(context_);
        nabto_client_stream_close(stream_, future->getFuture());
        return future;
    }
 private:
    NabtoClientStream* stream_;
    NabtoClient* context_;
};

class TcpTunnelImpl : public TcpTunnel {
 public:
    TcpTunnelImpl(NabtoClient* context, NabtoClientConnection* connection)
        : context_(context)
    {
        tcpTunnel_ = nabto_client_tcp_tunnel_new(connection);
    }
    virtual ~TcpTunnelImpl() {
        nabto_client_tcp_tunnel_free(tcpTunnel_);
    };
    virtual std::shared_ptr<FutureVoid> open(uint16_t localPort, const std::string& remoteHost, uint16_t remotePort)
    {
        auto future = std::make_shared<FutureVoidImpl>(context_);
        nabto_client_tcp_tunnel_open(tcpTunnel_, future->getFuture(), localPort, remoteHost.c_str(), remotePort);
        return future;
    }
 private:
    NabtoClientTcpTunnel* tcpTunnel_;
    NabtoClient* context_;
};




class ConnectionEventsListenerImpl : public ConnectionEventsListener {
 public:
    ConnectionEventsListenerImpl(NabtoClient* context, NabtoClientConnection* connection)
        : context_(context), connection_(connection)
    {

        listener_ = nabto_client_listener_new(context);
    }

    void init()
    {
        NabtoClientError ec = nabto_client_connection_events_init_listener(connection_, listener_);
        if (ec) {
            throw NabtoException(ec);
        }
    }

    virtual ~ConnectionEventsListenerImpl() {
        nabto_client_listener_free(listener_);
    }
    std::shared_ptr<FutureConnectionEvent> listen()
    {
        auto future = std::make_shared<FutureConnectionEventImpl>(context_, &event_);
        nabto_client_listener_connection_event(listener_, future->getFuture(), &event_);
        return future;
    }

    void stop() {
        nabto_client_listener_stop(listener_);
    }
 private:
    NabtoClient* context_;
    NabtoClientConnection* connection_;
    int event_;
    NabtoClientListener* listener_;
};

class ConnectionImpl : public Connection {
 public:
    ConnectionImpl(NabtoClient* context)
        : context_(context)
    {
        connection_ = nabto_client_connection_new(context);
    }
    ~ConnectionImpl() {
        nabto_client_connection_free(connection_);
    }

    void setProductId(const std::string& productId)
    {
        NabtoClientError ec = nabto_client_connection_set_product_id(connection_, productId.c_str());
        if (ec) {
            throw NabtoException(ec);
        }
    }
    void setDeviceId(const std::string& deviceId)
    {
        NabtoClientError ec = nabto_client_connection_set_device_id(connection_, deviceId.c_str());
        if (ec) {
            throw NabtoException(ec);
        }
    }
    void setServerKey(const std::string& serverKey)
    {
        NabtoClientError ec = nabto_client_connection_set_server_key(connection_, serverKey.c_str());
        if (ec) {
            throw NabtoException(ec);
        }
    }

    void setApplicationName(const std::string& applicationName)
    {
        NabtoClientError ec = nabto_client_connection_set_application_name(connection_, applicationName.c_str());
        if (ec) {
            throw NabtoException(ec);
        }
    }
    void setApplicationVersion(const std::string& applicationVersion)
    {
        NabtoClientError ec = nabto_client_connection_set_application_version(connection_, applicationVersion.c_str());
        if (ec) {
            throw NabtoException(ec);
        }
    }

    void setServerUrl(const std::string& serverUrl)
    {
        NabtoClientError ec = nabto_client_connection_set_server_url(connection_, serverUrl.c_str());
        if (ec) {
            throw NabtoException(ec);
        }
    }
    void setServerJwtToken(const std::string& serverJwtToken)
    {
        NabtoClientError ec = nabto_client_connection_set_server_jwt_token(connection_, serverJwtToken.c_str());
        if (ec) {
            throw NabtoException(ec);
        }
    }

    void setPrivateKey(const std::string& privateKey)
    {
        NabtoClientError ec = nabto_client_connection_set_private_key(connection_, privateKey.c_str());
        if (ec) {
            throw NabtoException(ec);
        }
    }
    std::string getDeviceFingerprintHex()
    {
        char* f;
        auto ec = nabto_client_connection_get_device_fingerprint_hex(connection_, &f);
        if (ec) {
            throw NabtoException(ec);
        }
        auto str = std::string(f);
        nabto_client_string_free(f);
        return str;
    }

    std::string getClientFingerprintHex()
    {
        char* f;
        auto ec = nabto_client_connection_get_client_fingerprint_hex(connection_, &f);
        if (ec) {
            throw NabtoException(ec);
        }
        auto str = std::string(f);
        nabto_client_string_free(f);
        return str;
    }

    void enableDirectCandidates()
    {
        NabtoClientError ec = nabto_client_connection_enable_direct_candidates(connection_);
        if (ec) {
            throw NabtoException(ec);
        }
    }

    void addDirectCandidate(const std::string& hostname, uint16_t port)
    {
        NabtoClientError ec = nabto_client_connection_add_direct_candidate(connection_, hostname.c_str(), port);
        if (ec) {
            throw NabtoException(ec);
        }
    }

    void endOfDirectCandidates()
    {
        NabtoClientError ec = nabto_client_connection_end_of_direct_candidates(connection_);
        if (ec) {
            throw NabtoException(ec);
        }
    }

    std::shared_ptr<FutureVoid> connect()
    {
        auto future = std::make_shared<FutureVoidImpl>(context_);
        nabto_client_connection_connect(connection_, future->getFuture());
        return future;
    }
    std::shared_ptr<Stream> createStream()
    {
        return std::make_shared<StreamImpl>(connection_, context_);
    }
    std::shared_ptr<FutureVoid> close()
    {
        auto future = std::make_shared<FutureVoidImpl>(context_);
        nabto_client_connection_close(connection_, future->getFuture());
        return future;
    }

    std::shared_ptr<Coap> createCoap(const std::string& method, const std::string& path)
    {
        return CoapImpl::create(context_, connection_, method, path);
    }

    std::shared_ptr<TcpTunnel> createTcpTunnel()
    {
        return std::make_shared<TcpTunnelImpl>(context_, connection_);
    }

    std::shared_ptr<ConnectionEventsListener> createEventsListener()
    {
        auto ptr = std::make_shared<ConnectionEventsListenerImpl>(context_, connection_);
        ptr->init();
        return ptr;
    }

 private:
    NabtoClientConnection* connection_;
    NabtoClient* context_;
};

class LogMessageImpl : public LogMessage {
 public:
    ~LogMessageImpl() {
    }
    LogMessageImpl(const std::string& message, const std::string& severity)
        : LogMessage(message, severity)
    {
    }
};

class LoggerProxy {
 public:
    LoggerProxy(std::shared_ptr<Logger> logger, NabtoClient* context)
        : logger_(logger)
    {
        nabto_client_set_log_callback(context, &LoggerProxy::cLogCallback, this);
    }

    static void cLogCallback(const NabtoClientLogMessage* message, void* userData)
    {
        LoggerProxy* proxy = (LoggerProxy*)userData;
        LogMessageImpl msg = LogMessageImpl(message->message, message->severityString);
        proxy->logger_->log(msg);
    }

 private:
    std::shared_ptr<Logger> logger_;
};

class ContextImpl : public Context {
 public:
    ContextImpl() {
        context_ = nabto_client_new();
    }
    ~ContextImpl() {
        nabto_client_free(context_);
    }

    std::shared_ptr<Connection> createConnection() {
        return std::make_shared<ConnectionImpl>(context_);
    }

    std::shared_ptr<MdnsResolver> createMdnsResolver() {
        return std::make_shared<MdnsResolverImpl>(context_);
    }

    void setLogger(std::shared_ptr<Logger> logger) {
        // todo test return value.
        loggerProxy_ = std::make_shared<LoggerProxy>(logger, context_);
    }

    void setLogLevel(const std::string& level) {
        NabtoClientError ec = nabto_client_set_log_level(context_, level.c_str());
        if (ec) {
            throw NabtoException(ec);
        }
    }

    std::string createPrivateKey() {
        char* privateKey;
        auto ec = nabto_client_create_private_key(context_, &privateKey);
        if (ec) {
            throw NabtoException(ec);
        }
        auto ret = std::string(privateKey);
        nabto_client_string_free(privateKey);
        return ret;
    }

 private:
    NabtoClient* context_;
    std::shared_ptr<LoggerProxy> loggerProxy_;

};

std::string Context::version() {
    return std::string(nabto_client_version());
}

std::shared_ptr<Context> Context::create()
{
    return std::make_shared<ContextImpl>();
}

void Future::callback(std::function<void (Status status)> cb)
{
    callback(std::make_shared<CallbackFunction>(cb));
}

} } // namespace
