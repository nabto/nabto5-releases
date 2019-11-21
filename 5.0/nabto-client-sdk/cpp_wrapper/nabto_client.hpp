#pragma once

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <exception>
#include <cstdint>

namespace nabto {
namespace client {

class Status;

class Status {
 public:
    Status(int errorCode) : errorCode_(errorCode) {}
    bool ok() const;

    /**
     * e.g. Stream has reached "end of file (eof)".
     */
    const char* getDescription() const;

    /**
     * e.g. NABTO_CLIENT_STREAM_EOF
     */
    //std::string getStringConstant();

    static const Status& OK;

    int getErrorCode() { return errorCode_; }
 private:
    int errorCode_;
};


class NabtoException : public std::exception
{
 public:
    NabtoException(Status status)
        : status_(status)
    {
    }

    NabtoException(int ec)
        : status_(Status(ec))
    {
    }

    const char* what() const throw()
    {
        return status_.getDescription();
    }

    Status status() const
    {
        return status_;
    }

 private:
    const Status status_;
    std::string w;
};

class LogMessage {
 protected:
    LogMessage(const std::string& message, const std::string& severity)
        : message_(message), severity_(severity)
    {
    }
 public:
    virtual ~LogMessage() {}
    virtual std::string getMessage() { return message_; }
    virtual std::string getSeverity() { return severity_; }
 protected:
    std::string message_;
    std::string severity_;
};

class Logger {
 public:
    virtual ~Logger() {}
    virtual void log(LogMessage message) = 0;
};

class FutureCallback {
 public:
    virtual ~FutureCallback() { }
    virtual void run(Status status) = 0;
};

class Buffer {
 public:
    virtual ~Buffer() {}
    virtual unsigned char* data() = 0;
    virtual size_t size() = 0;
};

class Future {
 public:
    virtual ~Future() {}

    virtual void callback(std::shared_ptr<FutureCallback> cb) = 0;
    void callback(std::function<void (Status status)> cb);
};


class FutureVoid : public Future {
 public:
    virtual ~FutureVoid() {}
    virtual void waitForResult() = 0;
    virtual void getResult() = 0;
};

class FutureBuffer : public Future {
 public:
    virtual ~FutureBuffer() {}
    virtual std::shared_ptr<Buffer> waitForResult() = 0;
    virtual std::shared_ptr<Buffer> getResult() = 0;
};

class MdnsResult {
 public:
    virtual ~MdnsResult() {};
    virtual std::string getAddress() = 0;
    virtual int getPort() = 0;
    virtual std::string getDeviceId() = 0;
    virtual std::string getProductId() = 0;
};

class FutureMdnsResult : public Future {
 public:
    virtual ~FutureMdnsResult() {}
    virtual std::shared_ptr<MdnsResult> waitForResult() = 0;
    virtual std::shared_ptr<MdnsResult> getResult() = 0;
};

class MdnsResolver {
 public:
    virtual ~MdnsResolver() {};
    virtual std::shared_ptr<FutureMdnsResult> getResult() = 0;
};

class Coap {
 public:
    virtual ~Coap() {};
    virtual void setRequestPayload(int contentFormat, std::shared_ptr<Buffer> payload) = 0;
    virtual std::shared_ptr<FutureVoid> execute() = 0;
    virtual int getResponseStatusCode() = 0;
    virtual int getResponseContentFormat() = 0;
    virtual std::shared_ptr<Buffer> getResponsePayload() = 0;
};

class Stream {
 public:
    virtual ~Stream() {};
    virtual std::shared_ptr<FutureVoid> open(uint32_t contentType) = 0;
    virtual std::shared_ptr<FutureBuffer> readAll(size_t n) = 0;
    virtual std::shared_ptr<FutureBuffer> readSome(size_t max) = 0;
    virtual std::shared_ptr<FutureVoid> write(std::shared_ptr<Buffer> data) = 0;
    virtual std::shared_ptr<FutureVoid> close() = 0;
};

class TcpTunnel {
 public:
    virtual ~TcpTunnel() {};
    virtual std::shared_ptr<FutureVoid> open(uint16_t localPort, const std::string& remoteHost, uint16_t remotePort) = 0;
};

class ConnectionEvent {
 public:
    ConnectionEvent(int event) : event_(event) {}
    int getEvent() { return event_; }
 private:
    int event_;
};

class FutureConnectionEvent : public Future {
 public:
    virtual ~FutureConnectionEvent() {}
    virtual ConnectionEvent waitForResult() = 0;
    virtual ConnectionEvent getResult() = 0;
};

class ConnectionEventsListener {
 public:
    virtual ~ConnectionEventsListener() {}
    virtual std::shared_ptr<FutureConnectionEvent> listen() = 0;
    virtual void stop() = 0;
};

class Connection {
 public:
    virtual ~Connection() {};
    virtual void setProductId(const std::string& productId) = 0;
    virtual void setDeviceId(const std::string& deviceId) = 0;
    virtual void setApplicationName(const std::string& applicationName) = 0;
    virtual void setApplicationVersion(const std::string& applicationVersion) = 0;
    virtual void setServerUrl(const std::string& serverUrl) = 0;
    virtual void setServerKey(const std::string& serverKey) = 0;
    virtual void setServerJwtToken(const std::string& serverJwtToken) = 0;
    virtual void setPrivateKey(const std::string& privateKey) = 0;
    virtual std::string getDeviceFingerprintHex() = 0;
    virtual std::string getClientFingerprintHex() = 0;
    virtual void enableDirectCandidates() = 0;
    virtual void addDirectCandidate(const std::string& hostname, uint16_t port) = 0;
    virtual void endOfDirectCandidates() = 0;



    virtual std::shared_ptr<FutureVoid> connect() = 0;
    virtual std::shared_ptr<Stream> createStream() = 0;
    virtual std::shared_ptr<FutureVoid> close() = 0;
    virtual std::shared_ptr<Coap> createCoap(const std::string& method, const std::string& path) = 0;
    virtual std::shared_ptr<TcpTunnel> createTcpTunnel() = 0;
    virtual std::shared_ptr<ConnectionEventsListener> createEventsListener() = 0;
};

class Context {
 public:
    // shared_ptr as swig does not understand unique_ptr yet.
    static std::shared_ptr<Context> create();
    virtual ~Context() {};
    virtual std::shared_ptr<Connection> createConnection() = 0;
    virtual std::shared_ptr<MdnsResolver> createMdnsResolver() = 0;
    virtual void setLogger(std::shared_ptr<Logger> logger) = 0;
    virtual void setLogLevel(const std::string& level) = 0;
    virtual std::string createPrivateKey() = 0;
    static std::string version();
};

class BufferRaw : public Buffer {
 public:
    BufferRaw(const unsigned char* data, size_t dataLength)
    {
        std::copy(data, data + dataLength, std::back_inserter(data_));
    }
    ~BufferRaw() {
    }
    virtual unsigned char* data()
    {
        return data_.data();
    }
    virtual size_t size()
    {
        return data_.size();
    }
 private:
    std::vector<unsigned char> data_;
};

// Helper buffer implementation
class BufferImpl : public Buffer {
 public:
    BufferImpl(std::vector<unsigned char> data)
        : data_(data)
    {
    }
    BufferImpl(const unsigned char* data, size_t dataLength)
    {
        std::copy(data, data + dataLength, std::back_inserter(data_));
    }
    ~BufferImpl() {
    }
    virtual unsigned char* data()
    {
        return data_.data();
    }
    virtual size_t size()
    {
        return data_.size();
    }
 private:
    std::vector<unsigned char> data_;
};


class CallbackFunction : public FutureCallback {
 public:
    CallbackFunction(std::function<void (Status status)> cb)
        : cb_(cb)
    {
    }
    void run(Status status) {

        cb_(status);
    }
 private:
    std::function<void (Status status)> cb_;
};

} } // namespace
