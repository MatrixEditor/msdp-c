# MSDP: Multicast Server Discovery Protocol

***

A simple Protocol defined for resolving local servers that provide a specific service. MSDP is used to identify open chat 
servers in the local network. Therefore, a UDP-Broadcast on a pre-defined port could be implemented. This standard defines 
`224.0.0.200` as the broadcast-channel and `3439` as the port number. See `msdp.h` fora  detailed description of each constant.

The general structure of a packet is listed below:

    +--------+---------------+--------+----------------+--------+---------+-------------+----------+----------+-----------+
    | code   | network_flags | net_id | reserved_bytes | system | version | client_type | uid      | data_len | data      |
    |--------|---------------|--------|----------------|--------|---------|-------------|----------|----------|-----------|
    | 1 Byte | 1 Byte        | 1 Byte | 12 Bytes       | 1 Byte | 1 Byte  | 1 Byte      | 10 Bytes | 1 Byte   | 224 Bytes |
    +--------+---------------+--------+----------------+--------+---------+-------------+----------+----------+-----------+
