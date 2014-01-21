#!/usr/bin/env python3.3

import socket 
import struct

def gpsDriver( ip_address, port_number, callback ) :
    
    typeJumpTable = { 0x01 : (['time','week','svs','flags','init_num'],">IHBHB"),
                      0x02 : (['latitude','longitude','height'],">ddd"),
                      0x03 : (['X','Y','Z'],">ddd"),
                      0x08 : (['velocityFlags','velocity','heading','up'],">Bfff"),
                      0x09 : (['PDOP','HDOP','VDOP','TDOP'],">ffff"),
                      0x0B : (['PositionRMS','VCVxx','VCVxy','VCVxz','VCVyy','VCVyz','VCVzz',
                               'UnitVariance','numEpoch'],">ffffffffH"),
                      0x0C : (['PositionRMS','sigma-E','sigma-N','cov-EN','sigma-Up','sigma-Major',
                               'sigma-Minor','Orient','UnitVarience','numEpotch'],">fffffffffH") }
    
    packetHash = {}
    
    trimble = socket.socket( socket.AF_INET, socket.SOCK_STREAM ) 
    trimble.connect( ( ip_address, port_number ) )
    # the following code uses blocking oi, so it should not eat your processor.
    while True:
        header  = trimble.recv(7)
        stx, status, type, length, trans_number, page_index, max_page_index = header
        assert stx == 0x02
        assert type == 0x40
        while length > 3 :
            data = trimble.recv(2)
            record_type , record_length = data
            record_data = trimble.recv(record_length)
            if record_type in typeJumpTable :
                assert record_length == struct.calcsize(typeJumpTable[record_type][1])
                for name,value in zip(typeJumpTable[record_type][0],
                                      struct.unpack_from(typeJumpTable[record_type][1],record_data)) :
                    packetHash[name] = value
            else :
                print("unknown record type : {0}".format(record_type))
            length -= record_length + 2
        trimble.recv(2) # chksum and etx
        callback(packetHash)

# ip_address and port_number are ignored, but provided for compatibility with the above
def gpsMockUp( ip_address, port_number, callback) :
    packetHash = {'HDOP': 0.0,        'Orient': 0.0,
                  'PDOP': 0.0,        'PositionRMS': 0.0,
                  'TDOP': 0.0,        'UnitVarience': 0.0,
                  'VDOP': 0.0,        'X': 0.0,
                  'Y': 0.0,           'Z': 0.0,
                  'cov-EN': 0.0,      'flags': 37888,
                  'heading': 0.0,     'height': 0.0,
                  'init_num': 0,      'latitude': 0.0,
                  'longitude': 0.0,   'numEpotch': 0,
                  'sigma-E': 0.0,     'sigma-Major': 0.0,
                  'sigma-Minor': 0.0, 'sigma-N': 0.0,
                  'sigma-Up': 0.0,    'svs': 0,
                  'time': 62200,      'up': 0.0,
                  'velocity': 0.0,    'velocityFlags': 0,
                  'week': 0}
    while True:
        packetHash['time'] += 200
        callback( packetHash )
    

if __name__ == "__main__" :
    import pprint
    import argparse
    parser = argparse.ArgumentParser(description="Trimble TCP driver")
    parser.add_argument('-f', default = False, help = "fake a connection",
                        action = 'store_const', const = True, dest = 'fakeConnection')
    parser.add_argument('ip_address', type=str)
    parser.add_argument('port_number', type=int)
    args = parser.parse_args()
    pp = pprint.PrettyPrinter()
    if args.fakeConnection :
        gpsMockUp( args.ip_address, args.port_number, pp.pprint )
    else :
        gpsDriver( args.ip_address, args.port_number, pp.pprint )
