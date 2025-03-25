We have already compiled a "hello world" program with `mpi` (use `mpic++` to compile).

Let's now look at it in detail:

```cpp
#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size; //also commomly called "nps" or "npes"
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::cout<<"Hello world from rank "<<rank<<" out of " << world_size<< " processes\n";

    // Finalize the MPI environment.
    MPI_Finalize();
}

```

#### MPI Initialization (MPI_Init):

This function initializes the MPI environment. It **must** be called before any other MPI functions. 
It takes command-line arguments `argc` and `argv`, passing them to all processes. *From MPI standard 2, it's actually safe to initialize with `NULL, NULL` if you don't need them as by the standard implementations cannot rely on the command line arguments anymore*.

#### Finalization (MPI_Finalize):

This function cleans up the MPI environment. It **must** be called after all MPI communication is finished, and no MPI calls can be made after it.

*These are two functions that must be present in any MPI program.*


#### World Size (MPI_Comm_size):

We have already seen that to run an mpi program we need to do `mpirun -np NUMBER ./program.x`. The `NUMBER` here will be the total number of the processes that the program runs with. We usually need to know it from inside the program, so the function `MPI_Comm_size` provides us this number. It expects a pointer, so you need to pass the variable with an `&`.

The first variable here, `MPI_COMM_WORLD` is the communicator that includes all processes. A communicator is a group of processes that take part in some communication. `MPI_COMM_WORLD` is the "default" one, that includes all the processes. We will see how to create other communicators later, but you will you this one the most.

*Note:* typically "NUMBER" should not be bigger than the number of cores you have (otherwise performace deteriorates), but for testing purposes on a laptop you can use more. With `openmpi` implementaion you need to explicitly pass `--oversubscribe` flag, with some implimentations it will be done implicitly.


#### Rank of the Process (MPI_Comm_rank):

Within a given communicator, each process gets a unique identifier - its "rank" and *the processes will do different things only based on this rank*. Basically, all of them run *the exactly same* program, but will encounter conditions based on rank.

*You will almost always have to use the "world size" and "rank" in your programs*.

Now if you ran the "hello world" program with 4 processes, you will see that we got "4 programs" output, only being different in their rank.

```bash
Hello world from process 0 out of 4 processes
Hello world from process 1 out of 4 processes
Hello world from process 2 out of 4 processes
Hello world from process 3 out of 4 processes
```


### Communication

The essence of MPI lies in communication between processes (that is, sending and receiving various messages messages). Remember that each of them has it's own memory space and cannot access others memory - that's why they need to communicate. 

Let's start with basics: blocking point to point communication


### point to point communication

These are used for direct communication between two processes, involving a "sender" and a "receiver". We first look at the most "logical" functions: `MPI_Send` and `MPI_Recv`.


```c
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
```

 - `buf`: Pointer to the buffer where the data to be sent is stored.
 - `count`: Number of elements in the buffer to be sent.
 -  `datatype`: The type of data being sent. *MPI has its own datatypes that you cannot template on, so we need to find workarounds for templating.* Common `MPI_Datatype` values are:
    - `MPI_INT`: Integer data type.
    - `MPI_DOUBLE`: Double-precision floating-point data type.
    - `MPI_FLOAT`: Single-precision floating-point data type.
    - `MPI_CHAR`: Character data type.
    - `MPI_BYTE`: Allows you to send raw bytes, useful for arbitrary data.
    - the rest you can google when you need them. It is also possible to create your own datatypes, we will see that later
 - `dest`: Rank of the destination process in the communicator.
 - `tag`: Message tag (an integer that can be used to identify different messages). Sometimes your algorithms require using them, sometimes you can just out "0" everywhere.
 - `comm`: Communicator, typically MPI_COMM_WORLD (but can be something created by user).

The `MPI_Recv` is similar:

```cpp
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);
``` 
  - `buf`: Pointer to the buffer where the received data will be stored.
  - `count`: Number of elements expected in the buf.
  - `datatype`: The type of data being received. 
  - `source`: Rank of the source process from which to receive the message.
  - `tag`: Message tag used to match the sender and receiver.
  - `comm`: Communicator
  - `status`: A pointer to an MPI_Status object that provides information about the received message (e.g., the source and tag). Very often, you can use `MPI_STATUS_IGNORE`.


Let's look at code:

```cpp
#include <mpi.h>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 2) {
        std::cerr << "World size must be at least two for this example" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);//this is how you "exit" from MPI program
    }

    std::vector<int> numbers;
    numbers.resize(10);
    if (world_rank == 0) {
        // Send the numbers vector from process 0 to process 1
        numbers[0] = 77;
        MPI_Send(numbers.data(), numbers.size(), MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else if (world_rank == 1) {
        // Receive the numbers at process 1 from process 0
        MPI_Recv(numbers.data(), numbers.size(), MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "checking first number on process 1: " << numbers[0] << std::endl;
    }

    MPI_Finalize();
    return 0;
}
```

#### Deadlocks

MPI programs can easily run into deadlocks (when nothing is happening and the program is not doing anything anymore) if two processes are both waiting to receive data but are also supposed to send data to each other. It is a very annoying bug "in real life"...

```cpp
#include <mpi.h>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    std::vector<int> vec,vec2;
    vec.resize(1011); //4076 max working for mpich
    vec2.resize(1011);//1010 max working for openmpi    
    
    int number;
    if (world_rank == 0) {
        // Process 0 sends to process 1 and then waits to receive from process 1
        MPI_Send(vec.data(), vec.size(), MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(vec2.data(), vec.size(), MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else if (world_rank == 1) {
        // Process 1 sends to process 0 and then waits to receive from process 0
        MPI_Send(vec.data(), vec.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(vec2.data(), vec.size(), MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    MPI_Finalize();
    return 0;
}

```

This is a very dangerous type of deadlock as it doesn't always happen! In theory, both processes are going to wait for another one to recieve and wouldn't be able to finish their send. In "practice", mpi has internal buffers to send data and the call from `MPI_Send` can return before the other process recieves anything. Play with both `mpich` and `openmp` on your laptop to see if your numbers are the same as mine (then deadlock happens, kill the program with `ctrl+c`). Be careful not to code something like that in the future.

However, if you excange send and recv for both processes - you will get a guaranteed deadlock. At least that one will be easy to see...



### Dealing with templates or custom types - casting everything to bytes:

```cpp
#include <mpi.h>
#include <iostream>
#include <cstring>
#include <vector>

struct CustomData {
    int id;
    double value;
};

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    CustomData data;
    std::vector<int> vec;
    vec.resize(10);
 
    if (world_rank == 0) {
        // Process 0 sends CustomData
        data.id = 123;
        data.value = 456.789;
        MPI_Send(&data, sizeof(CustomData), MPI_BYTE, 1, 0, MPI_COMM_WORLD);
    } else if (world_rank == 1) {
        // Process 1 receives CustomData
        MPI_Recv(&data, sizeof(CustomData), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Process 1 received id: " << data.id << ", value: " << data.value << std::endl;
    }

    if (world_rank == 0) {
        vec[0]=99;
     //here we know it's ints, but lets pretend we don't
        MPI_Send(reinterpret_cast<void*>(vec.data()), vec.size()*sizeof(int), MPI_BYTE, 1, 0, MPI_COMM_WORLD);
    } else if (world_rank == 1) {
        MPI_Recv(reinterpret_cast<void*>(vec.data()), vec.size()*sizeof(int), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Process 1: " << vec[0] << std::endl;
    } 

    MPI_Finalize();
    return 0;
}
```

#### Specializing templates:

```cpp
#include <mpi.h>
#include <iostream>
#include <type_traits>

template<typename T>
void mpi_send(const T& data, int dest, int tag, MPI_Comm comm) {
    if constexpr (std::is_same<T, int>::value) {
        MPI_Send(&data, 1, MPI_INT, dest, tag, comm);
    } else if constexpr (std::is_same<T, double>::value) {
        MPI_Send(&data, 1, MPI_DOUBLE, dest, tag, comm);
    } else {
        MPI_Send(&data, sizeof(T), MPI_BYTE, dest, tag, comm);
    }
}

template<typename T>
void mpi_recv(T& data, int source, int tag, MPI_Comm comm) {
    if constexpr (std::is_same<T, int>::value) {
        MPI_Recv(&data, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE);
    } else if constexpr (std::is_same<T, double>::value) {
        MPI_Recv(&data, 1, MPI_DOUBLE, source, tag, comm, MPI_STATUS_IGNORE);
    } else {
        MPI_Recv(&data, sizeof(T), MPI_BYTE, source, tag, comm, MPI_STATUS_IGNORE);
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 2) {
        std::cerr << "World size must be at least two for this example" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (world_rank == 0) {
        int number = 123;
        mpi_send(number, 1, 0, MPI_COMM_WORLD);

        double dvalue = 456.789;
        mpi_send(dvalue, 1, 0, MPI_COMM_WORLD);
    } else if (world_rank == 1) {
        int number;
        mpi_recv(number, 0, 0, MPI_COMM_WORLD);
        std::cout << "Process 1 received int: " << number << std::endl;

        double dvalue;
        mpi_recv(dvalue, 0, 0, MPI_COMM_WORLD);
        std::cout << "Process 1 received double: " << dvalue << std::endl;
    }

    MPI_Finalize();
    return 0;
}
```



#### Sendrecv

The `MPI_Sendrecv` function is a combination of `MPI_Send` and `MPI_Recv` that allows a process to send a message to one process while simultaneously receiving a message from another process. This is particularly useful in avoiding deadlocks that can occur when processes are trying to send and receive data at the same time (such as in ring communication).

```cpp
int MPI_Sendrecv(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 int dest, int sendtag, void *recvbuf, int recvcount,
                 MPI_Datatype recvtype, int source, int recvtag,
                 MPI_Comm comm, MPI_Status *status);
```
  - `sendbuf`: Pointer to the data buffer that is to be sent.
  - `sendcount`: Number of elements to send.
  - `sendtype`: Datatype of the data to send 
  - `dest`: Rank of the destination process to which the message is sent.
  - `sendtag`: Tag of the send operation (used to match sends with receives).
  - `recvbuf`: Pointer to the buffer where the received data will be stored.
  - `recvcount`: Number of elements expected to be received.
  - `recvtype`: Datatype of the data to receive (yes, it can be different from the send one if we are dealing with derived types).
  - `source`: Rank of the process from which the message will be received.
  - `recvtag`: Tag of the receive operation (used to match receives with sends).
  - `comm`: Communicator used for communication, typically MPI_COMM_WORLD.
  - `status`: A pointer to an MPI_Status object which provides information about the received message, source and tag.



#### MPI_PROC_NULL

You can send to (or recieve from) "nothing". In this case the operation just won't happen, but you can write more genral code and avoid extra "ifs". You will see a useful example in the next course about mpi, but so far we want to see how it works and remember it exists.

This "nothing" is called `MPI_PROC_NULL` and can be used as the source or destination in functions.


```cpp
#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

     int send_data{rank};
    int recv_data{777};

    // Example: Send to the next process, receive from the previous process
    //without "periodic boundary conditions"
    int next_rank = (rank == world_size - 1) ? MPI_PROC_NULL : rank + 1;
    int prev_rank = (rank == 0) ? MPI_PROC_NULL : rank - 1;

    MPI_Sendrecv(&send_data, 1, MPI_INT, next_rank, 0,  // Send to next process
                 &recv_data, 1, MPI_INT, prev_rank, 0,  // Receive from previous process
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    std::cout << "Process " << rank << " sent " << send_data
              << " to process " << next_rank << " and received " << recv_data
              << " from process " << prev_rank << std::endl;

    MPI_Finalize();
    return 0;
}
```

*Note*: when running this example, you might see that the messages from different processes overlap. It's actually often a problem and if they *really* all need to write meaningful data, they should write to different files. On the screen you are usually doing it for testing, so "whatever"...


### Non-blocking communication:

The functions that have an extra "I" in them are for "non-blocking" communication. These functions return immediately and the program can continue doing something else while the communication is in progress.


```cpp
int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request *request);
```
  - `buf`: The starting address of the data to be sent.
  - `count`: Number of elements in the data buffer.
  - `datatype`: The data type of each element in the buffer 
  - `dest`: The rank of the destination process.
  - `tag`: A unique identifier for matching the send and receive.
  - `comm`: The communicator in which the communication is happening 
  - `request`: A "handle" to an `MPI_Request` object, which is used to track the status of the non-blocking operation.


```cpp
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Request *request);
```
  - `buf`: The starting address of the receive buffer where the incoming data will be stored.
  - `count`: Number of elements in the receive buffer.
  - `datatype`: The data type of the elements in the receive buffer 
  - `source`: The rank of the source process from which to receive data 
  - `tag`: The tag that matches the corresponding send (or MPI_ANY_TAG to receive any tag).
  - `comm`: The communicator in which the communication is happening
  - `request`: A "handle" to an `MPI_Request` object, which is used to track the status of the non-blocking operation.

```cpp
#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size != 2) {
        std::cerr << "This program requires exactly 2 processes" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int send_data = world_rank;
    int recv_data = -1;

    MPI_Request request_send, request_recv;  // To handle non-blocking communication

    // Process 0 sends data to Process 1 and receives from Process 1
    if (world_rank == 0) {
        MPI_Isend(&send_data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request_send);  
        MPI_Irecv(&recv_data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request_recv); 
    }
    // Process 1 sends data to Process 0 and receives from Process 0
    else if (world_rank == 1) {
        MPI_Isend(&send_data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request_send);  
        MPI_Irecv(&recv_data, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request_recv); 
    }

    // Wait for both non-blocking send and receive to complete
    //this is a very important part
    MPI_Wait(&request_send, MPI_STATUS_IGNORE);
    MPI_Wait(&request_recv, MPI_STATUS_IGNORE);

    std::cout << "Process " << world_rank << " received data: " << recv_data << std::endl;

    MPI_Finalize();
    return 0;
}
```


### Collective communication

Collective communication functions are executed by *all* processes in a communicator. The simplest one is 

```cpp
int MPI_Barrier(MPI_Comm comm);
```

that just makes everyone wait till they all reach this point.

#### broadcast

`MPI_Bcast` is used to broadcast data from one process to all other processes in the communicator.

```cpp
int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm);
```
  - `buffer`: The data to be broadcast. On the root process, this contains the data to be sent; on other processes, it is the buffer where the data will be received.
  - `count`: Number of elements in the buffer.
  - `datatype`: The datatype of the elements in the buffer (e.g., MPI_INT, MPI_DOUBLE).
  - `root`: The rank of the root process (the one that sends the data).
  - `comm`: The communicator over which the broadcast occurs (e.g., MPI_COMM_WORLD).

```cpp
#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int data;
    if (world_rank == 0) {
        // Root process (rank 0) sets the data
        data = 100;
        std::cout << "Process 0 broadcasting data: " << data << std::endl;
    }

    // Broadcast the data from process 0 to all other processes
    MPI_Bcast(&data, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // All processes, including root, now have the data
    std::cout << "Process " << world_rank << " has data: " << data << std::endl;

    MPI_Finalize();
    return 0;
}
```

#### Gather

```cpp
int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);
```
  - `sendbuf`: The starting address of the send buffer (the data being sent from each process).
  - `sendcount`: The number of elements sent from each process.
  - `sendtype`: The datatype of the elements being sent
  - `recvbuf`: The starting address of the receive buffer (where the gathered data will be stored on the root process).
  - `recvcount`: The number of elements to receive from each process (this must be the same for all processes).
  - `recvtype`: The datatype of the elements being received.
  - `root`: The rank of the root process.
 
```cpp
#include <mpi.h>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Each process sends this data
    int send_data = world_rank;

    // The root process will gather the data into this receive buffer
    std::vector<int> recv_data;
    if (world_rank == 0) {
        recv_data.resize(world_size);  // Only the root process needs to allocate the recv buffer
    }

    // Gather data from all processes to root process (process 0)
    MPI_Gather(&send_data, 1, MPI_INT, recv_data.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Root process prints the gathered data
    if (world_rank == 0) {
        std::cout << "Process 0 gathered data: ";
        for (int i = 0; i < world_size; ++i) {
            std::cout << recv_data[i] << " ";
        }
        std::cout << std::endl;
    }

    MPI_Finalize();
    return 0;
}
```

It's easy to write deadlocks with collective calls, for example by calling them under some conditional operators. Or, in fact, worse - undefined behavior. Due to implementation details it might look like the code works, but it actually doesn't...

```cpp
#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int data = 0;

    //DO NOT DO THIS!!!
    if (world_rank == 0) {
        // Only process 0 initializes the data and calls MPI_Bcast
        data = 100;
        std::cout << "Process 0 broadcasting data: " << data << std::endl;
        MPI_Bcast(&data, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
 
    //they never got the data, do the result will be wrong
    std::cout << "Process " << world_rank << " has data: " << data << std::endl;

    MPI_Finalize();
    return 0;
}

```


### One sided communication:

(I just want to show it exists, we've never actually used it before. It's *supposed* to be faster in certain situations on new hardware. It can also be used to overlap communication and computation.)

One-sided communication allows a process to specify data movement to or from the memory of another process without the involvement or explicit participation of the target process at the time of communication. 

Here are the three main one-sided communication functions:

 - `MPI_Put`: Copies data from the origin process to a target process.
 - `MPI_Get`: Copies data from the target process to the origin process.
 - `MPI_Accumulate`: Combines data from the origin process with data at the target process using a specified operation 
 
We need a concept of "window" to write/read the data.

The `MPI_Win_create` function exposes the memory region of target_value for access by other processes. 

```cpp
MPI_Win_create(void *base, MPI_Aint size, int disp_unit, MPI_Info info, MPI_Comm comm, MPI_Win *win);
```
 - `base`: Pointer to the memory to expose (in this case, &target_value).
 - `size`: Size of the memory (in bytes) to expose.
 - `disp_unit`: The displacement unit (in bytes)
 - info: MPI information object (usually MPI_INFO_NULL).
 - `comm`: Communicator over which the window is shared.
 - `win`: The created window object.


```cpp
int MPI_Win_lock(int lock_type, int rank, int assert, MPI_Win win);
```
 - lock_type: Specifies whether the lock is shared or exclusive.
    - `MPI_LOCK_SHARED`: Multiple processes can hold a shared lock on the same window simultaneously.
    - `MPI_LOCK_EXCLUSIVE`: Only one process can hold an exclusive lock on the window at a time. 
    - `rank`: The rank of the target process 
    -  `assert`: Used to optimize the locking. This is typically 0, but you can provide hints about the lock behavior to MPI for optimization.
    - `win`: The MPI window object that defines the memory region of the target process that will be accessed.

```cpp
int MPI_Put(const void *origin_addr, int origin_count, MPI_Datatype origin_datatype,
            int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype,
            MPI_Win win);
```
  - `origin_addr`: Address of the data in the origin process. This is the data that you want to put into the target process's memory.
  -  `origin_count`: The number of elements of the origin_datatype to transfer from the origin buffer.
  - `target_disp`: The displacement (or offset) in the target window memory where the data should be written.
  - `target_count`: The number of elements of the target_datatype to write into the target memory.
  - `target_datatype`: The datatype of the data to be written into the target process's memory.


Full example:

```cpp
#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size != 2) {
        std::cerr << "This example requires exactly 2 processes" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int target_value = 0;  // This will be the target buffer on Process 1

    // Create an MPI window, exposing the memory region of `target_value`
    MPI_Win win;
    MPI_Win_create(&target_value, sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

    if (world_rank == 0) {
        int origin_value = 100;  // The value that Process 0 will put into Process 1's memory

        // Start an RMA (Remote Memory Access) epoch
        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 1, 0, win);

        // Put the value into Process 1's memory
        MPI_Put(&origin_value, 1, MPI_INT, 1, 0, 1, MPI_INT, win);

        // Complete the RMA epoch
        MPI_Win_unlock(1, win);

        std::cout << "Process 0 put the value " << origin_value << " into Process 1's memory." << std::endl;
    }

    // Synchronize processes
    MPI_Barrier(MPI_COMM_WORLD);

    if (world_rank == 1) {
        // Process 1 prints the value in its memory after the communication
        std::cout << "Process 1 has value " << target_value << " in its memory." << std::endl;
    }

    // Free the window
    MPI_Win_free(&win);

    MPI_Finalize();
    return 0;
}
```


#### Communicators

A communicator is a group of processes that can communicate with each other. 

There are cases when you want to split the set of processes into subgroups for different tasks, that is, create custom communicators. Custom communicators allow processes in one group to communicate independently from processes in another group.

```cpp
int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm);
```
 - `comm`: The existing communicator (e.g., MPI_COMM_WORLD).
 - `color`: An integer value used to define how processes are grouped into new communicators. All processes with the same color are placed in the same communicator.
 - key: Used to rank processes within the new communicator. It determines the order of processes in the new communicator.
 - newcomm: A pointer to the new communicator created.


Let’s say we have 4 processes, and we want to split them into two communicators. Then, we will broadcast a separate message within each communicator.

```cpp
#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size != 4) {
        std::cerr << "This program requires exactly 4 processes." << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Split the communicator into two groups
    MPI_Comm new_comm;
    int color = world_rank < world_size / 2 ? 0 : 1;  // First half gets color 0, second half gets color 1
    MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &new_comm);

    // Get the rank and size in the new communicator
    int new_rank, new_size;
    MPI_Comm_rank(new_comm, &new_rank);
    MPI_Comm_size(new_comm, &new_size);

    // Broadcast a different message in each communicator
    int broadcast_value;
    if (new_rank == 0) {
        // Root of each communicator sets the broadcast value
        broadcast_value = (color == 0) ? 111 : 222;
    }

    // Broadcast within the new communicator
    MPI_Bcast(&broadcast_value, 1, MPI_INT, 0, new_comm);

    std::cout << "Process " << world_rank << " in communicator " << color
              << " received value " << broadcast_value << std::endl;

    // Free the new communicator
    MPI_Comm_free(&new_comm);

    MPI_Finalize();
    return 0;
}
```


#### MPI_Datatype

Users can create their own "datatypes" by combining some elements of "basic types". There are a few functions for that, but lets consider just one of them - `MPI_Type_vector`.

It is used to define a datatype for regularly spaced blocks of data. This is useful when you want to send non-contiguous data from memory, such as sending every N-th element of an array (a strided array) without copying the data to a temporary contiguous buffer.

here is the "signature":

``cpp
int MPI_Type_vector(int count, int blocklength, int stride, MPI_Datatype oldtype, MPI_Datatype *newtype);
``
 - `count`: Number of blocks.
 - `blocklength`: Number of elements in each block.
 - `stride`: Number of elements between the start of each block.
 - `oldtype`: The type of elements in the original array (e.g., MPI_INT, MPI_DOUBLE).
 - `newtype`: The new datatype that MPI will create based on the vector pattern.

After defining the new type with MPI_Type_vector, you need to commit it using MPI_Type_commit() so that it can be used in communication functions.


Let's send every 2nd element of an array:

```cpp
#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    const int array_size = 10;
    int data[array_size];

    if (rank == 0) {
        for (int i = 0; i < array_size; i++) {
            data[i] = i;
        }

     // Define a vector datatype for sending every 2nd element
     MPI_Datatype vector_type;
     MPI_Type_vector(5, 1, 2, MPI_INT, &vector_type);  // Send 5 blocks, each block 1 int, stride 2
     MPI_Type_commit(&vector_type);

     MPI_Send(data, 1, vector_type, 1, 0, MPI_COMM_WORLD);
     MPI_Type_free(&vector_type);
    } else if (rank == 1) {
        for (int i = 0; i < array_size; i++) {data[i] = -1; }
  
     MPI_Recv(data, 5, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);  // 5 ints

     std::cout << "Process 1 received data: ";
     for (int i = 0; i < array_size; i++) {
            std::cout << data[i] << " ";
     }
     std::cout << std::endl;
    }//else if

    MPI_Finalize();
    return 0;
}
```


#### Valgrind

Load your `openmpi` module. Compile "mpi hello world" program. Run under `valgrind`. What do you see? It's typical to get *a lot* of errors and a summary like

```
ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)
==21659== 
==21659== HEAP SUMMARY:
==21659==     in use at exit: 16,745 bytes in 46 blocks
==21659==   total heap usage: 24,948 allocs, 24,902 frees, 4,639,256 bytes allocated
==21659== 
==21659== LEAK SUMMARY:
==21659==    definitely lost: 16,524 bytes in 40 blocks
==21659==    indirectly lost: 15 bytes in 1 blocks
==21659==      possibly lost: 0 bytes in 0 blocks
==21659==    still reachable: 206 bytes in 5 blocks
```

these are "fake" errors, it's due to how openmpi is implemented. There is probably a "suppression" file somewhere in your mpi library files (in `share` folder), something like `openmpi-valgrind.supp`. You can try passing it to valgrind with `--suppresions=PATH_TO_FILE` and see if that helps. For me, it doesn't. So, checking for memory errors with openmpi and valgrind becomes too complicated. The easiest solution is to switch to `mpich` module. It also usually has a small "fake" leak, but that doesn't clash with anything. 

#### GDB (there are real "parallel debuggers" in existense, but a quick choice is still gdb)

To start a session with tracking a few mpi processes manually, type:

```bash
mpirun -np NUMBER_OF_PROCESSES YOUR_CONSOLE_APP -e gdb ./EXECUTABLE
```

For example,

```bash
mpirun -np 2 konsole -e gdb ./hello.x
```

This will open the `NUMBER_OF_PROCESSES` windows (probably on top of each other, so they need to be moved manually), so do it with 2 first.

You will need to type `run` in *every* window if you want full manual contol. Otherwise, add `-ex run` to the command. 

If you just want to know where the thing crashes, type `backtrace` in the window that does. Though as usual with gdb, it can point to a somewhat random location if you have written some sophisticated undefined behavior. 


#### Exercise 0:

Read all the lecture notes, try examples.


#### Exercise 1:

"Ring Communication"

There are N processes arranged in a ring. Each process sends an integer message to its neighbor (the next process in the ring), and the message is passed around the ring in a circular manner.

 - 1.1 Write a program that does "the full circle" of this using MPI_Send and MPI_Recv functions. To see that each iteration worked, make all processes write each new value to a file (with their rank in the name).
 - 1.2 Make that into a function. Template the function and specialize the template for `ints` and `doubles`, while sending the default case as `MPI_BYTE`. Test that all three cases work.
 - 1.3 Write another function that does the same, but this time uses `MPI_Sendrecv`.


#### Exercise 2: (checked for exam)

Write "hello world" type programs (minimal examples that explain what they do) for the following functions:

  - `MPI_Gatherv`
  - `MPI_Scatter`
  - `MPI_Scatterv`
  - `MPI_Allgather`
  - `MPI_Allgatherv`
  - `MPI_Reduce`
  - `MPI_Allreduce`


#### Exercise 3:

Using `MPI_Type_vector`, exchange every 3rd element of vectors between 2 processes.

#### Exercise 4:

Create three communicators by splitting the processes based on `rank % 3`.
Make the new root processes broadcast some valure to their communicators.
Then let all processes print the result to check that it works.



#### Exercise 5: Parallize the timer (checked for exam)

Write a parallel version of your scoped timer (have a new header for it so you wouldn't confuse them). Essentially, what you need to do is to gather all the timing data on one process. Unfortunatelly, neighter `std::map` nor `std::pair` is contiguos, so you cannot just send that, you have to do it element by element or copy to a vector first. Since we only do it once, it's ok if you use not the most optimal solution. At the end, let your root process print all the statistics: the results for each process, the max, the min and the average.  For simplicity, you can assume that all processes have been called to time the same functions. Test this on "sleep" before proceeding further.

*We need to run the next timings on cineca as performance of mpi strongly depends on hardware, so results on laptop are rather meaningless*. (Of course you can test that it runs on laptop first, but the numbers are not guaranteed to have the same relations.)

 - time sending a big vector (let's say around 2 Gb) in one line vs "element by element"
 - time sending a big double vector using template specialization vs sending it by casting to bytes
 - time sending a big vector vs "putting" it (using one-sided communication)

#### Exercise 6: (checked for exam)

Let's prepare a "dummy" parallel code with a matrix class ready for the next parallel course. Take your exercise with std::vector and mnodify it in the following way:
  
  - only leave constructor, random fill function, print in your class and the multiplication operator, delete the rest
  - in the `main` initialize and finalize `mpi`
  - let your class now have extra fields for rank, world_size and communicator (variable of type `MPI_Comm`), initialize them in the constructor
  - fill matrix with random numbers
  - let each process print into a separate file (with the rank in the name) to check that everything works
  - time the printing to file operation (just for the sake of having the timer ready to use with this matrix later)
  - we want to have it looking "nice", so make sure your matrix class and timer live in separate header files

#### Exercise 7: (checked for exam) 

#### MPI scheduler

In distributed computing environments, tasks assigned to different processes may vary in completion time. To maximize efficiency, it's essential to ensure that processes don't remain idle waiting for others to finish.

Your assignment is to design an MPI-based dynamic task scheduler to address this scenario. Here's how it should work:

##### Master Process:

One of the MPI processes will act as the "Master" or "Scheduler". This process will not perform any computational tasks. Instead, it will be responsible for task distribution.
As worker processes complete their tasks and become available, the Master process should assign them new tasks dynamically.

##### Worker Processes:

The worker processes will receive tasks (in this case, sleep durations) from the Master process.
Upon receiving a task, each worker should print a message indicating it's going to sleep for the given duration.
After the sleep duration elapses, the worker should print a message indicating it has awakened. It should then request the Master process for the next task.

##### Simulating Task Durations:

For the purpose of this exercise, tasks are represented as sleep durations (in seconds).
We provide a function that generates these sleep durations. Use this function to generate and distribute tasks to the worker processes.


##### Hints: 
 - use tags to know when to stop:
```cpp
const int WORKTAG = 0;
const int STOPTAG = 1;
```
 - read up on  `MPI_ANY_TAG`
 - read up on `MPI_ANY_SOURCE`
 - read up on  `MPI_SOURCE`


Here is the function that generates the sleep times:
```cpp
void generate_sleep_times(std::vector<int>& sleep_times, int num_tasks) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10);

    sleep_times.resize(num_tasks);
    std::generate(sleep_times.begin(), sleep_times.end(), [&]() { return dis(gen); });
}
```

**Your main should be:**

```cpp
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        std::cerr << "World size must be greater than 1 for " << argv[0] << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (rank == 0) {
        std::vector<int> sleep_times;
        generate_sleep_times(sleep_times, NUM_TASKS);//set NUM_TASKS yourself
        supervise_work(sleep_times, size);
    } else {
        do_work();
    }

    MPI_Finalize();
    return 0;
}
```

#### Exercise 8 (also checked)

Write another ring communication program, but this time have a large vector to send around (you can still fill it in with ranks) and make all the processes calculate the sum of what they got. Do it in two ways - one with Sendrecv, another with Isend and Irecv - and try to overlap computation and communication. Time both variants.

Now modify your program to get the same result by using Allreduce and time it too. 








