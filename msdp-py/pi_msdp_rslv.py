import socket
import msdplib
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--host", type=str, required=False, default=None)


def msdp_resolve_server(namespace):
  c = msdplib.ClientHelper(namespace.host)

  local_server = msdplib.local_server(c)
  if len(local_server) == 0:
    print("[i] No servers in local network found!")
    return

  for server in local_server:
    print("[i] Response from server at:")
    print("    - addr : %s" % str(server["data"][:15], 'utf-8'))
    print("    - port : %d%d" % (int(server["data"][15]), int(server["data"][16])))

def pprint_info():
  print("[i] You can choose from the following addressees which one you would")
  print("    like to use. (IPv6 not implemented)\n")
  print("I\tFamily", "\t"*5, "Local\n-\t", "-"*6, "\t"*5, "-"*5, sep="")

  index = 1
  default = None
  for family, x, y, z, addr_info in socket.getaddrinfo(socket.gethostname(), 0):
    print("%s\t%s\t%s" % (index, family, addr_info[0]))
    index = index + 1
    if socket.AddressFamily.AF_INET == family and not default:
      default = addr_info[0]

  print("\n[i] Choose your address or let the system take the default one:")
  x = input_address("    - Addressno.: ")
  if x:
    default = x
  print("    - Address   :", default)
  return default

def input_address(prompt):
  try:
    x = int(input(prompt)) - 1
    return socket.getaddrinfo(socket.gethostname())[x][4][0]
  except:
    return None

# Press the green button in the gutter to run the script.
if __name__ == '__main__':
  n = parser.parse_args()
  if not n.host:
    n.host = pprint_info()

  msdp_resolve_server(n)
