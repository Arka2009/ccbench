import os
import sys
import numpy as np
import pprint as pp
import pandas as pd
from subprocess import Popen, PIPE
import argparse

parser = argparse.ArgumentParser(description = 'cache access/latency test')
parser.add_argument("type", help = "test cache access or latency") # 0 for cache miss/hit, 1 for cache latency
args = parser.parse_args()

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
    },
    'ARM' : {
        'Desc' : 'ARM Cortex A55',
        'L1' : 256*1024,
        'L2' : 1024*1024,
        'L3' : 3*1024*1024,
        'ClockFreq': 2.9e9
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
    allWorkinSet=[4,16,32,64,512,1024,1600,2048,2196,2362,2500,2800,3060,3400,3800,4192,5000,6000,7000, 8192,9000, 9500,\
        10000,10500,11000,12000,14000,16384,18000,20000,22000, 24576,29696, 32768, 35840,37000, 40000, 45000, 50100,55000, 60000, 62000,65536,68000,  70000, 72000,  75100,\
        78000, 80000,83000,85000,88000,90000,93000,95000,98000,100000, 110000, 120000,\
            131072,160100,190100,229376,262144,300100,400100,450000,500100,524287,524288,524289,600100, 670000,750100, 786432,850000, 917504,\
         996148, 1048576, 1101004 ,1572864, 1966080 ,2097152, 2228224 ,2400000, 2621440,3145728, 4194304,6000000,8388608,10000000, 13000000,16777216,\
           20000000,25000000,33554432,40000000,45000000,50000000,67108864]
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
