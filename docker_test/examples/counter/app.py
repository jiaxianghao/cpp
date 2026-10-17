"""单进程教学服务：访问 / 累加文件计数，/health 不改变计数。"""
import json
import os
import socket
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path


DATA_DIR = Path(os.environ.get("DATA_DIR", "/data"))
MESSAGE = os.environ.get("APP_MESSAGE", "Hello Docker")
PORT = int(os.environ.get("PORT", "8080"))


class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/health":
            self.reply(200, {"status": "ok"})
        elif self.path == "/":
            counter = DATA_DIR / "visits.txt"
            visits = int(counter.read_text()) + 1 if counter.exists() else 1
            counter.write_text(str(visits))
            self.reply(200, {"message": MESSAGE, "visits": visits,
                             "hostname": socket.gethostname()})
        else:
            self.reply(404, {"error": "not found"})

    def reply(self, status, payload):
        body = (json.dumps(payload, ensure_ascii=False) + "\n").encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


if __name__ == "__main__":
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    print(f"Listening on 0.0.0.0:{PORT}; data={DATA_DIR}", flush=True)
    HTTPServer(("0.0.0.0", PORT), Handler).serve_forever()
