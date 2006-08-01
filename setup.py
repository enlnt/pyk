from distutils.core import setup, Extension
import os
####
kdir = os.path.join(os.getenv('HOME'), 'k')
klib = kdir
kinclude = kdir
xlib = '/usr/X11R6/lib'
####  
module_k20 = Extension('_k20',
                       sources=[ os.path.join('Src','_k20.c') ],
                       include_dirs = [ kinclude ],
                       libraries = ['k20', 'X11'],
                       library_dirs = [klib, xlib],
                       runtime_library_dirs = [klib, xlib]
                       )

module_nk20 = Extension('_nk20',
                        sources=[os.path.join('Src','_nk20.c')],
                        include_dirs = [ kinclude ],
                        libraries = ['k20', 'X11'],
                        library_dirs = [klib, xlib],
                        runtime_library_dirs = [klib, xlib]
                        )

setup(name = 'Pyk',
      ext_package='pyk',
      ext_modules = [ module_k20, module_nk20 ],
      py_modules = ['pyk.K', 'pyk.ksql'],
      package_dir=dict(pyk='Lib'))

                                
