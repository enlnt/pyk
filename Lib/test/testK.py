import unittest

from pyk.K import \
     k,\
     K,\
     ktopy
from Numeric import *


class KTestCase(unittest.TestCase):
    def setUp(self):
        self.ik = k('1 2 3 4 5')
        self.fk = k('1.0 2 3 4 5')
        self.ck = k('"1234"')
        self.ia = array((1, 2, 3, 4, 5), Int)
        self.fa = array((1.0, 2, 3, 4, 5), Float)
        self.ca = "1234"

    def testConversion(self):
        "Test k object to Numeric array conversion"
        ika = asarray(self.ik)
        fka = asarray(self.fk)
        self.failUnlessEqual(ika, self.ia, "integer list")
        self.failUnlessEqual(ika.typecode(), self.ia.typecode(),
                             "integer list type")
        self.failUnlessEqual(fka, self.fa, "double list")
        self.failUnlessEqual(fka.typecode(), self.fa.typecode(),
                             "double list type")
        # test scalars
        ko = K(1)
        a = asarray(ko)
        self.failUnlessEqual(a, 1, "integer scalar")
        ko = K(1.0)
        a = asarray(ko)
        self.failUnlessEqual(a, 1.0, "integer scalar")

        # test strings
        cka = ktopy(self.ck)
        self.failUnlessEqual(cka, self.ca, "string %s converts from k as %s"\
                             %(self.ca,cka))

    def testOperators(self):
        x = y = self.ik
        self.failUnless(x == y, "k-k equals")
        y = self.ia
        self.failUnless(x == y, "k-py equals")
        
    def testDataSharing(self):
        "Test data sharing"
        ika = asarray(self.ik)
        ika[0] = 3
        self.failUnlessEqual(self.ik[0], 3, "integer")
        fka = asarray(self.ik, Float)
        fka[0] = 5
        self.failUnlessEqual(self.ik[0], 3, "unshared")

    def test_dict(self):
        kd = k('.((`a;1);(`b;2))')
        #print type(kd['a'])
        
################################################################################
if __name__ == '__main__':
    unittest.main()
