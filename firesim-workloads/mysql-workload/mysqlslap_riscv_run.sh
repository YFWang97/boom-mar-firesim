#!/usr/bin/env bash
set -euo pipefail

# --- config: change these to your paths ---
RISCV_MYSQL_BIN="/"        # contains mysqld, mysql, mysqladmin, mysqlslap
PORT=3307
DATADIR="$(mktemp -d /tmp/riscv-mysqldata.XXXXXX)"
SOCKET="/tmp/riscv-mysqld.sock"
USER_BENCH="bench"
PASS_BENCH="pass"

mysqld_bin="$RISCV_MYSQL_BIN/mysqld"
mysql_bin="$RISCV_MYSQL_BIN/mysql"
mysqladmin_bin="$RISCV_MYSQL_BIN/mysqladmin"
mysqlslap_bin="$RISCV_MYSQL_BIN/mysqlslap"

echo "[1/6] Initialize data dir (insecure, no root password)…"
"$mysqld_bin" --no-defaults --innodb_use_native_aio=0 --initialize-insecure --datadir="$DATADIR"

echo "[2/6] Start RISC-V mysqld on :$PORT …"
"$mysqld_bin" --no-defaults \
  --user=root \
  --datadir="$DATADIR" \
  --port="$PORT" \
  --bind-address=127.0.0.1 \
  --socket="$SOCKET" \
  --innodb_use_native_aio=0 \
  --innodb_buffer_pool_size=64M \
  --innodb_log_file_size=64M \
  --skip-symbolic-links \
  --log-error-verbosity=3 \
  >/tmp/riscv-mysqld.log 2>&1 &

mysqld_pid=$!

cleanup() {
  echo "[CLEANUP] Shutting down mysqld (pid $mysqld_pid)…" || true
  "$mysqladmin_bin" -h127.0.0.1 -P"$PORT" -uroot shutdown >/dev/null 2>&1 || true
  kill "$mysqld_pid" >/dev/null 2>&1 || true
  rm -rf "$DATADIR" "$SOCKET" >/dev/null 2>&1 || true
}
trap cleanup EXIT

echo "[3/6] Wait for server to be ready…"
for i in {1..60}; do
  if "$mysqladmin_bin" -h127.0.0.1 -P"$PORT" -uroot --protocol=TCP ping >/dev/null 2>&1; then
    break
  fi
  sleep 1
  if ! kill -0 "$mysqld_pid" 2>/dev/null; then
    echo "mysqld died; check /tmp/riscv-mysqld.log"; exit 1
  fi
done

echo "[4/6] Create bench user…"
"$mysql_bin" -h127.0.0.1 -P"$PORT" -uroot --protocol=TCP <<SQL
CREATE USER IF NOT EXISTS '$USER_BENCH'@'127.0.0.1' IDENTIFIED BY '$PASS_BENCH';
GRANT ALL ON *.* TO '$USER_BENCH'@'127.0.0.1';
FLUSH PRIVILEGES;
SQL

echo "[5/6] Run mysqlslap…"
"$mysqlslap_bin" \
  -h127.0.0.1 -P"$PORT" --protocol=TCP \
  -u"$USER_BENCH" -p"$PASS_BENCH" \
  --auto-generate-sql \
  --auto-generate-sql-load-type=mixed \
  --concurrency=10,50 \
  --iterations=2 \
  --number-of-queries=1000

echo "[6/6] Done. Server will shut down now."

