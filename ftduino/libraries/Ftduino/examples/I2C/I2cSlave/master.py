#! /usr/bin/env python3
# -*- coding: utf-8 -*-
#

import smbus, time, struct

CLIENT = 43

# versuche alle vorhandenen I2C-Busse zu öffnen, dies sollte
# nicht mit Root-Rechten gemacht werden
for b in range(20):
    try:
        bus = smbus.SMBus(b)
        print("I2C-Bus erfolgreich geöffnet: ", b);

        # die folgenden Zeile löst eine Exception aus
        # wenn unter Afdresse CLIENT kein Chip vorhanden ist
        bus.read_byte_data(CLIENT, 0);
        break
    except:
        bus = None
        
if bus:
    print("Client gefunden, lasse Lampe an Ausgang O1 blinken ...");
        
    # I1 in Widerstandsmessing schalten
    bus.write_byte_data(CLIENT, 0x10, 0x01);

    # C2 auf steigende Flanke reagieren lassen
    bus.write_byte_data(CLIENT, 0x24, 0x01)
    # C2 löschen
    bus.write_byte_data(CLIENT, 0x25, 0x01)

    # C1 für Ultraschallsensor aktivieren
    bus.write_byte_data(CLIENT, 0x20, 0x04)
    
    for i in range(10):
        bus.write_i2c_block_data(CLIENT, 0x00, [ 0x12, 0xff ]);

        # alternativ Einzelbytes schreiben:
        # bus.write_byte_data(CLIENT, 0x00, 0x12);
        # bus.write_byte_data(CLIENT, 0x01, 0xff);
            
        time.sleep(0.1)
        bus.write_i2c_block_data(CLIENT, 0x00, [ 0x13, 0xff ]);
        time.sleep(0.1)

        # Widerstand an I1 als zwei Bytes (ein 16-Bit-Wort) lesen
        data = bus.read_i2c_block_data(CLIENT, 0x10, 2)
        value = struct.unpack("<h", bytes(data))[0]
        print("I1 R: ", value)

        # alternativ Einzelbytes lesen:
        # b0 = bus.read_byte_data(CLIENT, 0x10)
        # b1 = bus.read_byte_data(CLIENT, 0x11)
        # value = b1*256+b0

        # Distanzsensor an C1 auswerten
        data = bus.read_i2c_block_data(CLIENT, 0x21, 2)
        value = struct.unpack("<h", bytes(data))[0]
        print("C1 Dist: ", value)

    # Zählerstand C2 ausgeben
    data = bus.read_i2c_block_data(CLIENT, 0x25, 2)
    value = struct.unpack("<h", bytes(data))[0]
    print("C2: ", value)
        
    # alle Ausgänge ausschalten -> 16 0-Bytes senden
    bus.write_block_data(CLIENT, 0x00, [ 0x00 ]*16 );
else:
    print("Kein Gerät an Adresse", CLIENT, "gefunden")

print("fertig")
