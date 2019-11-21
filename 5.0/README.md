# Nabto 5.0.0 release

## Breaking changes
### Embedded SDK
 * Nabto device events moved out of experimental

### Client SDK
 * The experimental functions for connection events now uses listeners
   with the same pattern as the Embedded SDK.

## Improvements over last release
 * A lot of minor stabilizing fixes for improved software quality

## Limitations and known issues

### General
 * Though the client and embedded SDKs are, with this release,
   considered ready for production, the Nabto basestation has yet to
   be release for production. This means that the development
   basestation must for the time being continue to be used. (server
   URLs should still be of the dev.nabto.net domain, cloud state is
   still configured though https://console.cloud.dev.nabto.com/)

### Embedded SDK
 * IAM and by extension tcptunnelling remains in the experimental
   header as IAM may be subject to change.

### Client SDK
 * TCP tunnels are stil in experimental header.
 * Connection events are still in experimental header.

### Examples
 * On some unlikely errors the examples terminates without proper
   clean-up causing memory leaks upon termination.

## Getting started

To get started using this release, first build both SDK's, then try
each example.

### building embedded SDK
```
mkdir nabto-embedded-sdk/build
cd nabto-embedded-sdk/build
cmake ..
make -j
```

### building client SDK
```
mkdir nabto-client-sdk/build
cd nabto-client-sdk/build
cmake ..
make -j
```

### Note on local connections
When running the examples, a App Server Key is required on the client,
and the device fingerprint must be registered on the basestation. If
the App Server Key is invalid, the client will not be allowed access
to the basestation, similarly, if the device fingerprint is invalid
the device will not be allowed to attach to the basestation. In this
scenario, the client would still be able to discover and connect to
the device if they are on the same local network using mDNS.

### Running the TCP tunnel example
Go to the Nabto cloud console and retrieve a Product ID, a Device ID,
and an App Server Key. Through this example, whereever it says
`<productId>`, `<deviceId>`, `<serverKey>` insert the values you
retrieved.

From the device build directory, initialize the device:

```
./examples/tcptunnel/tcptunnel_device --init -p <productId> -d <deviceId> -s a.devices.dev.nabto.net
```

This creates a new keypair for your tunnel and prints the
fingerprint of the public key. This fingerprint must be configured for the device in the
Nabto Cloud Console. The `init` command also creates a file called
`tcptunnel_device.json` with the configuration of your new device. If
you already have a keypair with the public key fingerprint configured in the
Nabto Cloud Console, you can replace the created device keypair in
the JSON file.

Once your device is configured, it can be started with:

```
./examples/tcptunnel/tcptunnel_device
```

The device should now print the lines:

```
######## Nabto tcptunnel device ########
# Product ID:      <productId>
# Device ID:       <deviceId>
# Fingerprint:     <fingerprint>
# Paring Password: <password>
# Version:         5.0.0.768+b6d0df0
########
Device is attached to the basestation
```

You should now be able to connect to your device using the example
client. First the client must be paired with the device using the
password printed by the device. From the client build directory run:

```
./examples/tcptunnel/tcptunnel_client --password-pairing --password <password> -k <serverKey> -p <productId> -d <deviceId> -s https://a.clients.dev.nabto.net
```

The client will now ask you to verify the fingerprint of the
device. If it matches what the device printed, press `y` to accept the
fingerprint.  Once your client is paired with the device, a TCP tunnel
can be opened between the two. For this example we tunnel port 80 from
the device host to port 4242 on the client host. Allowing you to
access a webserver on the device by connecting to `127.0.0.1:4242` on
the client host.

```
./examples/tcptunnel/tcptunnel_client --tcptunnel --local-port 4242 --remote-port 80 --remote-host 127.0.0.1
```

### Running the Heat pump example
Similarly to the TCP tunnel example, you first need a Product ID, a
Device ID and an App Server Key from the Nabto Cloud Console. Also
like the TCP tunnel, the device must first be initialized to generate
the `heat_pump_device.json` configuration file and a keypair:

```
./examples/heat_pump/heat_pump_device --init -d <deviceId> -p <productId> -s a.devices.dev.nabto.net
```

The device fingerprint must be configured in the Nabto Cloud Console,
after wich the device can be started:

```
./examples/heat_pump/heat_pump_device
```

Again your client must be paired with your new device before it can be
accessed. From the client build directory run:

```
./examples/heat_pump/heat_pump_client --pair -p <productId> -d <deviceId> -k <serverKey> -s https://a.clients.dev.nabto.net
```

This example shows a different pairing approach. Firstly, the client
promts you to accept the fingerprint of the device is correct before
pairing. To accept press `y` followed by enter. Once the device
fingerprint is accepted, the client will attempt to pair with the
device. This causes the device to make a similar prompt for the user
to accept the fingerprint of the client. To accept press `y` followed
by enter. This pairing pattern emulates pressing a physical button on
your embedded device to ensure only someone with physical access to
the device can be allowed to pair.

Once your client is paired with the device, several COAP requests can
be made using the client. A full list of queries can be found in the
with the command line option `--help`. For example, the simulated
heatpump can be turned on by the following command:

```
./examples/heat_pump/heat_pump_client --set-power ON
```

A few other examples could be:

```
./examples/heat_pump/heat_pump_client --set-target 24
./examples/heat_pump/heat_pump_client --get
./examples/heat_pump/heat_pump_client --users-list
./examples/heat_pump/heat_pump_client --users-get --user User-0
```


The heat pump example also shows how the internal MDNS client can be
used to scan for devices on the local network. This can be done using
the below command. This does not require the client to be paired.

```
./examples/heat_pump/heat_pump_client --scan
```
