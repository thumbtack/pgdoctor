pgDoctor
========

Simple, lightweight web service to perform health checks on PostgreSQL instances.


## Dependencies
* Header files for `libpq5` (PostgreSQL library)
* GNU `libmicrohttpd`
* `check`, a unit test framework for C
  
On a Debian-based system, these can easily be installed by running
  
`sudo apt-get install libpq-dev libmicrohttpd-dev check`

  
## Build and install

* Running tests (optional):
  
  The default configuration parameters for running tests rely on a local PostgreSQL instance 
  listening on port 5432. Connections will be established with the `postgres` user and no 
  password.
  
  If this does not match the system you're using, simply edit the relevant `pg_*` parameters
  on `tests/pgdoctor.cfg`.
  
  To actually run the tests all we need to do is execute 
  
    `make check`
    
  If the last line of the output shows something like `100%: Checks: X, Failures: 0, Errors: 0`,
  where `X` is the number of checks performed, all is good.

* Building:
  
    `make`

* Installing

    `sudo make install`


## Configuration
A default [configuration file](https://github.com/thumbtack/pgdoctor/blob/master/pgdoctor.cfg)
is created under `/etc/pgdoctor.cfg`. Each setting is preceded by a comment describing it briefly.


### Runtime settings
| Parameter        | Description           | Default  |
| ------------- |-------------|-----|
| `http_port`      | Port to listen on | 8071 |
| `syslog_facility` | Syslog facility (local) to messages log to | `local7` |

### Target PostgreSQL instance
| Parameter        | Description           | Default  |
| ------------- |-------------|-----|
| `pg_host` | Host name of the instance | `localhost` |
| `pg_port` | Port on which the server is listening | 5432 |
| `pg_user` | User to connect with | `postgres` |
| `pg_password` | Password to use with `pg_user` | *empty* |
| `pg_database` | Name of the database to connect to | `postgres` |

### Health checks
| Parameter        | Description           | Default  |
| ------------- |-------------|-----|
| `pg_connection_timeout` | Timeout (seconds) when connecting to PostgreSQL | 3 |
| `pg_max_replication_lag` | Maximum acceptable delay (seconds) on streaming replication | -1 (disabled) |

#### Custom health checks
pgDoctor supports the definition of custom health checks in the form of arbitrary SQL queries &mdash; one
check per line.

These may be defined in two forms: 
* a plain SQL query, and the health check is considered successful if and only if it is executed without
any errors;
* a SQL query (must return exactly one field) **and** a condition, in which case the health check 
is considered successful when the query is executed without and errors *and* the condition 
evaluates to `true`.

Conditional checks are of the form
```
"QUERY" comparion_operator "VALUE"
```
where `comparison_operator` is one of `<`, `>`, or `=`. `QUERY` is any valid SQL command 
(surrounded by double-quotes), and `VALUE` (also surrounded by double-quotes) is the expected 
result from `QUERY`. 

When using `=`, a string comparison is performed. For both `<` and `>` floating point values are
used. 

---

*Simple examples &mdash; each health check passes iff the query runs without any errors*
* `"SELECT 1"`
* `"SELECT NOW()"`

*Conditional example &mdash; successful iff `on_rotation` equals `1`* 
* `"SELECT on_rotation FROM maintenance WHERE hostname = 'production-replica3'" = "1"`

## Contact
* [pgdoctor](https://groups.google.com/forum/#!forum/pgdoctor) group &mdash; general discussion list.
