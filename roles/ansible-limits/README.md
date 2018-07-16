## limits

[![Build Status](https://travis-ci.org/Oefenweb/ansible-limits.svg?branch=master)](https://travis-ci.org/Oefenweb/ansible-limits) [![Ansible Galaxy](http://img.shields.io/badge/ansible--galaxy-limits-blue.svg)](https://galaxy.ansible.com/tersmitten/limits)

Manage (security)limits in Debian-like systems.

#### Requirements

None

#### Variables

* `limits_pam_files` [default: `['/etc/pam.d/common-session', '/etc/pam.d/common-session-noninteractive']`]: Pam files to enable limits for

* `limits_conf_d_files` [default: `{}`]: Limits configuration file declarations
* `limits_conf_d_files.key`: The name of the limits configuration file (e.g `000-percona-server.conf`)
* `limits_conf_d_files.key.domain`: The domain (e.g `mysql`)
* `limits_conf_d_files.key.type`: The type (e.g. `soft`)
* `limits_conf_d_files.key.item`: The item (e.g. `nofile`)
* `limits_conf_d_files.key.value`: The value (e.g. `65535`)

## Dependencies

None

#### Example(s)

##### Simple configuration

```yaml
---
- hosts: all
  roles:
    - limits
```

##### Percona Server (open_files_limit)

```yaml
---
- hosts: all
  roles:
    - limits
  vars:
    limits_conf_d_files:
      000-percona-server.conf:
        - domain: mysql
          type: soft
          item: nofile
          value: 65535
        - domain: mysql
          type: hard
          item: nofile
          value: 65535
```

#### License

MIT

#### Author Information

Mischa ter Smitten (based on work of [Julien Dauphant](https://github.com/jdauphant))

#### Feedback, bug-reports, requests, ...

Are [welcome](https://github.com/Oefenweb/ansible-limits/issues)!
