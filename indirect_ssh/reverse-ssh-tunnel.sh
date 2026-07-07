#!/usr/bin/env bash
set -euo pipefail

CONFIG_FILE="${1:-./config.env}"

if [[ ! -f "${CONFIG_FILE}" ]]; then
  echo "Config file not found: ${CONFIG_FILE}" >&2
  exit 1
fi

# shellcheck source=/dev/null
source "${CONFIG_FILE}"

: "${SERVER_HOST:?SERVER_HOST is required}"
: "${SERVER_USER:?SERVER_USER is required}"

SSH_IDENTITY_FILE="${SSH_IDENTITY_FILE:-}"
REMOTE_BIND_ADDRESS="${REMOTE_BIND_ADDRESS:-127.0.0.1}"
REMOTE_PORT="${REMOTE_PORT:-2222}"
LOCAL_HOST="${LOCAL_HOST:-127.0.0.1}"
LOCAL_PORT="${LOCAL_PORT:-22}"
SERVER_ALIVE_INTERVAL="${SERVER_ALIVE_INTERVAL:-30}"
SERVER_ALIVE_COUNT_MAX="${SERVER_ALIVE_COUNT_MAX:-3}"

if [[ "${SSH_IDENTITY_FILE}" == "~/"* ]]; then
  SSH_IDENTITY_FILE="${HOME}/${SSH_IDENTITY_FILE#"~/"}"
fi

if [[ -n "${SSH_IDENTITY_FILE}" && ! -f "${SSH_IDENTITY_FILE}" ]]; then
  echo "SSH identity file not found: ${SSH_IDENTITY_FILE}" >&2
  exit 1
fi

ssh_args=(
  -N
  -T
  -o ExitOnForwardFailure=yes
  -o ServerAliveInterval="${SERVER_ALIVE_INTERVAL}"
  -o ServerAliveCountMax="${SERVER_ALIVE_COUNT_MAX}"
  -R "${REMOTE_BIND_ADDRESS}:${REMOTE_PORT}:${LOCAL_HOST}:${LOCAL_PORT}"
)

if [[ -n "${SSH_IDENTITY_FILE}" ]]; then
  ssh_args+=(-i "${SSH_IDENTITY_FILE}")
fi

exec ssh "${ssh_args[@]}" "${SERVER_USER}@${SERVER_HOST}"
