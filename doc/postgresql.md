# PostgreSQL

PostgreSQL 18 + TimescaleDB are installed. `~/setup.sh` handles initialization and startup automatically.

## Paths

- Data directory: `/home/ibuser/pgdata`
- Socket directory: `/home/ibuser/pgsocket`

## Connect

```bash
psql -h /home/ibuser/pgsocket -U ibuser postgres
```

## Stop

```bash
pg_ctl -D /home/ibuser/pgdata stop
```
