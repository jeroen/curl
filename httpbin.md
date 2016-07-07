Install httpbin:

```
sudo pip install httpbin
sudo pip install gunicorn
sudo mkdir /var/log/httpbin
```

Edit `crontab -e` to start a daemonized server. Note you need multiple workers to test for concurrent connections.


```
@reboot /usr/local/bin/gunicorn httpbin:app -D --workers=8 --access-logfile=/var/log/httpbin/access.log --error-logfile=/var/log/httpbin/error.log
```

Example nginx config to copy in `/etc/nginx/sites-enabled` (assuming https is configured)

```
server {
        listen 8007;
        server_name test.opencpu.org httpbin.opencpu.org;
        location / {
                proxy_pass http://localhost:8000/;
        }
}

server {
        listen 8006;
        server_name test.opencpu.org httpbin.opencpu.org;
        location / {
                proxy_pass http://localhost:8000;
        }
}

```

