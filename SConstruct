lib='comms'
libs=['Ws2_32.lib'] #libs=['']
testlibs=['comms']

# Fixed constants
libdir='lib'
bindir='bin'
testdir='test'
srcdir='src'
target=libdir+'/'+lib
testTarget=bindir+'/test-'+lib
extensions='*.c'
headers=' *.h'
api='src/conn.h'
map='bin/comms.map'

# Fixed code
import os
import sys
import glob
import shutil
#def runUnitTest(env,target,source):
#	import subprocess
#	app=str(source[0].abspath)
#	if not subprocess.call(app):
#		open(str(target[0],'w').write("PASSED\n")

srcs=[]
for extension in extensions.split():
    srcs+=glob.glob(os.path.join(srcdir,extension))
testsrcs=[]
for extension in extensions.split():
    testsrcs+=glob.glob(os.path.join(testdir,extension))

env = Environment()

# Fix bad vc9 detection using the environment variables instead
#include_path=os.environ['INCLUDE']
#env.PrependENVPath('INCLUDE', include_path)
#lib_path=os.environ['LIB']
#env.PrependENVPath('LIB', lib_path)
   
#env.Append(LIBPATH='#/'+libdir)
#env.Append(LIBS = libs)

# Main target: the DLL
env.Append(LINKFLAGS='/MAP:lib/conn.map');
mainTarget=env.SharedLibrary(target,srcs, LIBS = libs, LIBPATH='#/'+libdir)

# Test target: unit test program (depends on main target) 
test=env.Program(testTarget,testsrcs, LIBS=testlibs, LIBPATH='#/'+libdir)
#env.AddPostAction(test,test[0].abspath)
Depends(test, mainTarget)
env.AlwaysBuild(test)
#Command("test.passed",'test',runUnitTest)

# Binary installation target depends on main target
dlls=glob.glob(libdir+'/*.dll');
installation=Install(bindir,dlls)
Depends(installation,mainTarget)
    
# Source Zip target (depends on main target)
hdrs=[]
for header in headers.split():
    hdrs+=glob.glob(os.path.join(srcdir,header))
for header in headers.split():
    hdrs+=glob.glob(os.path.join(testdir,header))
allsources=['SConstruct']+testsrcs+srcs+hdrs
srczip=Zip(lib+".zip",allsources)
Depends(srczip,mainTarget)

# Binary Zip target depends on main target
allbins=['src/conn.h']+glob.glob(os.path.join('bin','*.dll'))+glob.glob(os.path.join('bin','*.so'))
binzip=Zip(lib+".bin.zip",allbins)
Depends(binzip,mainTarget)

# Latest release installation target depends on main target
latest=Install('latest', [mainTarget, api]) # ,map

