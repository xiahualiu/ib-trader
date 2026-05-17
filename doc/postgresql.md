# PostgreSQL

The trading engine stores market data in PostgreSQL with the TimescaleDB extension for time-series optimization.

## Requirements

- **PostgreSQL 18+** with the **TimescaleDB** extension installed
- The `timescaledb` extension must be enabled on the target database:

Installation guides:
- [PostgreSQL APT repository setup](https://www.postgresql.org/download/linux/ubuntu/)
- [TimescaleDB Debian/Ubuntu install](https://docs.timescale.com/install/latest/self-hosted/installation-debian/)

  ```sql
  CREATE EXTENSION IF NOT EXISTS timescaledb;
  ```

## Connectivity

The code connects to PostgreSQL over TCP using the
`host.docker.internal` hostname, which resolves to the host's loopback
interface from inside the container. This avoids Unix-socket permission issues.

The devcontainer sets up the hostname via `devcontainer.json`:

```json
"--add-host", "host.docker.internal:host-gateway"
```

### Host PostgreSQL configuration

The host PostgreSQL instance must accept TCP connections:

**`postgresql.conf`:**
```
listen_addresses = 'localhost'
```

**`pg_hba.conf`** — allow the Docker bridge subnet (adjust to match your setup):
```
host all all 172.17.0.0/16 md5
```

## Port

The default PostgreSQL port is **5432**. If your host PostgreSQL uses a different port,
change `port` in `postgresql.conf`:

```
port = 5433
```

Then update the `DATABASE_URL` in `.env` to match:

```
DATABASE_URL=postgresql://postgres:...@host.docker.internal:5433/ibalgotrade
```

## Database setup

Create the database, set a password for the postgres role, and enable TimescaleDB:

```bash
sudo -u postgres psql <<'SQL'
ALTER ROLE postgres WITH PASSWORD 'your-password-here';
CREATE DATABASE ibalgotrade;
\c ibalgotrade
CREATE EXTENSION IF NOT EXISTS timescaledb;
SQL
```

Then update the `DATABASE_URL` in `.env` with the password you chose:

```
DATABASE_URL=postgresql://postgres:your-password-here@host.docker.internal:5432/ibalgotrade
```