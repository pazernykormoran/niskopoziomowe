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
    filename = '/var/www/html/photos/some_image'+str(random())+'.jpg'  # I assume you have a way of picking unique filenames
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
        
        try:
            print( 'client connected:', client_address)
            while True:
                connection.settimeout(0.01)
                try:
                    connection.send(bytes('hello', 'UTF-8'))
                except socket.error as e:
                    print("error connection")
                    connection.close()

                # time.sleep(0.1)
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

        except Exception as e: 
            print("Broken connection, closing")


        finally:
            connection.close()
    

if __name__ == '__main__':
    socket_server = threading.Thread(target=run_socket_server)
    socket_server.start()
    time.sleep(2)
