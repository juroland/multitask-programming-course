#include <errno.h>
#include <pthread.h>
#include "buffer.h"
static buffer_t buffer[BUFSIZE];
static int bufin = 0;
static int bufout = 0;
static int totalitems = 0;

int getitem(buffer_t *itemp) {  /* remove item from buffer and put in *itemp */ 
   int erroritem = 0;
   if (totalitems > 0) {                   /* buffer has something to remove */
      *itemp = buffer[bufout];
       bufout = (bufout + 1) % BUFSIZE;
       totalitems--;
   } else
       erroritem = EAGAIN; 
   return erroritem;
}

int putitem(buffer_t item) {                    /* insert item in the buffer */
   int erroritem = 0; 
   if (totalitems < BUFSIZE) {           /* buffer has room for another item */
      buffer[bufin] = item;
      bufin = (bufin + 1) % BUFSIZE;
      totalitems++;
   } else
      erroritem = EAGAIN;
   return erroritem; 
}
