#!/usr/bin/env python
import unittest
import subprocess
import os
import time
import httplib
from my_utils import *

HTTP_URL = "localhost:8000"

class TestInvalidHttpServer(unittest.TestCase):

    def test1(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        headers = { "Range" : "bytes=0" }
        conn.request("GET", "/lorem.html", None, headers)
        res = conn.getresponse()
        self.assertEqual(res.status, 400)
        conn.close()
    
    def test2(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        headers = { "Range" : "bytes=123123123123-" }
        conn.request("GET", "/lorem.html", None, headers)
        res = conn.getresponse()
        self.assertEqual(res.status, 416)
        conn.close()

    def test3(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        headers = { "Range" : "bytes=123123123-" }
        conn.request("GET", "/lorem.html", None, headers)
        res = conn.getresponse()
        self.assertEqual(res.status, 416)
        conn.close()
    
    def test4(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        headers = { "Range" : "bytes=4921565-" }
        conn.request("GET", "/lorem.html", None, headers)
        res = conn.getresponse()
        self.assertEqual(res.status, 416)
        conn.close()
    

    @classmethod
    def setUpClass(self):
        self.proc =  Utils.setUpClass()

    @classmethod
    def tearDownClass(self):
        Utils.tearDownClass(self.proc)

if __name__ == '__main__':
    unittest.main()
