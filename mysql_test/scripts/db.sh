#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
docker compose exec -T -e MYSQL_PWD=review_local mysql \
  mysql -uroot --default-character-set=utf8mb4 "$@"
