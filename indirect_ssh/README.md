# Reverse SSH Tunnel

This package sets up a Linux computer without a public IP so it can be reached through a Linux server with a public IP.

The private computer keeps an outbound SSH connection to the public server. The public server exposes a loopback-only port, and that port forwards back to the private computer's SSH service.

## Topology

```text
your laptop -> public Linux server -> 127.0.0.1:2222 -> private Linux computer:22
```

Default behavior binds the remote tunnel to `127.0.0.1` on the public server. That means the forwarded port is not directly exposed to the internet.

## Public Server Setup

On the public Linux server, make sure SSH remote forwarding is allowed:

```bash
sudo sshd -T | grep -E 'allowtcpforwarding|gatewayports'
```

Recommended settings in `/etc/ssh/sshd_config`:

```text
AllowTcpForwarding yes
GatewayPorts no
```

Reload SSH after changing the file:

```bash
sudo systemctl reload ssh
```

Some distributions name the service `sshd` instead:

```bash
sudo systemctl reload sshd
```

The public server only needs its normal SSH port open, usually port `22`.

## Private Computer Setup

On the private Linux computer:

1. Copy the example config and edit it.

   ```bash
   cp config.example.env config.env
   vi config.env
   ```

   `config.env` is ignored by git so real hosts, users, and key paths are not committed by accident.

2. Ensure the private computer can log in to the public server with a key.

   ```bash
   ssh-copy-id tunnel_user@PUBLIC_SERVER_IP
   ssh tunnel_user@PUBLIC_SERVER_IP
   ```

3. Test the tunnel manually.

   ```bash
   ./reverse-ssh-tunnel.sh ./config.env
   ```

4. From another terminal on the public server, test access back to the private computer.

   ```bash
   ssh -p 2222 private_user@127.0.0.1
   ```

5. Install the persistent systemd service on the private computer.

   ```bash
   sudo ./install_client.sh ./config.env
   ```

## Daily Use

Connect to the public server first:

```bash
ssh tunnel_user@PUBLIC_SERVER_IP
```

Then connect from the public server to the private computer:

```bash
ssh -p 2222 private_user@127.0.0.1
```

## Service Commands

On the private computer:

```bash
sudo systemctl status reverse-ssh-tunnel
sudo journalctl -u reverse-ssh-tunnel -f
sudo systemctl restart reverse-ssh-tunnel
```

## Troubleshooting

If the service fails with `remote port forwarding failed`, the remote port may already be in use on the public server:

```bash
ss -ltnp | grep 2222
```

If the tunnel connects but login fails, test direct SSH from the public server:

```bash
ssh -p 2222 private_user@127.0.0.1 -vv
```

If the tunnel drops often, check network stability and the service logs:

```bash
sudo journalctl -u reverse-ssh-tunnel --since '1 hour ago'
```

## Security Notes

- Use SSH keys, not passwords, for the private computer's connection to the public server.
- Keep `REMOTE_BIND_ADDRESS=127.0.0.1` unless you intentionally want to expose the forwarded port publicly.
- If you set `REMOTE_BIND_ADDRESS=0.0.0.0`, also configure firewall rules on the public server.
- Consider creating a restricted user on the public server only for the tunnel.
