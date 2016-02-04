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
int finish_flag = 0;
int working_flag = 0;
int check_flag = 0;
int exit_flag = 1;

int extraMem = 65536;

void writeShmem()
{
    using boost::this_thread::get_id;

    std::cout << "=====(working)=====>  Writing data to shared memory" << std::endl;
    finish_flag = 0;
    working_flag = 1;

    try
    {

            //Shared memory front-end that is able to construct objects
            //associated with a c-string. Erase previous shared memory with the name
            //to be used and create the memory segment at the specified address and initialize resources
            boost::interprocess::shared_memory_object::remove ("DataSharedMemory");
            boost::interprocess::managed_shared_memory segment
                    (boost::interprocess::create_only
                    ,"DataSharedMemory" //segment name
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
            MyVector *vecTr = segment.construct<MyVector>("DataVector") //object name
                                            (alloc_inst); //first ctor parameter

            //std::size_t free_memory_after_allocation = segment.get_free_memory();
            //std::cout << "memory: : " << (float)(free_memory_after_allocation / (1024*1024)) << " MB" << std::endl;

            //std::cout << "number of Triangles: " << (int) (numData/1000) << " K" << std::endl;
            //std::cout << "sizeof(Triangle): " << sizeof(Triangle) << " Bytes" << std::endl;
            //std::cout << "size of data: " << (float) (sizeof(Triangle)*numData / (1024*1024)) << " MB" << std::endl;


            //std::vector<Triangle> triang;
            //Insert data in the vector
            for(int i=0; i < numData; ++i){
                  vecTr->push_back(Triangle());

                  vecTr->at(i).node[0] = Point((real_t)std::rand(),(real_t)std::rand(),(real_t)std::rand());
                  vecTr->at(i).node[1] = Point((real_t)std::rand(),(real_t)std::rand(),(real_t)std::rand());
                  vecTr->at(i).node[2] = Point((real_t)std::rand(),(real_t)std::rand(),(real_t)std::rand());

//                  vecTr->at(i).node[0] = Point(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
//                  vecTr->at(i).node[1] = Point(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
//                  vecTr->at(i).node[2] = Point(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());

                  //triang.push_back(vecTr->at(i));
            }

            //printTriangle( triang );

            //std::size_t free_memory_after_allocation = segment.get_free_memory();
            //std::cout << "free memory after vector initialization: " << (float) (free_memory_after_allocation / (1024*1024)) << " MB" << std::endl;
    }

    catch(...){
        std::cout << "=====(working)=====>  Shared memory Write Error" << std::endl;
        boost::interprocess::shared_memory_object::remove("DataSharedMemory");
        throw;
    }

    std::cout << "=====(working)=====>  Working thread finished" << std::endl;
    finish_flag = 1;
    working_flag = 0;

}

void checkData();

void readMsg()
{
    std::cout << "I am reading messages" << std::endl;

    // Open the condition variable
    boost::interprocess::named_condition named_cnd(boost::interprocess::open_only, "cnd");
    // Open the mutex
    boost::interprocess::named_mutex named_mtx(boost::interprocess::open_only, "mtx");

    //Open managed shared memory
    boost::interprocess::managed_shared_memory shmMsg(boost::interprocess::open_only, "MsgSharedMemory");

    // Waiting
    std::cout << "waiting for signal" << std::endl;
    // Lock the mutex
    boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(named_mtx);
    named_cnd.wait(lock);

    //Find the intmsg
    int *intmsg = shmMsg.find<int> ("IntMSG").first;

    std::cout << "Receive signal: " << *intmsg << " from working." << std::endl;

    // Switch cases for message variable
    switch(*intmsg)
    {

        // IPC_MESH_DATA
        case(3):
        {
             if(finish_flag) {
                 std::cout << "Notify ipc_1 to read the data" << std::endl;
                 *intmsg = 999;
             }
             else {
                 std::cout << "Data is not ready yet" << std::endl;
                *intmsg = 99;
             }
             named_cnd.notify_all();
        }
        break;

        // IPC_REQ_SOLVE
        case(5):
        {
             if(working_flag)
             {
                std::cout << "Send msg to ipc_1 that worker is still running" << std::endl;
             }
             else
             {
                 named_cnd.notify_all();
                 boost::thread thread1(writeShmem);
             }
        }
        break;

        // Testing
        case(100):
        {
            checkData();
        }
        break;

        default:
        {
            std::cout << "Unknown command" << std::endl;
            named_cnd.notify_all();
        }
    }

}


void createMsgSharedMem()
{
    std::cout << "Main thread is creating the shared memory for messaging" << std::endl;

    // Delete if previous instance exists
    boost::interprocess::shared_memory_object::remove("MsgSharedMemory");
    //Construct managed shared memory
    boost::interprocess::managed_shared_memory shmMsg(boost::interprocess::create_only, "MsgSharedMemory", extraMem);

    //Create an object of MyType initialized to {0.0, 0}
    int *intmsg = shmMsg.construct<int>
         ("IntMSG")   //name of the object
         (0);     //initialized to zero

    // Just to ignore compile warning
    *intmsg = 0;

    // Create the condition variable
    boost::interprocess::named_condition::remove("cnd");
    boost::interprocess::named_condition named_cnd(boost::interprocess::open_or_create, "cnd");

    // Create the mutex
    boost::interprocess::named_mutex::remove("mtx");
    boost::interprocess::named_mutex named_mtx(boost::interprocess::open_or_create, "mtx");
}

void checkData()
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

         boost::interprocess::managed_shared_memory segment1
         (boost::interprocess::open_only
         ,"workingDataSharedMemory"); //segment name

         //Construct a shared memory
         MyVector *myvector1 = segment1.find<MyVector>("workingDataVector").first;

         std::vector<Triangle> vecTr1;
         // Read the data
         for(int i=0;i<numData;i++) {
             vecTr1.push_back( myvector1->at(i) );
         }

         //printTriangle( vecTr1 );

         // Check the triangles
         for(int i=0;i<numData;i++)
         {
             if(!(vecTr.at(i) == vecTr1.at(i)))
             {
                check_flag = 1;
                break;
             }
         }


         if(check_flag)
             std::cout << "\n============ IPC ERROR ==============\n" << std::endl;
         else {
             exit_flag = 0;
             std::cout << "\n============ IPC SUCCESS ============\n" << std::endl;
         }
}

void terminate()
{
    // Delete shared memory segments
    boost::interprocess::shared_memory_object::remove("MsgSharedMemory");
    // Delete mutex and cond variable
    boost::interprocess::named_mutex::remove("mtx");
    boost::interprocess::named_condition::remove("cnd");
    // Remove data shared memory segment
    boost::interprocess::shared_memory_object::remove("DataSharedMemory");
}


int main(int argc, char *argv[])
{
    std::cout << "Executing main" << std::endl;

    // Create the message shared mem space
    createMsgSharedMem();

    // Keep reading messages
    while(exit_flag) {
        readMsg();
    }

    // Terminate
    std::cout << "Terminating main" << std::endl;
    terminate();

    return (check_flag == 1);
}

