"""宿主机自检：临时目录、随机端口，不依赖 Docker，不改变练习计数。"""
import json
import os
from pathlib import Path
import socket
import subprocess
import sys
import tempfile
import time
import urllib.error
import urllib.request


def main():
    with tempfile.TemporaryDirectory(prefix="docker-review-test-") as data:
        with socket.socket() as sock:
            sock.bind(("127.0.0.1", 0))
            port = sock.getsockname()[1]
        env = dict(os.environ, DATA_DIR=data, PORT=str(port), APP_MESSAGE="test-message")
        opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))

        def get(path):
            with opener.open(f"http://127.0.0.1:{port}{path}", timeout=2) as response:
                return json.load(response)

        def start():
            proc = subprocess.Popen(
                [sys.executable, str(Path(__file__).with_name("app.py"))], env=env,
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            for _ in range(50):
                try:
                    get("/health")
                    return proc
                except OSError:
                    time.sleep(0.1)
            proc.terminate()
            proc.wait()
            raise RuntimeError("server did not become ready")

        proc = start()
        try:
            assert get("/health") == {"status": "ok"}
            first = get("/")
            assert first["visits"] == 1 and first["message"] == "test-message"
            assert get("/")["visits"] == 2
            try:
                get("/missing")
                raise AssertionError("expected 404")
            except urllib.error.HTTPError as exc:
                assert exc.code == 404
        finally:
            proc.terminate()
            proc.wait()
        proc = start()
        try:
            assert get("/")["visits"] == 3
        finally:
            proc.terminate()
            proc.wait()
    print("PASS: HTTP, environment, counter, health, 404, persistence after restart")


if __name__ == "__main__":
    main()
