#!/usr/bin/env python

from StringIO import StringIO
import gzip
import os
import subprocess
import time

HTTP_URL = "localhost:7000"

class Utils:
    
    @staticmethod
    def read_file(path):
        f = open(path, "r")
        ret = f.read()
        f.close()
        return ret

    @staticmethod
    def uncompress_response(data):
        buf = StringIO(data)
        f = gzip.GzipFile(fileobj = buf)
        data = f.read()
        return data

    @staticmethod
    def setUpClass():
        print "setting up environment..."
        pwd = os.path.dirname(os.path.realpath(__file__))
        os.chdir(pwd)
        with open(os.devnull, 'w') as output:
           proc = subprocess.Popen(["/usr/local/bin/interceptor","-c", "config.json"], stdout=output)
        time.sleep(1) # wait for the server to be correctly spawned
        return proc

    @staticmethod
    def tearDownClass(proc):
        print "cleaning environment..."
        proc.kill()
