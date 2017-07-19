#!/usr/bin/env python

import unittest
import httplib
from my_utils import *
import threading

NB_THREADS = 100
NB_REPEAT_PER_THREAD = 100

class TestChargeHttpServer(unittest.TestCase):

    class ThreadConnect(threading.Thread):
        def __init__(self, threadId, outer):
            threading.Thread.__init__(self)
            self.threadID = threadId
            self.outer = outer

        def run(self):
            try:
                for i in range(1,NB_REPEAT_PER_THREAD):
                    conn = httplib.HTTPConnection(HTTP_URL)
                    conn.request("GET", "/forbidden/index.html")
                    res = conn.getresponse()
                    self.outer.assertEqual(res.status, 403)
                    conn.close()
                    conn.request("GET", "/")
                    res = conn.getresponse()
                    self.outer.assertEqual(res.status, 200)
                    conn.close()
                    headers = { 'Accept-Encoding' : 'gzip' }
                    conn.request("GET", "/", None, headers)
                    response = conn.getresponse()
                    self.outer.assertEqual(response.status, 200)
                    r = response.read()
                    if os.environ.has_key("ENABLE_GZIP") and os.environ["ENABLE_GZIP"] == "on":
                        self.outer.assertEqual(response.getheader('Content-Encoding'), 'gzip')
                        data = Utils.uncompress_response(r)
                        r = Utils.read_file("site1/index.html")
                        self.outer.assertEqual(data, r)
                    conn.close()
            except:
                print "Exception raised"
                self.outer.assertTrue(False)
    

    def test1(self):
        print "Spawning ", NB_THREADS, " threads for tests"
        print "Total expected queries : ", NB_THREADS * NB_REPEAT_PER_THREAD * 2
        threads = []
        for i in range(1, NB_THREADS):
            thread = TestChargeHttpServer.ThreadConnect(i, self)
            threads.append(thread)
            thread.start()

        for t in threads:
            t.join()
    
    def test_lastest(self):
        self.assertEqual(self.proc.poll(), None)

    @classmethod
    def setUpClass(self):
        self.proc =  Utils.setUpClass()

    @classmethod
    def tearDownClass(self):
        Utils.tearDownClass(self.proc)


if __name__ == '__main__':
    unittest.main()
