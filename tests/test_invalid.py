#!/usr/bin/env python
import unittest
import subprocess
import os
import time
import httplib
from my_utils import *


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

    def test5(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        conn.request("GET", "/forbidden/index.html")
        res = conn.getresponse()
        self.assertEqual(res.status, 403)
        conn.close()

    def test6(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        conn.request("GET", "/forbidden/index.html")
        conn.close();

    def test_lastest(self):
        self.assertEqual(self.proc.poll(), None)

    

    @classmethod
    def setUpClass(self):
        self.proc =  Utils.setUpClass()

    @classmethod
    def tearDownClass(self):
        Utils.tearDownClass(self.proc)

if __name__ == '__main__':
    print "Enable Gzip = " + os.environ["ENABLE_GZIP"]
    unittest.main()
