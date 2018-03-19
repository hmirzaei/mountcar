import numpy as np

import socket

import threading
import time
import sys
import json

N = 100
batch_len = 5
time_arr = np.array(range(N))
x_arr = np.zeros((N,))
act_arr = np.zeros((N,))
halt_arr = np.zeros((N,))
ret_arr = np.array([])

def get_udp_data():
    global x_arr, act_arr, halt_arr, time_arr, iter_arr, ret_arr
    inp = ""
    data_ind = 0
    rec_data = np.zeros((batch_len,))
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
                rec_data[data_ind] = float(t)
                data_ind += 1

        if len(new_batch) > 0:
            new_batch = new_batch[:N]
            new_batch = [list(i) for i in zip(*new_batch)]
            m = len(new_batch[0])
            x_arr[:-m] = x_arr[m:]
            x_arr[-m:] = new_batch[0]
            act_arr[:-m] = act_arr[m:]
            act_arr[-m:] = new_batch[1]
            halt_arr[:-m] = halt_arr[m:]
            halt_arr[-m:] = new_batch[2]
            last_iter = new_batch[3][-1]
            ret_arr.resize((int(last_iter),), refcheck=False)
            for iter, ret in zip(new_batch[3], new_batch[4]):
                if int(iter) > 0: ret_arr[int(iter)-1] = ret
                
            
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
        reply = str(json.dumps(dict(time = list(time_arr), x=list(x_arr), act=list(act_arr), halt=list(halt_arr), ret=list(ret_arr))))
        if not data: 
            break
     
        conn.sendall(reply)
     
    conn.close()
 
while True:
    conn, addr = s.accept()
    t=threading.Thread(target=clientthread, args = (conn,))
    t.start();
 
s.close()


