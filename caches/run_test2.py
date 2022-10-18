import os
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


def main(impl):
    path=os.getcwd()
    bin=f'{path}/caches.AMD64'
    # allWorkinSet=[4,16,32,64,512,1024,1600,2048,2196,2362,2500,2800,3060,3400,3800,4192,5000,6000,7000, 8192,9000,10000,12000,16384,18000,20000,22000, 24576,29696, 32768, 35840,37000, 40000, 50100, 60000, 62000,65536,68000,  70000,  75100,100000,131072,160100,190100,229376,262144,300100,400100,500100,524287,524288,524289,600100,750100, 786432, 917504, 996148, 1048576, 1101004 ,1572864, 1966080 ,2097152, 2228224 , 2621440,3145728, 4194304,8388608,16777216,33554432]
    lower, upper = (4000000, 13000000)
    length = 100
    allWorkinSet=[lower + x*(upper-lower)/length for x in range(length)]
    logFile=f'{path}/CacheAccessLat.csv'
    lfE = open(f'{path}/CacheAccessLat.log','w')
    with open(logFile,'w') as lf:
        print(f'App,NumThreads,NumElements,Time,NumIterations,l2_hits,l2_miss,l3_hits,l3_miss,RunType',file=lf)
        lf.flush()
        for workingSet in allWorkinSet:
            cmdList = [bin, f'{workingSet}', f'{NUMITERS}','0']
            cacheLatProc = Popen(cmdList,stdin=PIPE,stdout=lf,stderr=lfE,cwd=path)
            cacheLatProc.wait()
            lf.flush()
            lfE.flush()
            lfE.write('--------------------------------------------------------\n\n')
            print(f'{impl}@Completed {workingSet}')
    lfE.close()

if __name__=="__main__":
    main('ICX')