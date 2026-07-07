#!/usr/bin/env bash
set -euo pipefail

CONFIG_SOURCE="${1:-./config.env}"
SERVICE_NAME="reverse-ssh-tunnel"
INSTALL_CONFIG="/etc/${SERVICE_NAME}.env"
INSTALL_RUNNER="/usr/local/bin/${SERVICE_NAME}.sh"
INSTALL_SERVICE="/etc/systemd/system/${SERVICE_NAME}.service"

if [[ ! -f "${CONFIG_SOURCE}" ]]; then
  echo "Config file not found: ${CONFIG_SOURCE}" >&2
  echo "Copy config.example.env to config.env and edit it first." >&2
  exit 1
fi

if [[ "${EUID}" -ne 0 ]]; then
  echo "Please run as root, for example: sudo $0 ${CONFIG_SOURCE}" >&2
  exit 1
fi

require_command() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "Required command not found: $1" >&2
    exit 1
  fi
}

require_command ssh
require_command systemctl
require_command install

install -m 0600 "${CONFIG_SOURCE}" "${INSTALL_CONFIG}"
install -m 0755 "$(dirname "$0")/${SERVICE_NAME}.sh" "${INSTALL_RUNNER}"

cat >"${INSTALL_SERVICE}" <<EOF
[Unit]
Description=Reverse SSH tunnel to public server
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
EnvironmentFile=${INSTALL_CONFIG}
ExecStart=${INSTALL_RUNNER} ${INSTALL_CONFIG}
Restart=always
RestartSec=5
StartLimitIntervalSec=0

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable --now "${SERVICE_NAME}.service"

echo "Installed and started ${SERVICE_NAME}.service"
echo "Check status with: systemctl status ${SERVICE_NAME}"
