import unittest

from pyk._nk20 import \
     _ktoarray, \
     _arraytok
from pyk._k20 import _ksk, _Ki

from Numeric import *

def k(s): return _ksk(s)

class nk20TestCase(unittest.TestCase):
    def setUp(self):
        self.ik = k('1 2 3 4 5')
        self.fk = k('1.0 2 3 4 5')
        self.ck = k('"abcd"')
        self.sk = k('`abcd `aa `bb')
        self.ia = array((1, 2, 3, 4, 5), Int)
        self.fa = array((1.0, 2, 3, 4, 5), Float)
        self.ca = array("abcd")
        self.sa = array(("abcd", "aa", "bb"), PyObject)

    def testConversionFromK(self):
        ika = _ktoarray(self.ik)
        fka = _ktoarray(self.fk)
        ska = _ktoarray(self.sk)
        self.failUnlessEqual(ika, self.ia, "integer list")
        self.failUnlessEqual(ika.typecode(), self.ia.typecode(),
                             "integer list type")
        self.failUnlessEqual(fka, self.fa, "double list")
        self.failUnlessEqual(fka.typecode(), self.fa.typecode(),
                             "double list type")
        self.failUnlessEqual(ska, self.sa, "symbol list")
        self.failUnlessEqual(ska.typecode(), self.sa.typecode(),
                             "symbol list type")

    def testDictionaryConversionFromK(self):
        # just test that we don't crash for now
        d = _ksk('.((`a; 1;);(`b; 2;.,(`c;4)))')
        a =  _ktoarray(_ktoarray(d)[1])

    def testConversionToK(self):
        ik = _arraytok(self.ia)
        fk = _arraytok(self.fa)
        sk = _arraytok(self.sa)
        self.failUnlessEqual(_ktoarray(ik), self.ia, "integer list")
        self.failUnlessEqual(_ktoarray(fk), self.fa, "double list")
        self.failUnlessEqual(_ktoarray(sk), self.sa, "symbol list")
        
    def testDataSharing(self):
        ika = _ktoarray(self.ik)
        ika[0] = 3
        ikb = _ktoarray(self.ik)
        self.failUnlessEqual(ikb[0], 3, "integer")

        
################################################################################
if __name__ == '__main__':
    unittest.main()

