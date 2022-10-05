import os
import sys
import numpy as np
import pprint as pp
import pandas as pd
from subprocess import Popen, PIPE

# All numerical values in bytes
ELEMWIDTH=4
NUMITERS=1000
CACHEDETAILS={
    'ICX' : {
        'Desc' : 'Intel(R) Xeon(R) Gold 6326 CPU @ 2.90GHz',
        'L1' : 48*1024,
        'L2' : 1280*1024,
        'L3' : 24*1024*1024,
        'L4' : 48*1024*1024,
        'L5' : 96*1024*1024,
        'ClockFreq': 2.9e9
    },
    'GEM5_RUBY_RISCV' : {
        'Desc' : 'Configurable gem5 model with Ruby memory system',
        'L1' : 48*1024,
        'L2' : 1280*1024,
        'L3' : 24*1024*1024,
        'ClockFreq': 1e9
    }
}

def genWorkingSetSize(impl):
    allWorkingSets=[]
    fracs=list(np.linspace(0.1,1.2,10))
    for cacheHeirarchy in ['L1','L2','L3','L4','L5'] :
        cacheSz=int((CACHEDETAILS[impl][cacheHeirarchy])/ELEMWIDTH)
        allWorkingSets+=[int(f*cacheSz) for f in fracs]
    return allWorkingSets

def main(impl):
    path=os.getcwd()
    bin=f'{path}/caches'
    # allWorkinSet=genWorkingSetSize(impl)
    allWorkinSet=[4,16,32,64,512,1024,1600,2048,2196,2362,2500,2800,3060,3400,3800,4192,5000,6000,7000, 8192,9000,\
        10000,12000,16384,18000,20000,22000, 24576,29696, 32768, 35840,37000, 40000, 50100, 60000, 62000,65536,68000,  70000,  75100,100000,\
            131072,160100,190100,229376,262144,300100,400100,500100,524287,524288,524289,600100,750100, 786432, 917504,\
         996148, 1048576, 1101004 ,1572864, 1966080 ,2097152, 2228224 ,2400000, 2621440,3145728, 4194304,8388608,10000000, 13000000,16777216,\
            20000000,25000000,33554432,40000000,50000000,67108864,100000000,134217728,150000000,200000000,250000000,268435456]
    #33m is 128MB
    logFile=f'{path}/CacheAccessLat.csv'
    with open(logFile,'w') as lf:
        print(f'App,NumThreads,NumElements,Time,NumIterations,RunType',file=lf)
        for workingSet in allWorkinSet:
            cmdList = [bin, f'{workingSet}', f'{NUMITERS}','0']
            cacheLatProc = Popen(cmdList,stdin=PIPE,stdout=lf,stderr=PIPE,cwd=path)
            cacheLatProc.wait()
            lf.flush()
            print(f'{impl}@Completed {workingSet}')

if __name__=="__main__":
    main('ICX')