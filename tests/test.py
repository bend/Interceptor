#!/usr/bin/env python
import unittest
import urllib2
from StringIO import StringIO
import gzip
import subprocess
import os
import time

HTTP_URL = "http://localhost:8000"

class TestHttpServer(unittest.TestCase):

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
    
    # Basic test, get / page, should return 200
    def test1(self):
        request = urllib2.Request(HTTP_URL)
        response = urllib2.urlopen(request)
        self.assertEqual(response.getcode(), 200)
        r = response.read()
        r2 = TestHttpServer.read_file("site1/index.html")
        self.assertEqual(r, r2)

    # Page not found test
    def test2(self):
        try:
            request = urllib2.Request(HTTP_URL + "/notfound")
            response = urllib2.urlopen(request)
        except urllib2.HTTPError as err:
            self.assertEqual(err.code, 404)

    def test3(self):
        request = urllib2.Request(HTTP_URL)
        request.add_header('Accept-Encoding', 'gzip')
        response = urllib2.urlopen(request)
        self.assertEqual(response.getcode(), 200)
        if os.environ.has_key("ENABLE_GZIP") and os.environ["ENABLE_GZIP"] == "on":
            self.assertEqual(response.info().get('Content-Encoding'), 'gzip')
            data = TestHttpServer.uncompress_response(response.read())
            r = TestHttpServer.read_file("site1/index.html")
            self.assertEqual(data, r)

    def test4(self):
        for i in range(0, 30):
            request = urllib2.Request(HTTP_URL)
            request.add_header('Accept-Encoding', 'gzip')
            response = urllib2.urlopen(request)
            self.assertEqual(response.getcode(), 200)
            if os.environ.has_key("ENABLE_GZIP") and os.environ["ENABLE_GZIP"] == "on":
                self.assertEqual(response.info().get('Content-Encoding'), 'gzip')
                data = TestHttpServer.uncompress_response(response.read())
            else:
                self.assertEqual(response.info().get('Content-Encoding'), None)
                data = response.read()
            r = TestHttpServer.read_file("site1/index.html")
            self.assertEqual(r, data)
    
    def test5(self):
        request = urllib2.Request(HTTP_URL + "/lorem.html")
        request.add_header('Accept-Encoding', 'gzip')
        response = urllib2.urlopen(request)
        self.assertEqual(response.getcode(), 200)
        self.assertEqual(response.info().get('Transfer-Encoding'), 'chunked')
        if os.environ.has_key("ENABLE_GZIP") and os.environ["ENABLE_GZIP"] == "on":
            self.assertEqual(response.info().get('Content-Encoding'), 'gzip')
            data = TestHttpServer.uncompress_response(response.read())
            r = TestHttpServer.read_file("site1/lorem.html")
            self.assertEqual(data, r)

    @classmethod
    def setUpClass(self):
        print "setting up environment..."
        pwd = os.path.dirname(os.path.realpath(__file__))
        os.chdir(pwd)
        with open(os.devnull, 'w') as output:
           self.proc = subprocess.Popen(["/usr/local/bin/interceptor","-c", "config.json"], stdout=output)
        time.sleep(1) # wait for the server to be correctly spawned

    @classmethod
    def tearDownClass(self):
        print "cleaning environment..."
        self.proc.kill()

if __name__ == '__main__':
    unittest.main()
