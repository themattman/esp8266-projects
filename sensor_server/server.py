#!/usr/bin/env python3
"""
Very simple HTTP server in python for logging requests
Usage::
    ./server.py [<port>]
"""
from http.server import BaseHTTPRequestHandler, HTTPServer
import logging
from logging import FileHandler
from logging import Formatter

LOG_FORMAT = (
    "%(asctime)s [%(levelname)s]: %(message)s") # in %(pathname)s:%(lineno)d")
LOG_LEVEL = logging.INFO
SENSOR_LOG_FILE = "/tmp/sensor.log"
sensor_logger = logging.getLogger("sensor")
sensor_logger.setLevel(LOG_LEVEL)
sensor_logger_file_handler = FileHandler(SENSOR_LOG_FILE)
sensor_logger_file_handler.setLevel(LOG_LEVEL)
sensor_logger_file_handler.setFormatter(Formatter(LOG_FORMAT))
sensor_logger.addHandler(sensor_logger_file_handler)

class S(BaseHTTPRequestHandler):
    def _set_response(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

    def do_GET(self):
        self._set_response()
        self.wfile.write("GET request for {}".format(self.path).encode('utf-8'))

    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        sensor_logger.info("%s:%s", self.path.split('/')[1], post_data.decode("utf-8"))
        self._set_response()
        self.wfile.write("POST request for {}".format(self.path).encode('utf-8'))

def run(server_class=HTTPServer, handler_class=S, port=8080):
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()

if __name__ == '__main__':
    from sys import argv

    if len(argv) == 2:
        run(port=int(argv[1]))
    else:
        run()
