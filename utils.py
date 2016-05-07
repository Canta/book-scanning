# -*- coding: utf-8 -*-

"""
    Util functions 
    
    @author Daniel Cantarín
    @date 20160507
"""

import datetime

def debug(text) :
    
    if isinstance(text , list) :
        text = safe_unicode_list( text )
    else :
        text = safe_unicode(text)
    
    if isinstance(text, unicode) :
        debug_string = u"[" + safe_unicode(str(datetime.datetime.now())) + u"] - " + text
    else:
        debug_string = u"[" + safe_unicode(str(datetime.datetime.now())) + u"] - " + repr(text)
    
    print debug_string.decode("utf-8")


def safe_unicode(text):
    """
        Given a text, it returns an unicode strings.
    """
    if isinstance(text, unicode):
        u_text = text
    else:
        try:
            u_text = unicode(text, 'utf-8', errors='ignore')

        except Exception, e:
            u_text = text

    return u_text


def safe_unicode_list(lst):
    """
        Given a list, it returns the same list with all its internal 
        strings converted to unicode strings.
    """
    u_list = []
    for string in lst:
        u_list.append(safe_unicode(string))

    return u_list



