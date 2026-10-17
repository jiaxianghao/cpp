"""Linux 本地文件系统：文件替换与旧描述符；不模拟断电。"""
import os
from pathlib import Path
import tempfile


def publish(target, content):
    # 同目录临时文件保证替换发生在同一文件系统。
    with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", dir=target.parent,
                                     prefix=".pending-", delete=False) as stream:
        pending = Path(stream.name)
        stream.write(content)
        stream.flush()
        os.fsync(stream.fileno())
    os.replace(pending, target)
    directory = os.open(target.parent, os.O_RDONLY | os.O_DIRECTORY)
    try:
        os.fsync(directory)
    finally:
        os.close(directory)


def main():
    with tempfile.TemporaryDirectory(prefix="ostep-") as directory:
        target = Path(directory) / "config.txt"
        publish(target, "version=old\n")
        with target.open(encoding="utf-8") as old_reader:
            publish(target, "version=new\n")
            # 此前没有读取，所以没有用户态读缓存干扰观察。
            old = old_reader.read().strip()
            new = target.read_text(encoding="utf-8").strip()
        print(f"old fd: {old}")
        print(f"new open: {new}")
        print("file fsync -> replace -> directory fsync completed (not a power-loss test)")
        assert old == "version=old" and new == "version=new"


if __name__ == "__main__":
    main()
