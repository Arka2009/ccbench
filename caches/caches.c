////////////////////////////////////////////////////////////////////////////////
// Caches (time a pointer chase to get cache capacity, cache acces latency) 
// Author: Christopher Celio
// Date  : 2010 Nov 15
//
// Designed to assist in emperically discovering the cache size of a processor
// by repeatedly running this kernel with varying input sizes. 
//
// Create an array of size <input>, where each array element is a pointer to
// another element in the array (circular linked list). Arrange pointers
// randomly to minimize cache-prefetching effects, and walk the array for set
// period of elements to normalize for work/array-size. Thus, any increase in
// run-time should be localized to caching effects.
//
// The input is (at the moment) in "number of elements in the array", 
// which will be rounded to the nearest cache-line amount. This means that
// the graphing code should pay attention to the report file, and not the 
// input file when looking at where to plot a data point. 

//#define DEBUG 
//#define PRINT_ARRAY
 
//force benchmark to run for some minimum duration in cycles
// advantages: hopefully smoothes out noise of tests that run too quickly
// disadvantages: adds additional code to critical loop (empirically unnoticable)
// #define USE_MIN_CYCLES
#define MIN_CYCLES 100000UL
                      

// stride in # of elements, most processors have a 64byte cache line, so 16 elements
// would stride across cache lines.
#define CACHELINE_IN_BYTES_SZ (64)
#define CACHELINE_SZ (CACHELINE_IN_BYTES_SZ/sizeof(uint32_t))

// in # bytes
#define PAGE_SZ (4096) 

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <cclfsr.h>
#include <math.h>
#ifdef GEM5_RV64
 #include "gem5/m5ops.h"
#else
#include "roi_hooks.h"
#include "cpu_uarch.h"
#include "errordefs.h"
#endif

// Global Variables
uint32_t  g_num_cores;
uint32_t  g_num_elements;  
uint32_t  g_num_iterations;
uint32_t  g_performed_iterations;

int g_stride;
int g_run_type;  // choose between stride size, or random stride 

uint64_t run_cycles;

#if (__amd64__) && (USE_PCM)
static struct __eco_roi_stats_struct res;
#endif

// Function Declarations
uint32_t initializeGlobalArrays(uint32_t* arr_n_ptr, uint32_t num_elements, uint32_t stride);
uint32_t threadMain(unsigned lproc_id);

uint32_t printArray(uint32_t iter, uint32_t *arr_ptr, uint32_t num_elements, uint32_t stride);
uint32_t verifyArray(uint32_t *arr_ptr, uint32_t num_elements, uint32_t stride);

int main(int argc, char* argv[])
{
#ifdef DEBUG  
   printf("\nBegin Test\n");
#endif

   if (argc != 4) 
   {
      fprintf(stderr, "argc=%d\n", argc);
      fprintf(stderr, "\n[Usage]: cache_size <Number of Array Elements (Total)> <Number of Iterations> <RunType> \n\n");
      fprintf(stderr, "   RunType encodes the stride (in 32b words) between each memory access. If RunType==0,\
                \n   then a random memory access (on cacheline boundaries) is generated\n\n");
      return 1;
   }

   g_num_elements   = atoi(argv[1]);
   g_num_iterations = atoi(argv[2]);
   g_run_type       = atoi(argv[3]);


#ifdef DEBUG
   fprintf(stderr, "Size of the array     = %d\n",  g_num_elements);
   fprintf(stderr, "Number of Iterations  = %d\n",  g_num_iterations);
   fprintf(stderr, "Stride size (run type)= %d\n\n",g_run_type);
#endif

   unsigned lproc_id = 1;
   #if (__amd64__) && (USE_PCM)
   affinity_set_cpu2(lproc_id);
   __eco_init(lproc_id);
   #elif __amd64__
   affinity_set_cpu2(lproc_id);
   #endif

   if(g_run_type > 0)
      g_stride = g_run_type;

   // random access should hit cache-line addresses, to eliminate
   // spatial locality and force more misses.
   if(g_run_type == 0)
   {
      g_stride = CACHELINE_SZ;
   }

   // make the array size a multiple of the stride; round up
   // actually stride*2 so we can insure that no consecutive accesses are adjacent, and this makes implementing this easier
   if (g_num_elements % (2*g_stride) != 0)
      g_num_elements = g_num_elements + ((2*g_stride) - (g_num_elements % (2*g_stride)));

#ifdef DEBUG
   printf(" Adjusted Num of Elements: %d\n\n", g_num_elements);
#endif

   // this volatile ret_val is crucial, otherwise the entire run-loop 
   // gets optimized away! :(
   uint32_t volatile ret_val = threadMain(lproc_id);  

#ifdef PRINT_SCRIPT_FRIENDLY
   // fprintf(stdout, "App:[caches],NumThreads:[%d],AppSize:[%d],Time:[%g], TimeUnits:[Cycles Per Iteration],NumIterations:[%u],RunType:[%d]\n",
   //   
   // App:%s,NumThreads:%d,NumElements:%d,Time:%g,NumIterations,RunType
   #if (__amd64__) && (USE_PCM)
   fprintf(stdout,"%s,%d,%d,%g,%d,%lu,%lu,%lu,%lu,%d\n",\
      "CacheAccLatency",\
      g_num_cores,\
      g_num_elements,\
      ((double) run_cycles / (double) g_performed_iterations),\
	   g_performed_iterations,\
      res.l2_hits,\
      res.l2_miss,\
      res.l3_hits,\
      res.l3_miss,\
      g_run_type);
   #else
   fprintf(stdout,"%s,%d,%d,%g,%d,%d,%d,%d,%d,%d\n",\
      "CacheAccLatency",\
      g_num_cores,\
      g_num_elements,\
      ((double) run_cycles / (double) g_performed_iterations),\
	   g_performed_iterations,\
      -1,\
      -1,\
      -1,\
      -1,\
      g_run_type);
   #endif
#endif

#ifdef DEBUG
  fprintf(stderr, "Done. Exiting...\n\n");
#endif

  return 0;
}


uint32_t threadMain(unsigned lproc_id)
{
   uint32_t* arr_n_ptr;

   // pad out memory to next PAGE_SZ to make initialization easier...
   uint32_t num_total_elements_per_page = PAGE_SZ/sizeof(uint32_t);
   int num_elements_allocated = g_num_elements + (num_total_elements_per_page - (g_num_elements % num_total_elements_per_page));
   
   
   arr_n_ptr   = (uint32_t *) malloc(num_elements_allocated * sizeof(uint32_t));
   
   initializeGlobalArrays(arr_n_ptr,
                        g_num_elements,
                        g_stride);
   
   
   double const clk_freq = 1e9;
   g_performed_iterations = g_num_iterations;


   /** CRITICAL SECTION : START **/
   #if (__amd64__) && (USE_PCM)
   core_counter_state_ptr_t start = __eco_roi_begin(lproc_id);
   #elif GEM5_RV64
   m5_reset_stats(0,0);
   #else
   uint64_t start_cycles = __eco_rdtsc(); //cc_get_cycles(clk_freq);
   #endif

//TODO time code ahead of time, and set num_iterations based on that...
#ifdef USE_MIN_CYCLES
   // run for g_num_iterations or until MIN_CYCLES reached, whichever comes last
   cccycles_t estimated_end_cycles = start_cycles + MIN_CYCLES;
    
   intptr_t idx = 0;

   // we have to run for AT LEAST g_num_iterations, so dont muck up critical section 
   // with unnecessary code
   for (uint32_t k = 0; k < g_num_iterations; k++)
   {
      idx = arr_n_ptr[idx];
   }

   while (cc_get_cycles(clk_freq) < estimated_end_cycles)
   {
      g_performed_iterations += g_num_iterations;
      for (uint32_t k = 0; k < g_num_iterations; k++)
      {
         idx = arr_n_ptr[idx];
      }
   }

#else

   // run for g_num_iterations...

   intptr_t idx = 0;

   for (uint32_t k = 0; k < g_num_iterations; k++)
   {
      idx = arr_n_ptr[idx];
   }

#endif 

   
   /** CRITICAL SECTION : STOP **/
   #if (__amd64__) && (USE_PCM)
   core_counter_state_ptr_t stop = __eco_roi_end(lproc_id);
   res = __eco_counter_diff(stop, start);
   run_cycles = res.tsc;
   __eco_reset(lproc_id);
   #elif GEM5_RV64
   m5_dump_stats(0,0);
   #else
   uint64_t stop_cycles = __eco_rdtsc();
   run_cycles = stop_cycles - start_cycles;
   #endif

#ifdef DEBUG
   fprintf(stderr, "Total_Cycles               : %lu\n", run_cycles);
#endif

   // prevent compiler from removing ptr chasing...
   // although the receiver must put idx into a volatile variable as well!
   return (uint32_t) idx; 
}


//generate a random linked list of a PAGE_SZ, starting at address arr_n_ptr[page_offset]
//the calling function is in charge of stitching the page together with the other pages
//num_accesses is the number of pointers we are going to add (not the number of
//elements spanned by the page)
//returns the index to the last index (to help the calling function stitch pages together)
//interleaving_space is the size between entries (i.e., 2 means generated lines are even/odd)
//assumes num_accesses can be divided by interleaving_space
uint32_t initializePage(uint32_t* arr_n_ptr, uint32_t page_offset, uint32_t num_accesses, uint32_t stride, uint32_t interleaving_space)
{
   if (interleaving_space > 2)
      printf("ERROR: unsupported interleaving_space\n");

#ifdef DEBUG
   printf("\n--(offset = %d) Generating Page-- num_el=%d, pageSz=%d, strd=%d\n\n",
      page_offset, num_accesses, num_accesses*stride*sizeof(int32_t), stride);
#endif

   cc_lfsr_t lfsr;
   uint32_t lfsr_init_val = 1; //TODO provide different streams different starting positions?
   //(-1) because we are going to loop through twice, once for odd and once for even entries 
   uint32_t lfsr_width = (log(num_accesses) / log(2)) - (interleaving_space - 1); //TODO EVENODD
   // fprintf(stderr,"PageOffset=%d," \
   //                "num_accesses=%d," \
   //                "lfsr_width=%d\n", \
   //                page_offset,num_accesses,lfsr_width);

   uint32_t max_accesses = (0x1 << lfsr_width)*interleaving_space;

   // special case to handle non-powers of 2 num_accesses
   if (max_accesses < num_accesses)
      lfsr_width+=interleaving_space;


   uint32_t idx;
   uint32_t curr_idx, next_idx;

   curr_idx = page_offset;

   // special cases to handle the VERY small pages (too small to use a LFSR)
   if (lfsr_width < 2)
   {
      // lfsr width too small, forget about randomizing this page...
      for (int i=0; i < num_accesses-1; i++)
      {
         arr_n_ptr[curr_idx] = curr_idx + stride;
         curr_idx += stride; 
      }
      
      arr_n_ptr[curr_idx] = page_offset;
      return curr_idx;
   }

   
   cc_lfsr_init(&lfsr, lfsr_init_val, lfsr_width);


   // "-1" because we do the last part separately (lfsr's don't generate 0s)
   for (int i=0; i < num_accesses-interleaving_space; i+=interleaving_space)
   {
      //TODO do both even, odd at same time...
      next_idx = lfsr.value * (interleaving_space*stride) + page_offset; //TODO EVENODD
      
      arr_n_ptr[curr_idx] = next_idx;
      if (interleaving_space == 2)
         arr_n_ptr[curr_idx+stride] = next_idx+stride;


#ifdef DEBUG
      printf("   array[%4d] = %4d,   lfsr.value = %4d, i = %4d EVEN\n", 
                  curr_idx, arr_n_ptr[curr_idx], lfsr.value, i); 
      if (interleaving_space == 2)
         printf("   array[%4d] = %4d,   lfsr.value = %4d, i = %4d ODD\n", 
                     curr_idx+stride, arr_n_ptr[curr_idx+stride], lfsr.value, i); 
#endif

      curr_idx = next_idx;
      lfsr.value = cc_lfsr_next(&lfsr);

      //handle non powers of 2 num_accesses
      while (lfsr.value > ((num_accesses-1)/interleaving_space))
      {
         lfsr.value = cc_lfsr_next(&lfsr);
      }
   }

   //handle last index, wrap back to the start of the page
   //let calling function stitch to next pages...
   if (interleaving_space==1)
   {
      arr_n_ptr[curr_idx] = page_offset;
   }
   else
   {
      arr_n_ptr[curr_idx] = page_offset+stride;
      arr_n_ptr[curr_idx+stride] = page_offset;
#ifdef DEBUG
      printf("  *array[%4d] = %4d,   lfsr.value = %4d, \n", 
                  curr_idx, arr_n_ptr[curr_idx], lfsr.value); 
      printf("  *array[%4d] = %4d,   lfsr.value = %4d, \n", 
                  curr_idx+stride, arr_n_ptr[curr_idx+stride], lfsr.value); 
#endif
      curr_idx += stride;
   }


   return curr_idx;
}


// this is the future initializeGlobalArrays, but it has one or two bugs so it's commented out for now
// it will more accurately provide the L3 access times on some processors
uint32_t initializeGlobalArrays(uint32_t* arr_n_ptr, uint32_t num_elements, uint32_t stride)
{
   // create a strided access array (no randomization)
   if(g_run_type != 0)
   {
      for (uint32_t i = 0; i < g_num_elements-1; i++)
      {
         arr_n_ptr[i % num_elements] = (i+stride) % num_elements;
      }
   }
   else
   {
      //randomize array on cacheline strided boundaries
#ifdef DEBUG
      printf("-==== Begin two-level randomization of the chase array ====-\n");
#endif
 
      // check to see if the array is 1-element long...
      if (num_elements == stride)
      {
#ifdef DEBUG
         printf("Array is 1 element long, returning....\n");
#endif
         // points back on itself
         arr_n_ptr[0] = 0;
         return 0;
      }
      
      uint32_t num_elements_per_page = PAGE_SZ/sizeof(uint32_t);

#ifdef DEBUG
      printf("num_elements_per_page = %lu\n", PAGE_SZ/sizeof(uint32_t));
#endif
                    
      
      uint32_t last_idx;

      // two-level randomization (this is the outer level for-loop)
      // for each page...
      for(int i=0; i < num_elements; i+=num_elements_per_page)
      {
// #ifdef DEBUG
//          printf("Starting New Page: i=%d, page offset=%d\n", i, i); 
// #endif
         uint32_t page_offset = i;

         //TODO test for partial page (which is always the last page)...
         if ((int32_t) i >= (int32_t) (num_elements - num_elements_per_page)  //is last page?
            && ((num_elements - i) % num_elements_per_page != 0))  //and last page is partial
         {
            num_elements_per_page = num_elements - i; //num_elements % num_elements_per_page;
         }
         last_idx = initializePage(arr_n_ptr, page_offset, num_elements_per_page/stride, stride, 1);
         arr_n_ptr[last_idx] = (i+num_elements_per_page);
      }
      arr_n_ptr[last_idx] = 0;
   } //end of randomized array creation

#ifdef PRINT_ARRAY
   printArray(0, arr_n_ptr, num_elements, stride);
#endif

   verifyArray(arr_n_ptr, num_elements, stride);
  
   return 0;
}


uint32_t printArray(uint32_t iter, uint32_t *arr_ptr, uint32_t num_elements, uint32_t stride )
{
   fprintf(stderr, "Chasing through Array (run thru for 2x*num_el/stride) after iteration: %d\n", iter);
   uint32_t idx = 0;
   for (uint32_t i = 0; i < 2*num_elements/stride; i++)
   {
      fprintf(stderr, "%3d, ", arr_ptr[idx]);
      idx = arr_ptr[idx];
   }
   fprintf(stderr, "\n");
   
   fprintf(stderr, "arr_ptr: ");
   for (uint32_t i = 0; i < num_elements; i++)
   {
      if (i % stride == 0) fprintf(stderr, "\n");
      fprintf(stderr, "%3d, ", arr_ptr[i]);
   }
   fprintf(stderr, "\n\n\n");

   return 0;
}


uint32_t verifyArray(uint32_t *arr_ptr, uint32_t num_elements, uint32_t stride)
{
   uint32_t idx = 0;
   uint32_t counter = 0;

   uint32_t first_idx = 0;
   uint32_t finished = 0;

   uint32_t* verify_array;
   verify_array = (uint32_t *) calloc(g_num_elements, sizeof(uint32_t));
   
   while (!finished)
   {
      verify_array[idx] = verify_array[idx] + 1;
      counter++;
      idx = arr_ptr[idx];

      if(counter >= num_elements || idx == first_idx)
         finished = 1;
   }

   uint32_t error = 0;

   for (uint32_t i = 0; i < num_elements; i+=stride)
   {
      if(verify_array[i] != 1)  
      {
#ifdef DEBUG
         fprintf(stderr, "Error at Element [%d], accessed %d times\n", 
            i,
            verify_array[i]
            );
#endif
         error = 1;
      }
   }

   if (counter==(num_elements/stride) && !error) {
#ifdef DEBUG
      fprintf(stderr,"Array verified\n\n");
#else
      int x=0; //here for compiler reasons
#endif
   } 
   else 
   {
      fprintf(stderr,"Error: Array size:%d, Stride size:%d, Loops in:%d (wants to loop in %d)\n\n",
         num_elements,
         stride,
         counter,
         num_elements/stride);
   }

   return error | (counter==(num_elements/stride));
}