"""access k from python


>>> x = k("1 2 3 4 5")
>>> x.asarray()
array([1, 2, 3, 4, 5])


"""
from pyk._k20 import \
     _K, \
     _gi, _gf, _gc, _gs, _gn, _gp, _gk, _ksk, \
     _Ki, _Kf, _Kc, _Ks, kerr
from pyk._nk20 import \
     _ktoarray, _arraytok
import Numeric

__version__ = '$Revision: 1.4 $'

class K(_K):
    """Represents a K object in Python

    The following table summarizes the correspondence between K and
    Python objects

    K code   K type             Python type             Notes
    -----------------------------------------------------------------
       6   null atom                NoneType
       5   dictionary               dict             not implemented   
       4   symbol atom              str
       3   character atom           str
       2   double atom              float
       1   integer atom             int
       0   general list             sequence
      -1   integer list             int sequence
      -2   double list              float sequence
      -3   character list           str sequence
      -4   symbol list              str sequence


    Implementation note: This class is implemented as a subclass of a
    C-coded class _K to allow transparent reimplementation of
    perfomance critical methods in C."""
    
    def __init__(self, pyo, t=None):
        '''
        >>> K((1, 2, 3, 4))
        k("1 2 3 4")
        >>> K(dict(a=(1,2,3), b=(3,4,5)))
        k(""" .((`a
           1 2 3
           )
          (`b
           3 4 5
           )) """)
        >>> K(Numeric.array((1,2,3,4)))
        k("1 2 3 4")
        '''
        if type(pyo) is Numeric.ArrayType:
          k = _arraytok(pyo)  
        elif type(pyo) is _K:
            k = pyo
            if t and t != k.t:
                raise NotImplemented
        else:
            if not t:
                t = _guesst(pyo)
            else:
                if t > 0:
                    pyt = _pyt[t-1]
                    pyo = pyt(pyo)
                elif t < 0:
                    pyt = _pyt[-t-1]
                    pyo = map(pyt, pyo)
            kgen = _kgen(t)
            k = kgen(pyo)
        _K.__init__(self, k)

    def __repr__(self):
        """returns a representation of the K object which can be evaluated by Python"""
        r = _K.__repr__(self)
        if '\n' in r or '"' in r:
            return 'k(""" ' + r + ' """)'
        return 'k("' + r + '")'

    def __str__(self):
        """K objects are printed the way k would print them
        >>> print k('(1 ; 2 3; "abc")')
        (1
         2 3
         "abc")
         """
        return _K.__repr__(self)
    
    def __len__(self):
        """
        >>> len(k('1 2 3 4'))
        4
        """
        if self.t <= 0:
            return self.n
        raise TypeError, "len() of k object of type %s" % self.t

        
    def __call__(self, *args):
        """K objects representing k functions can be called
        >>> f = k('{x+y}')
        >>> print f(1,2)
        3
        """
        # XXX using undocumented feature - k functions have type 10
        t = self.t
        #if t != 10:
        #    raise TypeError("k object of type %d is not callable" % t)
        kk = K((self, K(args)))
        try:
            return K(_ksk('.', kk))
        except kerr, e:
            raise kerr("%s\nvars: %s" % (e, k("!.k")))


    def __eq__(self, other):
        """
        >>> print K(1) == K(2)
        0
        """
        return k('~')(self, other)

    def __ne__(self, other):
        """
        >>> print K(1) != K(2)
        1
        """
        return k('~~')(self, other)

    def __gt__(self, other):
        """
        >>> print K(1) > K(2)
        0
        """
        return k('{x>y}')(self, other)

    def __lt__(self, other):
        """
        >>> print  K(1) < K(2)
        1
        """
        return k('{x<y}')(self, other)

    def __contains__(self, item):
        """
        >>> 2 in k('1 2 3 4')
        True
        """
        return _Ki(k('{(x?y)<#x}')(self, asK(item)))


    def __int__(self):
        return _Ki(self)

    def __float__(self):
        return _Kf(self)

    def __getitem__(self, item):
        """
        >>> x = K((1, 2, 3, 4))
        >>> x[2]
        3
        >>> x = K(dict(a=(1,2,3), b=(3,4,5)))
        >>> #x['a']
        
        """
        if self.t == 5:
            return ktopy(k('{x.%s}'%item)(self))
        return ktopy(k('{x[y]}')(self,asK(item)))

    def __nonzero__(self):
        """returns true for non-empty composite objects, non-zero numbers,
        functions and non-zero characters, false for _n

        >>> k('_n') or k('0') or k('0.0') or  k('`') or k('""') or False
        False
        >>> k('20.5') and k('"x"') and k('{}') and k('0 0 0') and k(".()") and True
        True
        
        """
        return _Ki(_nonzero(self))
                
    def __getattr__(self, a):
        """access to k dictionary items
        >>> x = k('.((`a; 1 2 3);(`b; 3 4 5))')
        >>> x.a
        k("1 2 3")
        """
        if self.t == 5:
            return ktopy(k('{x.%s}'%a)(self))
        else:
            return _K.__getattr__(self, a)


    def __array__ (self, typecode = None):
        """Special hook for Numeric. Converts to Numeric if possible.

        >>> from Numeric import asarray
        >>> x = k("1 2 3 4 5 6")
        >>> asarray(x)
        array([1, 2, 3, 4, 5, 6])

        Note that data is not copied in conversion and the resulting
        numeric array refers to the same data as the original K object.

        >>> asarray(x)[0] = 42
        >>> print x
        42 2 3 4 5 6
        """
        a = _ktoarray(self)
        if typecode is None:
            return a
        return a.astype(typecode)

    asarray = __array__
    def first(self):
        """returns the first element of K object

        >>> x = k('1 2 3 4 5')
        >>> x.first()
        k("1")
        """
        return k('*:')(self)

    def last(self):
        """returns the last element of K object

        >>> x = k('1 2 3 4 5')
        >>> x.last()
        k("5")
        """
        return k('{x[-1+#x]}')(self)
# End of class K
        
def k(s):
    '''creates a K object given its string representation
    >>> k('(1; 2 3; "abc")')
    k(""" (1
     2 3
     "abc") """)
    '''
    try:
        return K(_ksk(s))
    except kerr, e:
        raise kerr("%s in %s (vars:%s)" % (e, s, k("!.k")))




def asK(x, t=None):
    """converts argument to K if necessary"""
    if isinstance(x, K):
        if t and t != x.t:
            raise NotImplemented
        return x
    return K(x, t=t)

def _pass(x): return x
#             -4     -3     -2     -1     0
_getters = (_pass, _pass, _pass, _pass, _pass,
#             1    2    3    4      5          6 
            _Ki, _Kf, _Kc, _Ks, _pass, lambda(x):None)
def ktopy(x):
    return _getters[x.t + 4](x)
    

def _char(x):
    c = str(x)
    if len(c) != 1:
        raise TypeError("char mush have length 1")
    return c

_pyt = (int, float, _char, str)



def _guesst(pyo):
    pyt = type(pyo)
    if pyt is int: return 1
    if pyt is float: return 2
    if pyt is str:return -3
    if pyt is dict: return 5
    if pyo is None: return 6
    # now try to iterate over pyo
    try:
        ts = [type(x) for x in pyo]
    except TypeError:
        pass
    else:
        if len(ts) == 0:
            return 6
        ts0 = ts[0]
        if ts0 is K:
            return 0
        h = True
        for t in ts[1:]:
            if ts0 != t:
                h = False
                break
        if h:
            t0 = _guesst(pyo[0])
            if 1 <= t0 <= 4:
                return -t0
        return 0
    raise TypeError("cannot pass %s of type %s to k" % (pyo, pyt))

def _gendict(pyo):
    values = [K(x) for x in pyo.values()]
    keys = pyo.keys()
    # k function to translate a list of values
    # to a k dictionary
    kfexpr = ('{[%s] .(%s)}'
              % (';'.join(keys),
                 ';'.join(['(`%(key)s;%(key)s)' % {'key': key}
                           for key in keys])))
    return K(_ksk(kfexpr, K(values)))
              
def _gennone(pyo):
    return _gn()

class _genlist:
    def __init__(self, t):
        self._t = t

    def __call__(self, pyo):
        ks = [asK(x, t=-self._t) for x in pyo]
        return K(_gk(*ks))

    
_kg = (_genlist(-4), _gp, _genlist(-2), _genlist(-1), _genlist(0),
       _gi, _gf, _gc, _gs, _gendict, _gennone)

def _kgen(t):
    return _kg[4+t]

def _test():
    print not k('_n')

# Keep hairy k stuff at the end so that we don't mess up formatting
_nonzero = k(r'{((5#(0<#:)),(~0~),(~0.0~),(~"\0"~),(~`~),(#:),(~_n~),{1})[4+4:x][x]}')


def _test():
    import doctest, pyk.K
    return doctest.testmod(pyk.K)

if __name__ == "__main__":
    _test()

