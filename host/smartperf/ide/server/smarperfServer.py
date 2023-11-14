#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (C) 2023 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import http
import json
from datetime import datetime
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import ssl
import os
from urllib.parse import urlparse
import urllib.request
import subprocess
import re
import platform
import webbrowser

PORT = 9000
CERT_FILE = './cert/certFile.pem'
KEY_FILE = './cert/keyFile.key'
version = 'v1.0.0'
serveInfo = ''


def is_windows():
    return platform.system() == 'Windows'


def is_darwin():
    return platform.system() == 'Darwin'


def is_linux():
    return platform.system() == 'Linux'


def openWeb(url):
    webbrowser.open(url)


def get_pid_by_port(port_number):
    res_pid = -1
    cmd_res = subprocess.run(f"netstat -ano -p tcp | findstr {port_number}", capture_output=True, text=True,
                             shell=True)
    cmd_res_str = cmd_res.stdout
    find_str = re.findall(r'\s(\d+)\s', cmd_res_str)
    if len(find_str) > 0:
        try:
            res_pid = int(find_str[0])
        except ValueError:
            res_pid = -1
    return res_pid


def check_port(port):
    if is_windows():
        pid = get_pid_by_port(port)
        if pid != -1:
            subprocess.run(f"taskkill /F /PID {pid} /T", shell=True)


class SpRequestHandler(http.server.BaseHTTPRequestHandler):
    global version
    global serveInfo

    def log_message(self, format, *args):
        return

    def do_GET(self):
        parse_result = urlparse(self.path)
        if parse_result.path == '/application/serverInfo':
            self.serverInfo_handler()
        elif parse_result.path.startswith('/application'):
            self.application_handler(parse_result)
        else:
            self.send_error(404, 'Not found')

    def do_POST(self):
        parse_result = urlparse(self.path)
        if parse_result.path.startswith('/logger'):
            self.console_handler()
        elif parse_result.path.startswith('/upload'):
            print(f'upload')

        elif parse_result.path.startswith('/download-file'):
            print(f'download-file')
            self.download_handler()

    def download_handler(self):
        self.clear_overdue_file()
        content_type = self.headers.get("Content-Type")
        if content_type and content_type.startswith("application/x-www-form-urlencoded"):
            url = self.post_form_value("url")
            try:
                req = urllib.request.Request(url)
                response = urllib.request.urlopen(req)
                content = response.read().decode("utf-8")
            except Exception as e:
                self.send_response(200)
                response = {"success": False, "code": -1, "message": str(e), "data": None}
                self.wfile.write(bytes(json.dumps(response), "utf-8"))
                return
            suffixStr = os.path.splitext(url.split("/")[-1])[1]
            file_name = f"upload/{datetime.now().strftime('%Y%m%d%H%M%S%f')}{suffixStr}"
            os.makedirs(os.path.dirname(file_name), exist_ok=True)
            try:
                with open(file_name, "wb") as f:
                    f.write(content)
                    written = f.tell()
            except Exception as e:
                self.send_response(200)
                response = {"success": False, "code": -1, "message": str(e), "data": None}
                self.wfile.write(bytes(json.dumps(response), "utf-8"))
                return
            self.send_response(200)
            response = {"success": True, "code": 0, "message": "success", "data": {"url": file_name, "size": written}}
            self.wfile.write(bytes(json.dumps(response), "utf-8"))
            return

    def post_form_value(self, key):
        content_length = int(self.headers.get("Content-Length", 0))
        post_data = self.rfile.read(content_length).decode("utf-8")
        for param in post_data.split("&"):
            param_key, param_value = param.split("=")
            if param_key == key:
                return param_value
        return ""

    def clear_overdue_file(self):
        path = "./upload/"
        upload_dir = os.path.dirname(os.path.normpath(path))
        if not os.path.exists(upload_dir):
            os.makedirs(upload_dir)
        now = datetime.now()
        for root, dirs, files in os.walk(path):
            for file_name in files:
                file_path = os.path.join(root, file_name)
                if self.check_due(file_name):
                    print(f"{now} delete -> {file_path}")
                    os.remove(file_path)

    def check_due(self, file_name):
        now = datetime.now()
        datetime_str = os.path.splitext(os.path.basename(file_name))[0]
        fileDate = datetime.strptime(datetime_str, "%Y%m%d%H%M%S%f")
        return (now - fileDate).total_seconds() > 3600

    def console_handler(self):
        self.check_dir('./logger')
        nowDate = datetime.now()
        now = nowDate.strftime("%Y-%m-%d")
        fileName = f"{now}.txt"
        with open("./logger/" + fileName, "a") as dst:
            content_type = self.headers.get("Content-Type")
            if content_type and content_type.startswith("application/json"):
                content_length = int(self.headers.get("Content-Length", 0))
                req_data = self.rfile.read(content_length)
                req = json.loads(req_data)
                now = datetime.now()
                formatted_date = now.strftime("%Y-%m-%d %H:%M:%S")
                dst.write(f"{formatted_date} {req['fileName']} ({req['fileSize']} M)\n")
                self.send_response(200)
                self.send_header("Content-type", "text/html")
                self.end_headers()
                self.wfile.write(bytes(f"日志写入成功", "utf-8"))

    def check_dir(self, dir_path):
        if not os.path.exists(dir_path):
            os.makedirs(dir_path)

    def application_handler(self, parse_result):
        file_path = parse_result.path[12:]
        file_extension = os.path.splitext(file_path)[1]
        if file_path == '/' or file_path == '':
            file_path = './index.html'
            file_extension = '.html'
        else:
            file_path = '.' + file_path
        try:
            with open(file_path, 'rb') as file:
                content = file.read()
            self.send_response(200)
            self.send_header('Content-type', get_content_type(file_extension))
            self.send_header("Cross-Origin-Opener-Policy", "same-origin")
            self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Access-Control-Allow-Credentials", "true")
            self.send_header("Access-Control-Allow-Headers", "x-requested-with, authorization, blade-auth")
            self.send_header("Access-Control-Allow-Methods", "*")
            self.send_header("Access-Control-Max-Age", "3600")
            self.send_header("data-version", version)
            self.end_headers()
            self.wfile.write(content)
        except FileNotFoundError:
            self.send_error(404, 'File not found')

    def serverInfo_handler(self):
        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("request_info", serveInfo)
        self.end_headers()


def read_text(read_file_path):
    try:
        with open(read_file_path, 'r') as file:
            content = file.read()
        return content
    except IOError:
        return None


def get_content_type(file_extension):
    if file_extension == '.js':
        return 'application/javascript'
    if file_extension == '.wasm':
        return 'application/wasm'
    if file_extension == '.json':
        return 'application/json'
    if file_extension == '.html':
        return 'text/html'
    if file_extension == '.svg':
        return 'image/svg+xml'
    return 'text/plain'


def gen_ssl(cert_file, key_file):
    serial_number = 123456789
    organization = "www.smartperf.com"
    organizational_unit = "ITs"
    common_name = "www.smartperf.com"
    validity_days = 3650
    subprocess.run(["openssl", "genpkey", "-algorithm", "RSA", "-out", key_file])
    csr_file = "cert/cert.csr"
    subprocess.run(["openssl", "req", "-new", "-key", key_file, "-out", csr_file, "-subj",
                    "/O={}/OU={}/CN={}".format(organization, organizational_unit, common_name)])
    subprocess.run(["openssl", "x509", "-req", "-days", str(validity_days), "-in", csr_file, "-signkey",
                    key_file, "-out", cert_file, "-set_serial", str(serial_number)])


class SpServer:
    def __init__(self, server_address, cert_file_path, keyfile_path):
        global version
        global serveInfo
        version = read_text('version.txt')
        serveInfo = read_text('server-config.txt')
        self.server_address = server_address
        self.context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
        self.context.load_cert_chain(cert_file_path, keyfile_path)
        self.httpd = ThreadingHTTPServer(server_address, SpRequestHandler)
        self.httpd.socket = self.context.wrap_socket(self.httpd.socket, True)

    def start(self):
        print(f'HTTPS[{PORT}] SmartPerf Server Start')
        if is_windows():
            openWeb("https://127.0.0.1:9000/application/")
        self.httpd.serve_forever()


def main():
    check_port(PORT)
    if not os.path.exists(CERT_FILE) or not os.path.exists(KEY_FILE):
        if is_linux():
            if not os.path.exists('./cert'):
                os.makedirs('./cert')
            gen_ssl(CERT_FILE, KEY_FILE)
    server_address = ('', PORT)
    server = SpServer(server_address, CERT_FILE, KEY_FILE)
    server.start()


if __name__ == '__main__':
    main()
