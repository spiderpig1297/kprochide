# __kprochide__

`kprochide` is an LKM for hiding processes from the userland. The module is able to hide multiple processes and is able to dynamically receive new processes to hide.

**NOTE:** the module was built and tested for linux version `4.19.98`.

## __How It Works__
`kprochide`'s MO is to replace the function that is responsible for iterating the `procfs`'s directories. 

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
            
By replacing `filldir()` function with our own, we are able to control which directories the user can see.
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

Once the module is unloaded, it restores `procfs`' original functions, unregisteres the character device and empties the linked-list.

## __Usage__

```sh
$ git clone https://github.com/spiderpig1297/kprochide.git
$ cd kprochide
$ make
$ sudo insmod kprochide.ko
```

Once the module starts, it logs the registered character device major number to the log. run `dmesg` to see it and `mknod` to create a node to the device:

```sh
$ dmesg
$ mknod /dev/readpid c <MAJOR_GOES_HERE> 0
```

Write a PID to hide to our newly-created device:
```sh
echo <pid> | /dev/readpid
```

List running processes:
```sh
$ ps aux 
$ ps aux | grep <PROCESS_NAME>
$ ls -al /proc
$ ls -al /proc | grep <PROCESS_NAME>
```

__ta-da!__

## __Limitations__

* Currently there is no support for dynamically removing _PIDs_ to hide.
* Currently there is no support for listing the processes to hide.
* When restarting the machine, the module will be unloaded and all process will be visible again.
