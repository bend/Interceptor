#!/usr/bin/env python

import unittest
import httplib
import socket
from my_utils import *

class TestLowLevelHttpServer(unittest.TestCase):
    
    def test1(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((IP_HOST, IP_PORT))
        sock.send("this ia a message")



    @classmethod
    def setUpClass(self):
        self.proc =  Utils.setUpClass()

    @classmethod
    def tearDownClass(self):
        Utils.tearDownClass(self.proc)


if __name__ == '__main__':
    unittest.main()
