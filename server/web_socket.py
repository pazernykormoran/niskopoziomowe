import base64
import os
import socket
import time
import threading
import json

from enum import Enum, IntEnum
from typing import List, Deque, Dict
from collections import deque


class NotifType(IntEnum):
    SUCCESS = 0
    ERROR = 1
    WARNING = 2
    INFO = 3


class MessageType(IntEnum):
    SERVER = 0
    CAMERA = 1
    VISION = 2


class MessageSubtype(IntEnum):
    INFO = 0
    CAMERA_CONFIGURATION = 1
    CAMERA_STATE = 2
    CREATE_MODEL = 3 
    TEST_OPENCV_SETTINGS = 4
    GET_SAVING_STATE = 3
    SET_SAVING_STATE = 4
    RELOAD_MODEL = 5
    SET_DELAYS = 6
    GET_LIVE_PHOTO = 7


class SocketUDS:
    connMv: socket.socket  # Connection with Module Vision
    connMc: socket.socket  # Connection with Module Camera

    def __init__(self, socket_bridge=None, file='/tmp/visionqb'):
        self.socket_bridge = socket_bridge
        server_address = file

        # Make sure the socket does not already exist
        try:
            os.unlink(server_address)
        except OSError:
            if os.path.exists(server_address):
                raise

        socket.setdefaulttimeout(60)  # Set socket timeout
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)  # Create a UDS socket
        print('UDS starting up on {}'.format(server_address))
        self.sock.bind(server_address)  # Bind the socket to the address

        self.acceptListen = True
        self.acceptReadMv = True
        self.acceptReadMc = True
        self.hasAllConn = False

        # Tells the socket library that we want it to queue up as many
        # as 2 connect requests before refusing outside connections
        self.sock.listen(2)

        self.connections: Deque[socket.socket] = deque()
        self.connMv = None
        self.connMc = None

        # After maxlen messages, the leftmost value will be popped
        self.dequeMv = deque(maxlen=100)
        self.dequeMc = deque(maxlen=100)

    def start_listen(self):
        while self.acceptListen:
            if not self.hasAllConn:
                # print('Waiting for a connection with client')
                self.sock.settimeout(1)

                try:
                    connection, client_address = self.sock.accept()
                except socket.timeout:
                    continue

                msg: Dict = dict()
                msg['type'] = MessageType.SERVER.value
                msg['subtype'] = MessageSubtype.INFO.value
                msg['id'] = int(time.time())

                data = ''
                waiting_counter = 5
                connection.settimeout(1)

                while data == '' and waiting_counter > 0:
                    print('Waiting for data')
                    try:
                        connection.sendall(json.dumps(msg).encode())
                        data = connection.recv(1024)

                    except socket.timeout:
                        continue

                    finally:
                        waiting_counter -= 1

                if data == '':  # No data received wait for another connection
                    continue

                msg_rcv = json.loads(data)

                print('Type message rcv: ', str(msg_rcv['type']))
                print('Id message rcv: ', str(msg_rcv['id']))

                if msg_rcv['type'] == MessageType.CAMERA.value:
                    print('Recognized type CAMERA: ', str(msg_rcv['type']))
                    print('Subtype', str(msg_rcv['subtype']))
                    self.socket_bridge.notify({'type': NotifType.SUCCESS.value, 'message': 'ModuleCamera connected'})
                    self.connections.append(connection)
                    self.connMc = connection
                    self.acceptReadMc = True
                    thread = threading.Thread(target=self.read_mc)
                    thread.start()

                elif msg_rcv['type'] == MessageType.VISION.value:
                    print('Recognized type VISION: ', str(msg_rcv['type']))
                    print('Subtype', str(msg_rcv['subtype']))
                    self.socket_bridge.notify({'type': NotifType.SUCCESS.value, 'message': 'ModuleVision connected'})
                    self.connections.append(connection)
                    self.connMv = connection
                    self.acceptReadMv = True
                    thread = threading.Thread(target=self.read_mv)
                    thread.start()

                else:
                    print('Unrecognized type', str(msg_rcv['type']))
                    continue

                if len(self.connections) == 2:
                    self.hasAllConn = True  # Enough Stop waiting for next connection

                print('Listen connections count: ', len(self.connections))

            time.sleep(0.1)

    def read_mv(self):
        self.connMv.settimeout(1)
        while self.acceptReadMv:
            try:
                # msg = 'Control Message from Server to Module Vision'  # For detection broken pipe
                msg = ''  # Control empty message for detection broken pipe
                self.connMv.sendall(msg.encode())
                data = self.connMv.recv(1024)
                if data:
                    message = json.loads(data)
                    self.dequeMv.append(message)
                    self.socket_bridge.parse_and_notify(message)
                    print("<< MV << Received message with id: " + str(message['id']))

            except socket.timeout:
                continue

            except BrokenPipeError as bre:
                print('Trying to read data from broken Module Vision socket connection !', bre)
                self.connections.remove(self.connMv)
                self.hasAllConn = False  # Start again waiting for broken connection
                self.acceptReadMv = False
                self.connMv = None
                self.socket_bridge.notify({'type': NotifType.WARNING.value, 'message': 'ModuleVision not connected'})

            time.sleep(0.1)

    def read_mc(self):
        while self.acceptReadMc:
            try:
                # msg = 'Control Message from Server to Module Camera'  # For detection broken pipe
                msg = ''  # Control empty message for detection broken pipe
                self.connMc.sendall(msg.encode())
                data = self.connMc.recv(1048576)
                if data:
                    message = json.loads(data)
                    self.dequeMc.append(message)
                    if message['subtype'] == MessageSubtype.CAMERA_CONFIGURATION.value:
                        self.socket_bridge.sendCameraConfig(json.loads(base64.b64decode(message['payload']).decode()))
                    if message['subtype'] == 4:
                        self.socket_bridge.sendModelInfo(json.loads(base64.b64decode(message['payload']).decode()))
                    print("<< MC << Received message with id: " + str(message['id']))
                    # print(message)

            except socket.timeout:
                continue

            except BrokenPipeError as bre:
                print('Trying to read data from broken Module Camera socket connection !', bre)
                self.connections.remove(self.connMc)
                self.hasAllConn = False  # Start again waiting for broken connection
                self.acceptReadMc = False
                self.connMc = None
                self.socket_bridge.notify({'type': NotifType.WARNING.value, 'message': 'ModuleCamera not connected'})

            time.sleep(0.1)

    def send_mv(self, msg='Message from Server to Module Vision'):
        try:
            if isinstance(msg, str):
                print('string message')
                self.connMv.sendall(msg.encode())
            elif isinstance(msg, bytes):
                print('bytes message')
                self.connMv.sendall(msg)
            else:
                print('{} message'.format(type(msg)))
                self.connMv.sendall(json.dumps(msg).encode())

        except BrokenPipeError as bre:
            print("Trying to send data to broken Module Vision socket connection !", bre)
            self.connections.remove(self.connMv)
            self.hasAllConn = False  # Start again waiting for broken connection
            self.acceptReadMv = False
            self.connMv = None
            self.socket_bridge.notify({'type': NotifType.WARNING.value, 'message': 'ModuleVision not connected'})

    def send_mc(self, msg='Message from Server to Module Camera'):
        print("Send MC to:", self.connMc)

        try:
            if isinstance(msg, str):
                print('Is string')
                self.connMc.sendall(msg.encode())
            elif isinstance(msg, bytes):
                print('Bytes string')
                self.connMc.sendall(msg)
            else:
                print("Not string")
                asd = json.dumps(msg).encode()
                print("sending message:", asd)
                self.connMc.sendall(asd)

        except BrokenPipeError as bre:
            print("Trying to send data to broken Module Camera socket connection !", bre)
            self.connections.remove(self.connMc)
            self.hasAllConn = False  # Start again waiting for broken connection
            self.acceptReadMc = False
            self.connMc = None
            self.socket_bridge.notify({'type': NotifType.WARNING.value, 'message': 'ModuleCamera not connected'})

        except AttributeError as ae:
            print("Connection to Module Camera is probably broken!", ae)

    def response_msg_mv(self, id_msg, timeout=10):
        tmo = time.time() + timeout  # 10 seconds from now
        while time.time() < tmo:
            for msg in list(self.dequeMv):
                if msg['id'] == id_msg:
                    return msg
            time.sleep(0.1)

    def response_msg_mc(self, id_msg, timeout=10):
        tmo = time.time() + timeout  # 10 seconds from now
        while time.time() < tmo:
            for msg in self.dequeMc:
                if msg['id']== id_msg:
                    return msg
            time.sleep(0.1)
