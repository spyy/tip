#include <Python.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

static void setStdio(void){
    struct termios stdio;    
    
    memset(&stdio,0,sizeof(stdio));
    stdio.c_iflag=0;
    stdio.c_oflag=0;
    stdio.c_cflag=0;
    stdio.c_lflag=0;
    stdio.c_cc[VMIN]=1;
    stdio.c_cc[VTIME]=0;
    tcsetattr(STDOUT_FILENO,TCSANOW,&stdio);
    tcsetattr(STDOUT_FILENO,TCSAFLUSH,&stdio);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);       // make the reads non-blocking         
}

int runForever(const char *device) {
    struct termios tio;
    struct termios old_stdio;
    int tty_fd;        
    unsigned char c='D';

    tcgetattr(STDOUT_FILENO,&old_stdio);

    setStdio();

    memset(&tio,0,sizeof(tio));
    tio.c_iflag=0;
    tio.c_oflag=0;
    tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
    tio.c_lflag=0;
    tio.c_cc[VMIN]=1;
    tio.c_cc[VTIME]=5;

    tty_fd=open(device, O_RDWR | O_NONBLOCK);      
    cfsetospeed(&tio,B115200);            // 115200 baud
    cfsetispeed(&tio,B115200);            // 115200 baud

    tcsetattr(tty_fd,TCSANOW,&tio);
    while (c!='q')
    {
            if (read(tty_fd,&c,1)>0)        write(STDOUT_FILENO,&c,1);              // if new data is available on the serial port, print it out
            if (read(STDIN_FILENO,&c,1)>0)  write(tty_fd,&c,1);                     // if new data is available on the console, send it to the serial port
    }

    close(tty_fd);
    tcsetattr(STDOUT_FILENO,TCSANOW,&old_stdio);

    return EXIT_SUCCESS;
}


static PyObject *
tip_run(PyObject *self, PyObject *args)
{
    const char *device;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &device))
        return NULL;
    sts = runForever(device);
    return Py_BuildValue("i", sts);
}


static PyMethodDef TipMethods[] = {
    {"run",  tip_run, METH_VARARGS,
     "Run in serial"},    
    {NULL, NULL, 0, NULL}       
};

PyMODINIT_FUNC
inittip(void)
{
    (void) Py_InitModule("tip", TipMethods);
}


