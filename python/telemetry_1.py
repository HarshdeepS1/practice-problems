# Q1: You receive a telemetry packet as a bytes object. Layout: byte 0 is a sync 
# byte 0xAA, byte 1 is the sensor ID, bytes 2-3 are a 16-bit unsigned value in 
# big-endian, and byte 4 is a checksum equal to the XOR of bytes 1 through 3. 
# Write a function parse_packet(data) that returns (sensor_id, value) or raises an error 
# if the packet is invalid.
import pytest

def parse_packet(data):
    # Byte 0 -> byte 0xAA

    if (len(data) != 5):
        raise ValueError("incorrect length")

    if (data[0] != 0xAA):
        raise ValueError("incorrect sync")

    # Byte 1 -> sensor ID
    sensor_id = data[1]

    # Bytes 2,3 are 16 bit unsigned big-endian value

    value = (data[2] << 8) | data[3]

    xor = data[1]
    for i in range(2,4):
        xor ^= data[i]

    if (xor != data[4]):
        raise ValueError("incorrect checksum")

    return (sensor_id,value)


def make(sync=0xAA,id=0x01,hi=0x02,lo=0x03,chk=None):
    if (chk == None):
        chk = id ^ hi ^ lo

    return bytearray([sync,id,hi,lo,chk])

def test_valid():
    assert parse_packet(make()) == (1, 0x0203)

def test_max_value():
    assert parse_packet(make(hi=0xFF, lo=0xFF)) == (1, 65535)

def test_bad_sync():
    with pytest.raises(ValueError):
        parse_packet(make(sync=0x55))

def test_bad_checksum():
    with pytest.raises(ValueError):
        parse_packet(make(chk=0x00))

@pytest.mark.parametrize("n", [0, 4, 6])
def test_bad_length(n):
    with pytest.raises(ValueError):
        parse_packet(bytes(n))




    




