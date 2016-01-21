/*!
 *  \brief     Passing data from one process to another using boost interprocess
 *  \author    Katsileros Petros
 *  \date      21/01/2016
 *  \copyright
 */


#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

int numData = 5000;

int main(int argc, char *argv[])
{
    using namespace boost::interprocess;

    if(argc == 1) {
        try
        {
                //size_t sz = sizeof(vector<int, ShmemAllocator>) + 1024 * sizeof(int);

                //Shared memory front-end that is able to construct objects
                //associated with a c-string. Erase previous shared memory with the name
                //to be used and create the memory segment at the specified address and initialize resources
                shared_memory_object::remove ("MySharedMemory");
                managed_shared_memory segment
                        (create_only
                        ,"MySharedMemory" //segment name
                        , 4*numData*sizeof(int));

                //Alias an STL compatible allocator of ints that allocates ints from the managed
                //shared memory segment.  This allocator will allow to place containers
                //in managed shared memory segments
                typedef allocator<int,managed_shared_memory::segment_manager> ShmemAllocator;

                //Alias a vector that uses the previous STL-like allocator
                typedef vector<int, ShmemAllocator> MyVector;

                //Initialize shared memory STL-compatible allocator
                const ShmemAllocator alloc_inst(segment.get_segment_manager());

                //Construct a shared memory
                MyVector *myvector = segment.construct<MyVector>("MyVector") //object name
                                                (alloc_inst); //first ctor parameter

                std::size_t free_memory_after_allocation = segment.get_free_memory();
                std::cout << "free memory before grow: " << free_memory_after_allocation << std::endl;

                //int numData = 1024;
                //Now that the segment is not mapped grow it adding extra 500 bytes
                managed_shared_memory::grow("MySharedMemory", numData*sizeof(int));
                //Map it again
                managed_shared_memory shm(open_only,"MySharedMemory");

                //std::cout << "1024*sizeof(int): " << 1024*sizeof(int) << std::endl;
                free_memory_after_allocation = shm.get_free_memory();
                std::cout << "free memory after grow: " << free_memory_after_allocation << std::endl;

                std::cout << "number of int data: " << numData << std::endl;
                std::cout << "size of int data: " << sizeof(int)*numData << std::endl;

                //Insert data in the vector
                for(int i=0; i < numData; ++i){
                    myvector->push_back(i);
                }

                //std::cout << "1024*sizeof(int): " << 1024*sizeof(int) << std::endl;
                free_memory_after_allocation = shm.get_free_memory();
                std::cout << "free memory after vector initialization: " << free_memory_after_allocation << std::endl;

                //Launch child process
                std::string s(argv[0]); s += " child ";
                if(0 != std::system(s.c_str())) {
                    //When done, destroy the vector from the segment
                    segment.destroy<MyVector>("MyVector");
                    return 1;
                }

                //std::cout << "numData: " << numData;

        }

        catch(...){
            shared_memory_object::remove("MySharedMemory");
            throw;
        }

    }
    else {
        try
        {
            managed_shared_memory segment
            (open_only
            ,"MySharedMemory"); //segment name

            //Alias an STL compatible allocator of ints that allocates ints from the managed
            //shared memory segment.  This allocator will allow to place containers
            //in managed shared memory segments
            typedef allocator<int,managed_shared_memory::segment_manager> ShmemAllocator;

            //Alias a vector that uses the previous STL-like allocator
            typedef vector<int, ShmemAllocator> MyVector;

            //Construct a shared memory
            MyVector *myvector = segment.find<MyVector>("MyVector").first;

            std::size_t free_memory_after_allocation = segment.get_free_memory();
            std::cout << "free memory after allocation (child): " << free_memory_after_allocation << std::endl;

            //int numData = 1024;

            // Read the data
            int *p = myvector->data();
            for(int i=0;i<numData;i++) {
                std::cout << "i:" << i << " == " << *p++ << std::endl;
            }

            free_memory_after_allocation = segment.get_free_memory();
            std::cout << "free memory after allocation (child): " << free_memory_after_allocation << std::endl;
            std::cout << "numData: " << numData << std::endl;

            //When done, destroy the vector from the segment
            // segment.destroy<MyVector>("MyVector");

            shared_memory_object::remove("MySharedMemory");
            return 0;
        }

        catch(...){
            shared_memory_object::remove("MySharedMemory");
            throw;
        }

    }

}
