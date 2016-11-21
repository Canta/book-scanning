#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
    Book scanning web server.
    
    It controls the interaction with the scanner and the scanning 
    process.
    
    @date 20160509
    @author Daniel Cantarín <canta@canta.com.ar>
"""


import os
import sys, getopt
import datetime, time
import utils
from jsend import RSuccess, RFail, RError
import json
import serial_worker_process
import tornado.httpserver
import tornado.ioloop
import tornado.web
import tornado.websocket
import tornado.gen
from tornado.options import define, options
import subprocess as sp


CYCLE_STATUSES = {
    "ERROR"          : 0,
    "IDLE"           : 1,
    "GOING_HOME"     : 2,
    "PAGE_FIX"       : 3,
    "SCANNING"       : 4,
    "PAGE_TURN"      : 5
}

DEVICES         = {
    "FIRST"     :   0,
    "SECOND"    :   1
}

def check_queue():
    
    message = ""
    parsed  = { "description" : "", "action" : "" }
    
    if not output_queue.empty():
        message = output_queue.get()
        try:
            parsed = json.loads( message )
        except:
            print "Error parsing " + message
            pass
        
        for c in clients:
            c.write_message(message)
    
    if ( cycle["started"] ) : 
        if ( parsed["description"] == "state_up" ):
            if ( cycle["pages"] <= cycle["current"] ):
                cycle_stop()
            else:
                input_queue.put( "GO_DOWN" );
        elif ( parsed["description"] == "state_down" ):
            cycle["is_down"] = True
        elif ( "change_status" in parsed["description"] ):
            if (
                parsed["description"] != "change_status: " + str(CYCLE_STATUSES["SCANNING"])
                and 
                ( 
                    parsed["description"] == "change_status: " + str(CYCLE_STATUSES["IDLE"])
                    and cycle["last_status"] != CYCLE_STATUSES["SCANNING"]
                )
            ):
                #print "desc not Scanning and desc is Idle but last status is not Scanning"
                cycle["is_down"]    =   False
                cycle["scans"]      =   0
            
            cycle["last_status"] = int(parsed["description"].replace("change_status: ",""))

        # When the plate's down, we take pictures.
        if cycle["current"] < cycle["pages"] and cycle["is_down"] and cycle["scans"] < 2 :
            for c in clients:
                c.write_message( "{ \"command\" : \"scan\", \"device\" : " + str(cycle["device"]) + " }" )
            cycle["current"] = cycle["current"] + 1
            cycle["scans"]   = cycle["scans"]   + 1
        elif cycle["is_down"] and cycle["scans"] >= 2 and cycle["last_status"] == CYCLE_STATUSES["IDLE"] :
            input_queue.put( "PAGE_TURN" );
            cycle["is_down"] = False
        elif cycle["is_down"] and cycle["current"] >= cycle["pages"] and cycle["last_status"] == CYCLE_STATUSES["IDLE"] :
            input_queue.put( "PAGE_TURN" );
            cycle["is_down"] = False
        #print cycle

def cycle_start():
    print "Starting cycle."
    print cycle
    cycle["started"] = True
    input_queue.put( "GO_HOME" );

def cycle_stop():
    print "Stopping cycle."
    print cycle
    cycle["started"] = False
    if cycle["is_down"] :
        input_queue.put( "GO_HOME" );

class WebSocketHandler(tornado.websocket.WebSocketHandler):
    
    def check_origin(self, origin):
        return True
    
    def open(self):
        print 'new ws connection'
        clients.append(self)
        ret         = RSuccess()
        ret.message = "connected"
        self.write_message( json.dumps( ret, indent=2 ) )
 
    def on_message(self, message):
        print 'tornado received from ws client: %s' % json.dumps(message)
        
        command = ""
        parsed  = ""
        try:
            parsed  = json.loads(message)
            command = str(parsed["command"])
        except:
            pass
        
        if ( command == "HOLA" ):
            print "HOLA!"
        elif ( command == "CYCLE_START" ):
            cycle["pages"]      = parsed["pages"]
            cycle["current"]    = 0
            ret                 = RSuccess()
            ret.message         = "cycle started"
            cycle_start()
            self.write_message( json.dumps( ret, indent=2 ) )
        else:
            # input_queue.put( command )
            print "received " + command
 
    def on_close(self):
        print 'ws connection closed'
        clients.remove(self)

class Acquire :
    
    def get( self, numero ) :
        
        try : 
            numero = int(numero)
        except Exception, e:
            ret         = RFail()
            ret.message = str(numero) + " no es un número."
            return ret
        
        #value = sp.call(["python", "image-acquire.py", "-d" + str(numero), "-r1280x1080", "-oacquisition-" + str(numero) + ".png"])
        value = sp.call(["python", "image-acquire.py", "-d" + str(numero), "-r640x480", "-oacquisition-" + str(numero) + ".png"])
        if value != 0 :
            ret         = RFail()
            ret.message = "Acquisition return value: " + str(value) + "."
        else:
            ret = RSuccess()
            with open("acquisition-" + str(numero) + ".png", "rb") as f:
                ret.data["image_base64"] = f.read().encode("base64")
        
        if (numero == 0):
            cycle["device"] = DEVICES["SECOND"]
        elif (numero == 1):
            cycle["device"] = DEVICES["FIRST"]
        
        return ret

class Api (tornado.web.RequestHandler) :
    
    def get( self, parms = None):
        
        self.set_header('Content-Type','application/json; charset=utf-8') 
        ret = RSuccess()
        
        if parms == None:
             ret            = RError()
             ret.message    = "API: api method name expected."
        
        parms   = parms.split("/")
        nombre  = parms[0]
        
        if nombre == "acquire":
            api = Acquire()
            ret = api.get( parms[1] )
        
        return self.write( json.dumps(ret, indent=2) )

class Index (tornado.web.RequestHandler) :
    def get( self ) :
        return self.render("web/index.html")

if __name__ == '__main__':
    
    urls = [
        ("/"                ,   Index           ),
        ("/api"             ,   Api             ),
        ("/api/(.+)"        ,   Api             ),
        ("/api/(.+)/(.*)"   ,   Api             ),
        ("/ws"              ,   WebSocketHandler),
        ("/(.*)"            ,   tornado.web.StaticFileHandler, { "path":  "./web"})
    ]
    
    worker              = serial_worker_process.SerialWorker()
    worker.daemon       = True
    worker.start()
    tornado.options.parse_command_line()
    app                 = tornado.web.Application( handlers=urls )
    server              = tornado.httpserver.HTTPServer(app)
    server.listen( 8080 )
    print "Listening on port:", 8080
    
    clients             = [] 
    input_queue         = worker.input_queue
    output_queue        = worker.output_queue
    cycle               = {
        "pages"         : 0,
        "current"       : 0,
        "scans"         : 0, 
        "device"        : 0,
        "last_status"   : 0,
        "is_down"       : False,
        "started"       : False
    }
    
    main_loop           = tornado.ioloop.IOLoop.instance()
    # adjust the scheduler_interval according to the frames sent by the serial port
    scheduler_interval  = 100
    scheduler           = tornado.ioloop.PeriodicCallback(check_queue, scheduler_interval, io_loop = main_loop)
    scheduler.start()
    main_loop.start()

