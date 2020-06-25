import socket
import sys
import time
import base64
from datetime import datetime
from random import random
import threading

from flask import Flask, render_template, Response
# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the address given on the command line
server_address = ('0.0.0.0', 10000)
print('starting up sever')
sock.bind(server_address)
sock.listen(1)

frame=""

def save_data(img):
    global can_return
    print("save data")
    imgdata = base64.b64decode(img)
    frame = imgdata
    can_return = True
    filename = 'photos/some_image'+str(random())+'.jpg'  # I assume you have a way of picking unique filenames
    with open(filename, 'wb') as f:
        f.write(imgdata)

# def get_frame():
#     global can_return
#     while True:
#         time.sleep(0.05)
#         if can_return == True:
#             can_return = False
#             return frame


def run_socket_server():
    while True:
        print("waiting for connection")
        connection, client_address = sock.accept()
        connection.settimeout(0.01)
        try:
            print( 'client connected:', client_address)
            while True:
                time.sleep(0.01)
                full_data = ""
                data_recv = ""
                try:
                    data_recv = connection.recv(1024)
                    full_data = data_recv
                    while(len(data_recv) >0):
                        data_recv = connection.recv(1024)
                        full_data += data_recv
                except socket.timeout as tme:
                    # print("pas")
                    pass
            
                if len(full_data)>0:
                    print("data received length: ", len(full_data))
                    save_data(full_data)


        finally:
            connection.close()
    

# app = Flask(__name__)

# def render_template():
#     template = '<html><head><title>Video Streaming Demonstration</title></head><body><h1>Video Streaming Demonstration</h1><img src="/vide_feed"></body></html>'

# @app.route('/')
# def index():
#     return render_template()

# @app.route('/test')
# def test():
#     return "test", 200

# def gen():
#     while True:
#         frame = get_frame()
#         yield (b'--frame\r\n'
#                b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

# @app.route('/video_feed')
# def video_feed():
#     return Response(gen(), mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    socket_server = threading.Thread(target=run_socket_server)
    socket_server.start()
    time.sleep(2)
    # app.run(host="127.0.0.1",debug=True)