[gpi]
  memory_size = 5G
  mtu = default
  network_type = default
  port = default
  processes = -1
  timeout = 30
  socket_mode = 0700
  socket_path = /var/tmp/gpi-space
  socket_name = control
  ; api = gpi.api.fake
  api = gpi.api.real

[log "server"]
  url = "localhost:9155"
  level  = INFO

[node]
  daemonize = false

[api "compat"]
  shm_size = 1073741824
