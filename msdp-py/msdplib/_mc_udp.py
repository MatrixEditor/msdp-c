import socket
import struct

import msdplib

class InternalSocket:
  def __init__(self, proto='ipv4', address=None):
    self.stopped = False

    if proto not in ('ipv4'):
      raise ValueError("Invalid proto - expected one of {}".format(('ipv4')))

    bind_address = None
    if proto == 'ipv4':
      self._af_type = socket.AF_INET
      self._broadcast_ip = msdplib.MSDP_MULTICAST_IPv4
      self._address = (self._broadcast_ip, msdplib.MSDP_PORT)
      bind_address = "0.0.0.0"

    self.sock = socket.socket(self._af_type, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    if proto == 'ipv4':
      mreq = socket.inet_aton(self._broadcast_ip)
      if address is not None:
        mreq += socket.inet_aton(address)
      else:
        mreq += struct.pack(b"@I", socket.INADDR_ANY)
      self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq, )
      self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_LOOP, 1)

    if bind_address:
      self.sock.bind((bind_address, msdplib.MSDP_PORT))

  def send_bytes(self, b):
    return self.sock.sendto(b, self._address)

  def next(self):
    if not self.stopped:
      return self.sock.recvfrom(256)

  def timeout(self, t):
    self.sock.settimeout(t)
