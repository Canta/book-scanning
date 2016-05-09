#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
    Image adquisition routine in an autonomous executable script.
    
    It's intended for use with webcams, but it actually works by reading
    "/dev/video??" files using the OpenCV library. So... any device that
    registers as a "/dev/video??" may work.

    @date 20160507
    @author Daniel Cantarín <canta@canta.com.ar>
    
"""

import cv2
import os
import sys, getopt
import datetime, time
import utils


def debug( thing ) :
    if _debug : 
        utils.debug(thing)

def show_help() :
    help_string = """Usage: python image-acquire.py [OPTIONS]

Options:

  -h, --help        Show this message and exit.
  --debug           Enables debugging output.
  -d, --device      Required. Stablishes the video device number to use
                    to image acquisition.
  -r, --resolution  Optional. Sets the resolution for the device.
                    Default is 640x480.
  -f, --format      Optional. Image output format. Default is "png". 
                    Valid string values are "png", "jpg", and "tiff".
                    There may be other formats available, depending on 
                    the OpenCV library compilation.
  -o, --output      Optional. The output image path. 
                    Default value is "./image.[file_format]".

Examples:

  python image-adquire.py -d0 -r640x480
  
  python image-adquire.py --device 0 --resolution 640x480
  
  python image-adquire.py --debug -d1 -r800x600 -o/tmp/image12.jpg -fjpg

    """
    
    print help_string

def main():
    # First, check the command line parameters.
    
    resolution  =   "640x480"
    device      =   None
    imgformat   =   "png"
    output      =   "image.png"
    
    try:                                
        opts, args = getopt.getopt(sys.argv[1:], "hr:p:d:f:o:", ["help", "debug", "resolution", "device", "format", "output"])
    except getopt.GetoptError:
        print "opciones invalidas"
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--help") :
            show_help()
            sys.exit(0)
        elif opt == "--debug" :
            globals()["_debug"] = True
            debug("Debug enabled.")
        elif opt in ("-d","--device") :
            debug("found device param " + str(opt) + " / " + str(arg) )
            device = arg
        elif opt in ("-r","--resolution") :
            debug("found resolution param " + str(opt) + " / " + str(arg) )
            resolution = arg
        elif opt in ("-o","--output") :
            debug("found output param " + str(opt) + " / " + str(arg) )
            output = arg
        elif opt in ("-f","--format") :
            debug("found format param " + str(opt) + " / " + str(arg) )
            imgformat = arg
    
    if device is None :
        print "\nERROR : No device specified. Impossible to continue.\n"
        show_help()
        exit(1)
    
    width,height    = resolution.lower().split("x")
    chkformat       = output.split(".")[len(output.split(".")) - 1]
    
    try :
        width   = int(width)
        height  = int(height)
        if width <= 0 :
            print "\nERROR : Invalid image width \"" + str(width) + "\".\n Must be a non-zero integer.\n"
            show_help()
            exit(1)
        if height <= 0 :
            show_help()
            exit(1)
            print "\nERROR : Invalid image height \"" + str(height) + "\".\n Must be a non-zero integer.\n"
    except :
        print "\nERROR : Invalid resolution \"" + str(resolution) + "\".\n Must be two non-zero integers, with an \"x\" in between.\n"
        show_help()
        exit(1)
    
    if chkformat != imgformat : 
        #output += "." + imgformat
        tmpoutput = output.split(".")
        tmpoutput[len(output.split("."))-1] = imgformat
        output = ".".join(tmpoutput)
    
    
    debug( 
        [
            "resolution",resolution, type(resolution), 
            "width",width, type(width), 
            "height",height, type(height), 
            "device", device, type(device),
            "format", imgformat, type(imgformat),
            "output", output, type(output),
        ] 
    )
    
    # Then, the image acquisition procedure.
    cam = cv2.VideoCapture(int(device))
    
    cam.set(3,width)
    cam.set(4,height)
    
    take_picture( cam, output )
    
    debug("Done.")
    exit(0)

def take_picture( cam, output_file ) :
    ret, cap = cam.read()
    debug( [ "camera read() return value", ret ] )
    if cap is not False and ret is not False: 
        if cam.isOpened() == 1:
            ret2 = cv2.imwrite(output_file, cap)
            cam.release()
            if not ret2 :
                print "\nERROR : failed to write image to " + str(output_file) + ".\n"
                exit(1)
    else :
        print "\nERROR : failed to capture image from device #" + str(device) + ".\n"
        exit(1)
    

if __name__ == '__main__':
    _debug      =   False
    main()
