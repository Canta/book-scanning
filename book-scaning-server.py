#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
    Book scanning web server.
    
    It controls the interaction with the scanner and the scanning 
    process.
    
    @date 20160509
    @author Daniel Cantarín <canta@canta.com.ar>
"""


import web
import cv2
import os
import sys, getopt
import datetime, time
import utils
import subprocess as sp

urls = (
    '/acquire/(.+)', 'Acquire'
)

app = web.application(urls, globals())


class Acquire :
    
    def GET( self, numero ) :
        
        try : 
            numero = int(numero)
        except Exception, e:
            return str(numero) + " no es un número."
        
        ret = sp.call(["python", "image-acquire.py", "-d" + str(numero), "-r1280x1080", "-oacquisition-" + str(numero) + ".png"])
        return ret

if __name__ == "__main__":
    app.run()
