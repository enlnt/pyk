"""ksql access utilities

Loading this module makes kdb library available to the embeded k.

For example, ds function converts strings to dates
>>> k('.d.ds["2003-08-04"]')
k("-11473")
"""
__version__ = '$Revision: 1.3 $'
from K import k
from datetime import date,time

def ksql(q,*args):
    """provides a way to call ksql function without typing .d.r

    >>> ksql("date'2003-08-04'")
    k("-11473")

    Parametric queries are supported
    
    >>> ksql("asc ?", [1, 2, 3, 5])
    k("1 2 3 5")
    """
    if args:
        return _d_r((q,args))
    else:
        return _d_r(q)

k(r'\l db')
_d_r = k('.d.r')
                
################################################################################
def _test():
    import doctest, pyk.ksql
    return doctest.testmod(pyk.ksql)

if __name__ == "__main__":
    _test()
