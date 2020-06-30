# Nabto 5.1.0 release

## Breaking changes
### Embedded SDK

### Client SDK

## Improvements over last release

## Limitations and known issues

### General

### Embedded SDK

### Client SDK

### Examples

## Getting started

To get started using this release, first build both SDK's, then try
each example.


### building client examples
```
git clone https://github.com/nabto/nabto5-releases.git
mkdir rbuild
cd rbuild
cmake ../nabto5-releases/5.1/nabto-client-sdk/
make
cd ..
```

This will build the client examples in the directory.


### building embedded examples

```
git clone --recursive https://github.com/nabto/nabto-embedded-sdk.git
mkdir ebuild
cd ebuild
cmake ../nabto-embedded-sdk
make
cd ..
```

### Running the examples

Two examples exists with both client and embedded device implementation present in the public repositories.

* simple_coap_get - a very simple coap based hello world request/response example without any authentication etc.
* tcp_tunnel_client/server - a TCP tunnel client and server to remote connect and stream to a port on the embedded device



### Note on local connections
When running the examples, a Server Key is required on the client,
and the device fingerprint must be registered on the basestation. If
the Server Key is invalid, the client will not be allowed access
to the basestation, similarly, if the device fingerprint is invalid
the device will not be allowed to attach to the basestation. In this
scenario, the client would still be able to discover and connect to
the device if they are on the same local network using mDNS.

### Running the TCP tunnel example

Read more on how to setup the tunnels here:
https://docs.nabto.com/developer/guides/get-started/tunnels/quickstart.html

Go to the Nabto cloud console and retrieve a device configuration

Like this:
```
{
  "ProductId":"pr-udnromc7",
  "DeviceId":"de-g9caq9qv",
  "Server":"pr-udnromc7.devices.nabto.net"
}
```

Start the program:
```
./ebuild/apps/tcp_tunnel_device/tcp_tunnel_device
######## Nabto TCP Tunnel Device ########
# Product ID:        pr-udnromc7
# Device ID:         de-g9caq9qv
# Fingerprint:       917402823ffc53bd750fdb9bb5659230f380370927d130aaXXXXXXXXXXXXXXXX
# Pairing password:  s3jYHpsJicYx
# Paring SCT:        7jzPmzFghYcT
# Version:           .1.0
# Pairing URL:       https://tcp-tunnel.nabto.com/pairing?p=pr-udnromc7&d=de-g9caq9qv&fp=917402823ffc53bd750fdb9bb5659230f380370927d130aa8b8e5799a583e99a&pwd=s3jYHpsJicYx&sct=7jzPmzFghYcT
```

Look for the fingerprint (starting with 9174..) and copy it and register it on the device (Nabto Edge -> Products -> devices -> <device-id>)


Start the program again and now you should see the following message informing you about that the device is registered at the basestation and ready to accept incomming requests
```
Attached to the basestation
```


You should now be able to connect to your device using the example
client. First the client must be paired with the device using the
password printed by the device. From the client build directory run:


```
./rbuild/examples/tcp_tunnel_client/tcp_tunnel_client --pair
Scanning for local devices for 2 seconds.
Found 1 local devices.
Choose a device for pairing:
[q]: Quit without pairing
[0] ProductId: pr-udnromc7 DeviceId: de-g9caq9qv
```

Now press the appropriate number (here '0')

```
0
Connected to device ProductId: pr-udnromc7 DeviceId: de-g9caq9qv
Is this the correct fingerprint of the device 917402823ffc53bd750fdb9bb5659230f380370927d130aa8b8e5799a583e99a [yn]
```

Press 'y'

```
Connected to the device
Choose a pairing method 
[0] Password
[1] Local
```

Press 1

Now the client is paired with the device

You should be able to follow the Quick Start guide on how to start the tunnel

