import socket

import msdplib
import random

CHOICE = "1234567890ABCDEF"

class PacketHelper(dict):
  def __init__(self, from_, data):
    super().__init__()
    if from_:
      self.sender = from_

    if not data:
      return
    offset = 0
    self.setdefault("code", data[offset]); offset = offset + 1
    self.setdefault("nf", data[offset]); offset = offset + 1
    self.setdefault("netid", data[offset]); offset = offset + 1
    self.setdefault("rsvd", data[offset:offset+12]); offset = offset + 12
    self.setdefault("system", data[offset]); offset = offset + 1
    self.setdefault("version", data[offset]); offset = offset + 1
    self.setdefault("ct", data[offset]); offset = offset + 1
    self.setdefault("uid", data[offset:offset+10]); offset = offset + 10
    self.setdefault("len", data[offset]); offset = offset + 1
    if self["len"] > 0:
      self.setdefault("data", data[offset:offset+self["len"]])

class ClientHelper:
  def __init__(self, a):
    self.udp_sock = msdplib.InternalSocket(address=a)
    self.uid = [ord(random.choice(CHOICE)) for i in range(10)]

  def __uid_cmp__(self, u):
    if not u:
      return False
    for i in range(10):
      if u[i] != self.uid[i]:
        return False
    return True

  def next_packet(self):
    packet = None
    while not packet:
      try:
        d, a = self.udp_sock.next()
        p = PacketHelper(a, d)

        if not self.__uid_cmp__(p["uid"]):
          packet = p

      except socket.timeout:
        return None
    return packet

  def timeout(self, t):
    self.udp_sock.timeout(t)

def local_server(client: ClientHelper):
  if not client:
    return []

  ls = []
  client.timeout(2.5)
  data = [msdplib.OP_QRY, 0b00000101, msdplib.NET_ID]
  data += [0 for i in range(12)]
  data += [0, msdplib.VERSION, 0b00000010]
  data += client.uid
  data += [0]


  client.udp_sock.send_bytes(bytes(data))
  while True:
    np = client.next_packet()
    if not np or np["code"] != msdplib.OP_RSP:
      break
    ls.append({"data": np["data"], "type": np["ct"]})

  return ls