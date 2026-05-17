# TWS / IB Gateway

The trading engine connects to Interactive Brokers via the TWS or IB Gateway API.

## Requirements

- **TWS** or **IB Gateway** must be running and accepting API connections
- API connections must be enabled in TWS/Gateway settings (Configuration > API > Settings > Enable ActiveX and Socket Clients)
- The API port must be reachable from the container

## Ports

| Application | Default port |
|---|---|
| TWS | 7497 |
| IB Gateway | 4001 |

The trader code connects to `host.docker.internal:<port>`, which resolves to the
host's loopback interface from inside the container. The `--add-host` flag in
`devcontainer.json` sets up this hostname:

```json
"--add-host", "host.docker.internal:host-gateway"
```

If TWS/Gateway runs outside the container, ensure the API port is listening on the
host and the container can reach it over TCP.
