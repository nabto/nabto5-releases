#ifndef NABTO_CLIENT_EXPERIMENTAL_H
#define NABTO_CLIENT_EXPERIMENTAL_H

#include "nabto_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Experimental header. Functions here are all experimental. They
 * should be used with caution and can be changed in future releases
 * without notice.
 */

/*****************
 * mDNS API
 ******************/
typedef struct NabtoClientMdnsResolver_ NabtoClientMdnsResolver;
typedef struct NabtoClientMdnsResult_ NabtoClientMdnsResult;

/**
 * Experimental: create an mdns resolver
 * @return A new mdns resolver or NULL.
 */

NABTO_CLIENT_DECL_PREFIX NabtoClientMdnsResolver* NABTO_CLIENT_API
nabto_client_experimental_mdns_resolver_new(NabtoClient* context);

/**
 * Experimental: free an mdns resolver, any outstanding get calls will
 * be resolved before freeing.
 */
NABTO_CLIENT_DECL_PREFIX void NABTO_CLIENT_API
nabto_client_experimental_mdns_resolver_free(NabtoClientMdnsResolver* resolver);

/**
 * Experimental: wait for a result from the mdns resolver.
 *
 * The future resolves when a new mdns result is available or when the
 * resolver is freed.
 *
 * Future error codes
 * NABTO_CLIENT_EC_OK if a new result is available
 * NABTO_CLIENT_EC_STOPPED
 */
NABTO_CLIENT_DECL_PREFIX void NABTO_CLIENT_API
nabto_client_experimental_mdns_resolver_get_result(NabtoClientMdnsResolver* resolver, NabtoClientFuture* future, NabtoClientMdnsResult* result);

/**
 * Experimental: allocate new result object
 * @return A new mdns result or NULL
 */
NABTO_CLIENT_DECL_PREFIX NabtoClientMdnsResult* NABTO_CLIENT_API
nabto_client_experimental_mdns_result_new(NabtoClient* context);

/**
 * Experimental: free result object
 */
NABTO_CLIENT_DECL_PREFIX void NABTO_CLIENT_API
nabto_client_experimental_mdns_result_free(NabtoClientMdnsResult* result);

/**
 * Experimental: get IP address of from result object
 * @return String representation of IP address or NULL
 */
NABTO_CLIENT_DECL_PREFIX NabtoClientError NABTO_CLIENT_API
nabto_client_experimental_mdns_result_get_address(NabtoClientMdnsResult* result, const char** address);

/**
 * Experimental: get port of from result object
 * @return port number or 0
 */
NABTO_CLIENT_DECL_PREFIX NabtoClientError NABTO_CLIENT_API
nabto_client_experimental_mdns_result_get_port(NabtoClientMdnsResult* result, uint16_t* port);

/**
 * Experimental: get device ID of from result object
 * @return the device ID or NULL
 */
NABTO_CLIENT_DECL_PREFIX NabtoClientError NABTO_CLIENT_API
nabto_client_experimental_mdns_result_get_device_id(NabtoClientMdnsResult* result, const char** deviceId);

/**
 * Experimental: get product ID of from result object
 * @return the product ID or NULL
 */
NABTO_CLIENT_DECL_PREFIX NabtoClientError NABTO_CLIENT_API
nabto_client_experimental_mdns_result_get_product_id(NabtoClientMdnsResult* result, const char** productId);

/************
 * Tunnel   *
 ************/

/**
 * Nabto TCP tunnel handle.
 */
typedef struct NabtoClientTcpTunnel_ NabtoClientTcpTunnel;

/**
 * Create a tunnel (TODO - new+open not necessarily needed for tunnels, for now just following
 * stream pattern)
 *
 * @param connection  The connection to make the tunnel on, the connection needs
 * to be kept alive until the tunnel has been closed.
 * @return  Tunnel handle if the tunnel could be created, NULL otherwise.
 */
NABTO_CLIENT_DECL_PREFIX NabtoClientTcpTunnel* NABTO_CLIENT_API
nabto_client_tcp_tunnel_new(NabtoClientConnection* connection);

/**
 * Free a tunnel.
 *
 * @param tunnel, the tunnel to free
 */
NABTO_CLIENT_DECL_PREFIX void NABTO_CLIENT_API
nabto_client_tcp_tunnel_free(NabtoClientTcpTunnel* tunnel);

enum NabtoClientTcpTunnelListenMode {
    LISTEN_MODE_LOCALHOST,
    LISTEN_MODE_ANY
};

/**
 * Set the listen mode for the tcp listener. Default is to only listen
 * on localhost / loopback such that only applications on the local
 * machine can connect to the tcp listener. Anyone on the local system
 * can connect to the tcp listener. Some form of application layer
 * authentication needs to be present on the tcp connection if the
 * system is multi tenant or not completely trusted or if the
 * application is not run in isolation.
 */
//NABTO_CLIENT_DECL_PREFIX NabtoClientError NABTO_CLIENT_API
//nabto_client_tcp_tunnel_listen_mode(NabtoClientTcpTunnel* tunnel,
//                                    enum NabtoClientTcpTunnelListenMode listenMode);

/**
 * Opens a TCP tunnel to a remote TCP server through a Nabto enabled device connected to earlier. Device and remote server
 * are often the same.
 *
 * @param tunnel      Tunnel handle crated with nabto_client_tcp_tunnel_new
 * @param localPort   The local TCP port to listen on. If the localPort
 *                    number is 0 the api will choose the port number.
 * @param remoteHost  The host the remote endpoint establishes a TCP
 *                    connection to.
 * @param remotePort  The TCP port to connect to on remoteHost.
 * @return a future, when resolved the tunnel is either established or failed. If established, TCP clients can connect to the endpoint and metadata can be retrieved using nabto_client_tcp_tunnel_get_metadata.
 *
 * Future status:
 *   NABTO_CLIENT_EC_OK if opening went ok.
 *
 *      +--------+          +-----------+               +--------+
 *      | nabto  |   nabto  |   nabto   |   tcp/ip      | remote |
 *   |--+ client +----~~~---+   device  +----~~~-----|--+ server |
 * port | API    |          |           |          port |        |
 *      +--------+           +----------+               +--------+
 */
NABTO_CLIENT_DECL_PREFIX void NABTO_CLIENT_API
nabto_client_tcp_tunnel_open(NabtoClientTcpTunnel* tunnel, NabtoClientFuture* future, uint16_t localPort, const char* remoteHost, uint16_t remotePort);

/**
 * Close a TCP tunnel. Detailed semantics TBD:
 * - TCP listener closed
 * - existing TCP client connections closed?
 * - what about pending stream data?
 *
 * @param tunnel [in] the tunnel to close.
 * @param future [in] the future which resolves with the status of the operation.
 *
 * Future status:
 *  NABTO_CLIENT_EC_OK if the tunnel was closed.
 */
// TODO test that tunnel_close works and is implemented
NABTO_CLIENT_DECL_PREFIX void NABTO_CLIENT_API
nabto_client_tcp_tunnel_close(NabtoClientTcpTunnel* tunnel, NabtoClientFuture* future);

/**
 * Get TCP tunnel metadata as json object:
 *
 * {
 *     "listener_port":      53281,
 *     "TBD":                "TBD"
 * }
 *
 * @param tunnel The tunnel to retrieve meta data about
 * @param json The metadata returned as a JSON string.
 * @return NABTO_CLIENT_EC_OK if the connection is connected, json output is set and must be freed by caller.
 *         NABTO_CLIENT_CONNECTION_CLOSED if the connection is closed or not opened yet.
 */
//NABTO_CLIENT_DECL_PREFIX NabtoClientError NABTO_CLIENT_API
//nabto_client_tcp_tunnel_get_metadata(NabtoClientTcpTunnel* tunnel, char** json);




/*********************
 * Event handler API *
 *********************/
typedef struct NabtoClientListener_ NabtoClientListener;

// TODO add a nabto_client_listener_new to make it as the device.

NABTO_CLIENT_DECL_PREFIX NabtoClientListener* NABTO_CLIENT_API
nabto_client_listener_new(NabtoClient* context);

/**
 * Listeners are used to listen for async events.
 *
 * Free a  event handler.
 *
 * @param eventHandler  Handler to be freed
 */
NABTO_CLIENT_DECL_PREFIX void NABTO_CLIENT_API
nabto_client_listener_free(NabtoClientListener* listener);

/**
 * Stop an events handler. The stop function is needed such that the
 * eventhandler can be stopped without race conditions. When the
 * events handler has been stopped the next event or if there's a
 * current unresolved future will resolve imediately with the status
 * code STOPPED.
 */
NABTO_CLIENT_DECL_PREFIX void NABTO_CLIENT_API
nabto_client_listener_stop(NabtoClientListener* listener);

/**
 * Connection events
 */
typedef int NabtoClientConnectionEvent;
NABTO_CLIENT_DECL_PREFIX extern const NabtoClientConnectionEvent NABTO_CLIENT_CONNECTION_EVENT_CONNECTED;
NABTO_CLIENT_DECL_PREFIX extern const NabtoClientConnectionEvent NABTO_CLIENT_CONNECTION_EVENT_CLOSED;
NABTO_CLIENT_DECL_PREFIX extern const NabtoClientConnectionEvent NABTO_CLIENT_CONNECTION_EVENT_CHANNEL_CHANGED;

/**
 * Create new future for the event listener, called once ready to
 * receive the next event from the event handler.
 *
 * @param eventHandler   Handler to create future for
 * @param future         The future resolved once the next event is available
 * @return NABTO_CLIENT_EC_OK on success
 *         NABTO_CLIENT_EC_OPERATION_IN_PROGRESS if event handler already have a future
 *         NABTO_CLIENT_EC_OUT_OF_MEMORY if future or and underlying structure could not be allocated
 *         NABTO_CLIENT_EC_STOPPED if underlying service stopped (eg. if device closed)
 */
// TODO test channel changed
NABTO_CLIENT_DECL_PREFIX void NABTO_CLIENT_API
nabto_client_listener_connection_event(NabtoClientListener* listener, NabtoClientFuture* future, NabtoClientConnectionEvent* event);

/**
 * Listen for events on a connection. Each a future is resolved on the
 * event listener, the event parameter is set to the current
 * event.
 */
// todo make a init function as in the device
NABTO_CLIENT_DECL_PREFIX NabtoClientError nabto_client_connection_events_init_listener(NabtoClientConnection* connection, NabtoClientListener* listener);


#ifdef __cplusplus
} // extern C
#endif

#endif
