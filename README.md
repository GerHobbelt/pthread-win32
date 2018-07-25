
### pthreads-win32 2.10.0.0
This is a fork of version 2.10.0.0 the [pthreads-win32](https://sourceforge.net/projects/pthreads4w/ "https://sourceforge.net/projects/pthreads4w/") package. The ABI of this fork is different from the original. Changes done:

1. The type of the reuse counter in ptw32_handle_t has been changed from
   int to size_t in order to facilitate long-running servers.

2. Removed unused elements from pthread_once_t


See also the [README](docs/README) file for an intro about pthreads-win32.