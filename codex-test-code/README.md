# Codex Test Code

该目录用于存放 Codex 的测试代码与临时实验文件。

## 进制转换网页

新增了一个可直接打开的静态网页进制转换工具，支持 2~36 进制之间的整数互转。

### 文件说明

- `index.html`：页面结构。
- `styles.css`：页面样式。
- `script.js`：转换逻辑（基于 `BigInt`，支持较大整数）。

### 使用方式

1. 直接在浏览器中打开 `index.html`。
2. 输入待转换数值。
3. 填写原始进制和目标进制（范围 2~36）。
4. 点击“开始转换”查看结果。

### 部署到自己的服务器

这个进制转换工具是纯静态页面（HTML/CSS/JS），部署方式非常简单：只要把 `codex-test-code` 目录放到任意 Web 服务器目录即可。

#### 方案一：Nginx（推荐）

1. 在服务器安装 Nginx。
2. 将项目上传到服务器（例如 `/var/www/base-converter`）。
3. 新建站点配置（示例路径：`/etc/nginx/sites-available/base-converter`）：

```nginx
server {
    listen 80;
    server_name your-domain.com; # 没有域名可先写服务器 IP

    root /var/www/base-converter;
    index index.html;

    location / {
        try_files $uri $uri/ =404;
    }
}
```

4. 启用配置并重载 Nginx：

```bash
sudo ln -s /etc/nginx/sites-available/base-converter /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

5. 浏览器访问 `http://your-domain.com`（或服务器 IP）。

#### 方案二：Apache

把 `codex-test-code` 目录内容拷贝到 Apache 站点目录（如 `/var/www/html`），然后重启 Apache 即可。

```bash
sudo cp -r codex-test-code/* /var/www/html/
sudo systemctl restart apache2
```

#### 方案三：临时运行（快速验证）

如果你只是想快速验证上线效果，可以在服务器上直接运行：

```bash
cd codex-test-code
python3 -m http.server 8080
```

然后访问 `http://服务器IP:8080`。

> 注意：`python -m http.server` 适合测试，不建议用于生产。

#### HTTPS（建议）

如果你有域名，建议启用 HTTPS（比如通过 Certbot + Let's Encrypt），提升访问安全性。
