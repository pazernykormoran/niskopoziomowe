import socket
import sys


# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the address given on the command line
server_address = ('192.168.1.10', 10000)
print('starting up sever')
sock.bind(server_address)
sock.listen(1)


# def read_mc(self):
#     while self.acceptReadMc:
#         try:
#             # msg = 'Control Message from Server to Module Camera'  # For detection broken pipe
#             msg = ''  # Control empty message for detection broken pipe
#             self.connMc.sendall(msg.encode())
#             data = self.connMc.recv(1048576)
#             if data:
#                 message = json.loads(data)
#                 self.dequeMc.append(message)
#                 if message['subtype'] == MessageSubtype.CAMERA_CONFIGURATION.value:
#                     self.socket_bridge.sendCameraConfig(json.loads(base64.b64decode(message['payload']).decode()))
#                 if message['subtype'] == 4:
#                     self.socket_bridge.sendModelInfo(json.loads(base64.b64decode(message['payload']).decode()))
#                 print("<< MC << Received message with id: " + str(message['id']))
#                 # print(message)

#         except socket.timeout:
#             continue

#         except BrokenPipeError as bre:
#             print('Trying to read data from broken Module Camera socket connection !', bre)
#             self.connections.remove(self.connMc)
#             self.hasAllConn = False  # Start again waiting for broken connection
#             self.acceptReadMc = False
#             self.connMc = None
#             self.socket_bridge.notify({'type': NotifType.WARNING.value, 'message': 'ModuleCamera not connected'})

#         time.sleep(0.1)

while True:
    print("waiting for connection")
    connection, client_address = sock.accept()
    try:
        print( 'client connected:', client_address)
        while True:
            newdata = connection.recv(1024)
            print("reseived", newdata)
            while newdata:
                data += newdata
                newdata = None
            if data:
                # connection.sendall(data)
                print(data)
            else:
                break
            

            #msg_rcv = json.loads(data)

            #print('Type message rcv: ', str(msg_rcv['type']))
            #print('Id message rcv: ', str(msg_rcv['id']))

            #if msg_rcv['type'] == MessageType.CAMERA.value:
            #    print('Recognized type CAMERA: ', str(msg_rcv['type']))
            #    print('Subtype', str(msg_rcv['subtype']))
            #    self.socket_bridge.notify({'type': NotifType.SUCCESS.value, 'message': 'ModuleCamera connected'})
            #    self.connections.append(connection)
            #    self.connMc = connection
            #    self.acceptReadMc = True
            #    thread = threading.Thread(target=self.read_mc)
            #    thread.start()

    finally:
        connection.close()
    
