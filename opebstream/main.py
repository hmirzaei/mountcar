import numpy as np

import socket

import threading
import time
import sys
import json
from multiprocessing import Queue

N = 100
batch_len = 6
client_q_map = dict()

def get_udp_data():
    global client_q_map
    inp = ""
    data_ind = 0
    rec_data = [0] * batch_len
    UDP_IP = ""
    UDP_PORT = 1500

    sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind((UDP_IP, UDP_PORT))

    while True:
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        inp += data.replace(chr(0),"\n")
        tokens = inp.split()

        if inp[-1] == '\n':
            inp = ""
        else:
            inp = tokens[-1]
            del tokens[-1]

        new_batch = []
        for t in tokens:
            if t == 'start':
                if data_ind == batch_len: new_batch.append(rec_data)
                data_ind = 0
            elif data_ind < batch_len:
                try:
                    rec_data[data_ind] = int(t) if data_ind == 0 else float(t)
                    data_ind += 1
                except ValueError:
                    data_ind = 0

        if len(new_batch) > 0:
            items = client_q_map.items()
            for k, q in items:
                map(q.put, new_batch)
                if q.qsize() > 100:
                    client_q_map.pop(k, 0)

        time.sleep(0.01)

t = threading.Thread(target=get_udp_data)
t.daemon = True
t.start()

HOST = ''   
PORT = 1501 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
 
try:
    s.bind((HOST, PORT))
except socket.error as msg:
    print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()
     
s.listen(10)
 
def clientthread(conn):
    global x, y
    while True:
         
        data = conn.recv(1024)
        if not data in client_q_map:
            client_q_map[data] = Queue()
        size = client_q_map[data].qsize()
        l = []
        for i in range(size):
            l.append(client_q_map[data].get())
        reply = str(json.dumps(l))
        if not data: 
            break
     
        conn.sendall(reply)
     
    conn.close()
 
while True:
    conn, addr = s.accept()
    t=threading.Thread(target=clientthread, args = (conn,))
    t.start();
 
s.close()


