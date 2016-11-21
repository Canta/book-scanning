#!/usr/bin/env python
# -*- coding: utf-8 -*-

import serial
import time
import datetime
import multiprocessing

class SerialWorker(multiprocessing.Process):
 
    def __init__(self, port = "/dev/ttyACM0", speed = 9600, timeout = 0.5):
        multiprocessing.Process.__init__(self)
        self.input_queue    = multiprocessing.Queue()
        self.output_queue   = multiprocessing.Queue()
        self.port           = port
        self.speed          = speed
        self.timeout        = timeout
        self.serial         = serial.Serial( port, speed, timeout=timeout )
        self.history        = []
 
    def close(self):
        self.serial.close()
 
    def write(self, data):
        if data != str(data):
            data = str(data)
        self.history.append( {"write": data, "time": datetime.datetime.now() } )
        #if ( "\n" not in data ):
        #    data = data + "\n"
        self.serial.write(data)
        self.serial.flushInput()
        
    def read(self):
        readed = self.serial.readline().replace("\n", "")
        self.history.append( {"read": readed, "time": datetime.datetime.now() } )
        return readed
 
    def run(self):
 
        self.serial.flushInput()
 
        while True:
            # look for webserver request
            if not self.input_queue.empty():
                data = self.input_queue.get()
 
                # send it to the serial device
                self.write(data)
                print "writing to serial: " + data
 
            # look for incoming serial data
            if (self.serial.inWaiting() > 0):
                data = self.read()
                print "reading from serial: " + data
                # let it available to webserver
                self.output_queue.put(data)
