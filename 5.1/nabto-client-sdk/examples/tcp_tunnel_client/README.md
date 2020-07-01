# TCP tunnel client

Client application which shows how to use TCP tunnelling in nabto.

## Features

  * TCP tunnelling

## Configuration Files

The tcp tunnel is configured using a JSON formatted configuration file
with the format:

```
{
  "ServerKey": "sk-abcdefghijkl",
  "ServerUrl": "https://pr-abcdefg.clients.nabto.net"
}
```

The `ServerUrl` is optional, and will default to
`https://<ProductId>.clients.nabto.net` if not specified. The file can
be provided with the `--config` option, or will default to
`clients.json`. A valid server key can be created in the Nabto Cloud
Console.

Once a client is paired with a heat pump a file called
`tcp_tunnel_client_state.json` is created. The file contains all the
information which is neccessary to communicate with the TCP tunnel.

## Usage

First time the TCP Tunnel client is run it has to be started with the
`--pair` option.


After the client has been paired with the device use the option
`--tcptunnel` with the local port, remote port and remote host
options.
