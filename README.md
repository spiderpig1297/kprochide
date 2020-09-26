# __kprochide__

_kprochide_ is an LKM (loadable kernel module) for hiding processes from the userland. The module is able to hide multiple processes and is able to dynamically receive new processes to hide.

**NOTE:** the module was built and tested for linux version 4.19.98.

## __How It Works__
_kprochide_'s MO is to replace the function that is responsible for iterating the _procfs_'s directories. 

Lets take a detailed look at the process of finding all running processes:

    ----------------
    | ls -al /proc |
    ----------------
            |
            |   ls calls getdents() system-call to retrieve all procfs' directories.
            |
    ----------------------
    | getdents() syscall |
    ----------------------
            |
            |   getdents() syscall retrieves procfs' iterate_shared() function from
            |   its file_operation structure.
            |
    --------------------
    | iterate_shared() |
    --------------------
            |
            |   iterate_shared() is called, invoking the function filldir_t().
            |
    ---------------
    | filldir_t() |
    ---------------
            
                filldir_t() is called, responsible for listing the existing directories.
            
By replacing the _filldir()_ function with our own, we are able to control which directories the user can see.
Whenever our new function is called with the directory that represents the process we want to hide, the module "lies" to the user-mode telling that the directory doesn't exist.

    --------------------
    | iterate_shared() |
    --------------------
            |
            |   iterate_shared() is called, invoking the function filldir_t().
            |
    --------------------      ---------------
    | evil_filldir_t() | ---> | filldir_t() | 
    --------------------      ---------------
            
                evil_filldir_t() is called. If the process should be hidden, 
                it returns 0. Otherwise, it calls the original filldir_t() and returns its result. 

The module uses character device in order to __dynamically receive the _PIDs_ to hide.__ Each new PID is saved to a global linked-list.

Once the module is unloaded, it restores procfs' original functions, unregisteres the character device and empties the linked-list.

## __Usage__

1. Clone into the respository:

        git clone https://github.com/spiderpig1297/kprochide.git

2. Navigate to the source-code directory, and run:

        make

3. Upload the compiled module (_kprochide.ko_) to the target machine (if needed), and run:
    
        sudo insmod kprochide.ko

4. Create an FS node for the module's character device:

    **NOTE:** the module logs its char device major number to the log. run _dmesg_ to see it.

        mknod /dev/<name> c <major_from_dmesg> 0

5. Write a PID to hide to our newly-created device:

        echo <pid> | /dev/<name>

6. List running processes

        ps aux 
        ps aux | grep <your_process_name>

        ls -al /proc
        ls -al /proc | grep <your_process_name>

7. ta-da!

## __Limitiations__

* Currently there is no support for dynamically removing _PIDs_ to hide.
* Currently there is no support for listing the processes to hide.
