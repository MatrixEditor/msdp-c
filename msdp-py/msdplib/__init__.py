MSDP_MULTICAST_IPv4 = "224.0.0.200"
MSDP_PORT = 3439

OP_INI = 0b00000001
OP_ERR = 0b00000010
OP_QRY = 0b00000100
OP_RSP = 0b00001000
OP_LVE = 0b00010000

VERSION = 0b00000001

NET_ID = 0

from ._mc_udp import (
  InternalSocket
)

from ._mc_client import (
  local_server,
  ClientHelper
)