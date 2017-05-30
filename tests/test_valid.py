#!/usr/bin/env python
import unittest
import httplib
from my_utils import *

HTTP_URL = "localhost:8000"

class TestValidHttpServer(unittest.TestCase):

    # Basic test, get / page, should return 200
    def test1(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        conn.request("GET", "/")
        response = conn.getresponse()
        self.assertEqual(response.status, 200)
        r = response.read()
        r2 = Utils.read_file("site1/index.html")
        self.assertEqual(r, r2)
        conn.close()

    # Page not found test
    def test2(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        conn.request("GET", "/notfound")
        response = conn.getresponse()
        self.assertEqual(response.status, 404)
        conn.close()

    def test3(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        headers = { 'Accept-Encoding' : 'gzip' }
        conn.request("GET", "/", None, headers)
        response = conn.getresponse()
        self.assertEqual(response.status, 200)
        r = response.read()
        if os.environ.has_key("ENABLE_GZIP") and os.environ["ENABLE_GZIP"] == "on":
            self.assertEqual(response.getheader('Content-Encoding'), 'gzip')
            data = Utils.uncompress_response(r)
            r = Utils.read_file("site1/index.html")
            self.assertEqual(data, r)
        conn.close()

    def test4(self):
        for i in range(0, 30):
            conn = httplib.HTTPConnection(HTTP_URL)
            headers = { 'Accept-Encoding' : 'gzip' }
            conn.request("GET", "/", None, headers)
            response = conn.getresponse()
            self.assertEqual(response.status, 200)
            if os.environ.has_key("ENABLE_GZIP") and os.environ["ENABLE_GZIP"] == "on":
                self.assertEqual(response.getheader('Content-Encoding'), 'gzip')
                data = Utils.uncompress_response(response.read())
            else:
                self.assertEqual(response.getheader('Content-Encoding'), None)
                data = response.read()
            r = Utils.read_file("site1/index.html")
            self.assertEqual(r, data)
            conn.close()
    
    def test5(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        headers = { 'Accept-Encoding' : 'gzip' }
        conn.request("GET", "/lorem.html", None, headers)
        response = conn.getresponse()
        self.assertEqual(response.status, 200)
        self.assertEqual(response.getheader('Transfer-Encoding'), 'chunked')
        if os.environ.has_key("ENABLE_GZIP") and os.environ["ENABLE_GZIP"] == "on":
            self.assertEqual(response.getheader('Content-Encoding'), 'gzip')
            data = Utils.uncompress_response(response.read())
            r = Utils.read_file("site1/lorem.html")
            self.assertEqual(data, r)
        conn.close()

    def test6(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        conn.request("HEAD", "/lorem.html")
        res = conn.getresponse()
        self.assertEqual(res.status, 200)
        size = len(Utils.read_file("site1/lorem.html"))
        size2 = int(res.getheader("Content-Length"))
        self.assertEqual(size, size2)
        conn.close()

    def test7(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        headers = { "Range" : "bytes=0-200" }
        conn.request("GET", "/lorem.html", None, headers)
        res = conn.getresponse()
        self.assertEqual(res.status, 206)
        data = res.read()
        self.assertEqual(len(data), 201)
        conn.request("GET", "/lorem.html" , None, {"Range" : "bytes=201-"})
        res = conn.getresponse()
        self.assertEqual(res.status, 206)
        data2 = res.read()
        fdata = Utils.read_file("site1/lorem.html")
        self.assertEqual(len(fdata), len(data) + len(data2))
        self.assertEqual(fdata, data + data2)
        conn.close()

    def test8(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        conn.request("DELETE", "/lorem.html", None, headers)
        res = conn.getresponse()
        self.assertEqual(res.status, 501)
        conn.close()

    @unittest.skip("to fix")
    def test8(self):
        conn = httplib.HTTPConnection(HTTP_URL)
        headers = { "Accept-Encoding" : "gzip", "Range" : "bytes=0-200" }
        conn.request("GET", "/lorem.html", None, headers)
        res = conn.getresponse()
        self.assertEqual(res.status, 206)
        data = res.read()
        self.assertEqual(len(data), 201)

    @classmethod
    def setUpClass(self):
        self.proc =  Utils.setUpClass()

    @classmethod
    def tearDownClass(self):
        Utils.tearDownClass(self.proc)

if __name__ == '__main__':
    unittest.main()
