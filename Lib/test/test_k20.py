import unittest
__version__ = '$Revision: 1.3 $'
from pyk._k20 import \
     _gc,        \
     _gf,        \
     _gi,        \
     _gk,        \
     _gn,        \
     _gp,        \
     _gs,        \
     _gtn,       \
     _ksk,       \
     _K,         \
     _Kc,        \
     _Kf,        \
     _Ki,        \
     _Ks,        \
     _sKc,       \
     _sKf,       \
     _sKi,       \
     _sKs,       \
     kerr

class IntPykTestCase(unittest.TestCase):
    def setUp(self):
        self.i = _gi(5)
        self.f = _gf(5.0)
        self.c = _gc('c')
        self.s = _gs('abc')
        self.n = _gn()
        self.il = _gtn(-1, 5)
        self.fl = _gtn(-2, 0) 

    def testType(self):
        "Test that pyk object got the right k type"
        it = self.i.t
        ft = self.f.t
        ct = self.c.t
        st = self.s.t
        nt = self.n.t
        ilt = self.il.t
        flt = self.fl.t
        self.failIf(it != 1, 'got %s, expected 1' % it)
        self.failIf(ft != 2, 'got %s, expected 2' % ft)
        self.failIf(ct != 3, 'got %s, expected 3' % ct)
        self.failIf(st != 4, 'got %s, expected 4' % st)
        self.failIf(nt != 6, 'got %s, expected 6' % nt)
        self.failIf(ilt != -1, 'got %s, expected -1' % ilt)
        self.failIf(flt != -2, 'got %s, expected -2' % flt)
        
    def testValue(self):
        "Test that pyk object got the value with which it was created"
        i=_Ki(self.i)
        f=_Kf(self.f)
        c=_Kc(self.c)
        s=_Ks(self.s)
        self.failIf(i != 5, 'assigned a value 5 but came out as %s' % i)
        self.failIf(f != 5.0, 'assigned a value 5.0 but came out as %s' % f)
        self.failIf(c != 'c', 'assigned a value \'c\' but came out as %s' % c)
        self.failIf(s != "abc", 'assigned a value "abc" but came out as %s' % s)

    def testReassign(self):
        "Test that we can change the value of a pyk object"
        ki = _sKi(self.i, 10)
        i = _Ki(ki)
        kf = _sKf(self.f, 10.0)
        f = _Kf(kf)
        kc = _sKc(self.c, 'd')
        c = _Kc(kc)
        ks = _sKs(self.s, 'xyz')
        s = _Ks(ks)
        self.failIf(i != 10, 'reassigned a value 10 but came out as %s' % i)
        self.failIf(f != 10.0, 'reassigned a value 10.0 but came out as %s' % f)
        self.failIf(c != 'd', 'reassigned a value \'d\' but came out as %s' % c)
        self.failIf(s != "xyz", 'reassigned a value "xyz" but came out as %s' % s)

    def test_ksk(self):
        k = _ksk("1+2")
        p = _Ki(k)
        self.failIf(p != 3, 'got %s, expected 3' % p)
        k = _ksk("{x+y}", _gk(self.f, self.i))
        p = _Kf(k)
        self.failIf(p != 10.0, 'got %s, expected 10.0' % p)
        self.failUnlessRaises(kerr, _ksk, ')')

    def test_dict(self):
        k = _ksk(".((`a;1;.,(`z;42));(`b;2))")
        a = _Ki(k['a'])
        self.failIf(a != 1, 'got "%s" instead of 1' % a)
        k['a'] = _gi(2)
        a = _Ki(k['a'])
        self.failIf(a != 2, 'got "%s" instead of 2' % a)
        z = _Ki(k['a.']['z'])
        self.failIf(z != 42, 'attribute - got %s, expected 42')

    def test_gp(self):
        # empty string
        a = str(_gp(""))
        b = '""'
        self.failIf(a != b, "got %r instead of %r" % (a, b))
        # embedded \0
        a = str(_gp("\0"))
        b = r',"\000"'
        self.failIf(a != b, "got %r instead of %r" % (a, b))
        # whatever
        a = str(_gp("\000\001whatever\000string"))
        b = r'"\000\001whatever\000string"'
        self.failIf(a != b, "got %r instead of %r" % (a, b))
################################################################################
if __name__ == '__main__':
    unittest.main()
