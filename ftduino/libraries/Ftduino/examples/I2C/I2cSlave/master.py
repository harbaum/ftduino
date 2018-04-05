#! /usr/bin/env python3
# -*- coding: utf-8 -*-
#

import smbus, time

# versuche alle vorhandenen I2C-Busse zu öffnen, dies sollte
# nicht mit Root-Rechten gemacht werden
for b in range(20):
    try:
        bus = smbus.SMBus(b)
        print("I2C-Bus erfolgreich geöffnet: ", b);

        # die folgenden Zeile löst eine Exception aus
        # wenn unter Afdresse 42 kein Chip vorhanden ist
        bus.read_byte_data(42, 0);

        print("Client gefunden, lasse Lampe an Ausgang O1 blinken ...");
    except:
        bus = None
        
    if bus:
        for i in range(10):
            bus.write_block_data(42, 0x00, [ 0x12, 0x10 ]);
            time.sleep(0.1)
            bus.write_block_data(42, 0x00, [ 0x13, 0x10 ]);
            time.sleep(0.1)

        # alle Ausgänge ausschalten -> 16 0-Bytes senden
        bus.write_block_data(42, 0x00, [ 0x00 ]*16 );


print("fertig")
