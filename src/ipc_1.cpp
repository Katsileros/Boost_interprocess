/*!
 *  \brief     Passing data from one process to another using boost interprocess and multithreading
 *  \author    Katsileros Petros
 *  \date      02/02/2016
 *  \copyright
 */

#include <iostream>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

#include <boost/interprocess/sync/named_condition.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

#include <boost/thread.hpp>

#include <triangles.h>

int numData = 15e06;
int extraMem = 65536;

void readDataShm();

int main(int argc, char *argv[])
{
  boost::interprocess::named_condition named_cnd(boost::interprocess::open_only, "cnd");
  boost::interprocess::named_mutex named_mtx(boost::interprocess::open_only, "mtx");

  try
     {
       //Open managed shared memory
       boost::interprocess::managed_shared_memory shmMsg(boost::interprocess::open_only, "MsgSharedMemory");

       //Find the intmsg
       int *intmsg = shmMsg.find<int> ("IntMSG").first;

       // Send signal for Solve request
       *intmsg = 5;

       // Signal ipc_1 and wait
       std::cout << "ipc_1 send initial signal and waiting ..." << std::endl;
       named_cnd.notify_all();

       try
          {
            boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(named_mtx,boost::interprocess::try_to_lock);
            named_cnd.wait(lock);
         }
         catch(boost::interprocess::interprocess_exception &ex)
         {
            std::cout << "Exception Error:  " << ex.what() << std::endl;
            return 1;
         }

       int stop_flag = 1;
       while(stop_flag)
       {

         boost::this_thread::sleep_for(boost::chrono::seconds(1));

         *intmsg = (std::rand() % 10) + 1;
         // Send message to take back the data
         while(*intmsg == 5) { *intmsg = (std::rand() % 10) + 1; }
         //*intmsg = 3;

         // Signal ipc_1 and wait
         std::cout << "ipc_1 request: " << *intmsg << std::endl;

         try
          {
            //if(lock)
            //    std::cout << "ipc_1 waiting ..." << std::endl;
            //else
            //    std::cout << "lock error" << std::endl;
            //std::cout << " ====================== here in working (LOCKED) ====================" << std::endl;
            boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(named_mtx,boost::interprocess::try_to_lock);
            named_cnd.notify_all();
            named_cnd.wait(lock);
         }
         catch(boost::interprocess::interprocess_exception &ex)
         {
            std::cout << "Exception Error:  " << ex.what() << std::endl;
            return 1;
         }

        //Find the intmsg
        //intmsg = shmMsg.find<int> ("IntMSG").first;
        std::cout << "Read intmsg: " << *intmsg << std::endl;

        switch(*intmsg)
        {
            case(99):
            {
                std::cout << "Data is not ready yet." << std::endl;
            }
            break;

            case(999):
            {
                std::cout << "Call readDataShm function" << std::endl;
                readDataShm();

                // Send message to take back the data
                *intmsg = 100;

                // Signal ipc_0 to test data
                named_cnd.notify_all();
                stop_flag = 0;
            }
            break;

            default:
            {
                std::cout << "Unrecognized command" << std::endl;
            }
        }

       }
      }

      catch(...){
          boost::interprocess::shared_memory_object::remove("DataSharedMemory");
          throw;
      }

    //boost::interprocess::named_mutex::remove("mtx");
}

void writeShmem(std::vector<Triangle> vec);

void readDataShm()
{
         boost::interprocess::managed_shared_memory segment
         (boost::interprocess::open_only
         ,"DataSharedMemory"); //segment name

         //Alias an STL compatible allocator of ints that allocates ints from the managed
         //shared memory segment.  This allocator will allow to place containers
         //in managed shared memory segments
         typedef boost::interprocess::allocator<Triangle,boost::interprocess::managed_shared_memory::segment_manager> ShmemAllocator;

         //Alias a vector that uses the previous STL-like allocator
         typedef std::vector<Triangle, ShmemAllocator> MyVector;

         //Initialize shared memory STL-compatible allocator
         const ShmemAllocator alloc_inst(segment.get_segment_manager());

         //Construct a shared memory
         MyVector *myvector = segment.find<MyVector>("DataVector").first;

         std::vector<Triangle> vecTr;
         // Read the data
         for(int i=0;i<numData;i++) {
             vecTr.push_back( myvector->at(i) );
         }

         //printTriangle( vecTr );
         //printf("\n");

         // Write data back to another shared segment
         writeShmem(vecTr);

}

void writeShmem(std::vector<Triangle> vec)
{
    using boost::this_thread::get_id;

    std::cout << "=====(working inside working)=====>  Writing data to shared memory" << std::endl;

    try
    {

            //Shared memory front-end that is able to construct objects
            //associated with a c-string. Erase previous shared memory with the name
            //to be used and create the memory segment at the specified address and initialize resources
            boost::interprocess::shared_memory_object::remove ("workingDataSharedMemory");
            boost::interprocess::managed_shared_memory segment
                    (boost::interprocess::create_only
                    ,"workingDataSharedMemory" //segment name
                    , 32*numData + (numData*sizeof(Triangle)) + extraMem);

            //Alias an STL compatible allocator of ints that allocates ints from the managed
            //shared memory segment.  This allocator will allow to place containers
            //in managed shared memory segments
            typedef boost::interprocess::allocator<Triangle,boost::interprocess::managed_shared_memory::segment_manager> ShmemAllocator;

            //Alias a vector that uses the previous STL-like allocator
            typedef boost::interprocess::vector<Triangle, ShmemAllocator> MyVector;

            //Initialize shared memory STL-compatible allocator
            const ShmemAllocator alloc_inst(segment.get_segment_manager());

            //Construct a shared memory
            MyVector *vecTr = segment.construct<MyVector>("workingDataVector") //object name
                                            (alloc_inst); //first ctor parameter

            //std::size_t free_memory_after_allocation = segment.get_free_memory();
            //std::cout << "memory: : " << (float)(free_memory_after_allocation / (1024*1024)) << " MB" << std::endl;

            //std::cout << "number of Triangles: " << (int) (numData/1000) << " K" << std::endl;
            //std::cout << "sizeof(Triangle): " << sizeof(Triangle) << " Bytes" << std::endl;
            //std::cout << "size of data: " << (float) (sizeof(Triangle)*numData / (1024*1024)) << " MB" << std::endl;

            //std::cout << "vec size: " << vec.size() << std::endl;

            std::vector<Triangle> triang;
            //Insert data in the vector
            for(int i=0; i < numData; ++i){
                  vecTr->push_back(Triangle());
                  vecTr->at(i) = vec[i];

                  triang.push_back(vecTr->at(i));
            }

            //printTriangle( triang );
            //printf("\n");

            //free_memory_after_allocation = segment.get_free_memory();
            //std::cout << "free memory after vector initialization: " << (float) (free_memory_after_allocation / (1024*1024)) << " MB" << std::endl;
    }

    catch(...){
        std::cout << "=====(working inside working)=====>  Shared memory Write Error" << std::endl;
        boost::interprocess::shared_memory_object::remove("workingDataSharedMemory");
        throw;
    }

    std::cout << "=====(working inside working)=====>  Working thread finished" << std::endl;

}
