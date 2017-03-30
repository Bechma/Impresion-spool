#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define archivo_bloqueo "bloqueo"

static void manejador(int senal){
	wait(NULL);
	return;
}

int main (int argc, char *argv[]){
	
	if(argc != 2){
		puts("<ejecutable> <nombre_fifo>");
		exit(-1);
	}
	
	char entrada[strlen(argv[1]+2)], salida[strlen(argv[1]+2)];
	int fd_e, fd_s, fd_bloqueo;
	int pid_cliente;
	pid_t proceso;
	
	sprintf(entrada, "%se", argv[1]);
	sprintf(salida, "%ss", argv[1]);
	umask(0);

	// Creación de fifos
	mkfifo(entrada, S_IRWXU);
	mkfifo(salida, S_IRWXU);
	
	// Apertura de fifos en read, write
	if( (fd_e = open(entrada, O_RDWR)) < 0){	perror("abriendo entrada");	exit(-1);	}
	if( (fd_s = open(salida, O_RDWR)) < 0 ){	perror("abriendo salida");	exit(-1);	}
	
	// Creación del archivo de bloqueo de proxys
	if( (fd_bloqueo = open(archivo_bloqueo, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU)) < 0){
		perror("creando archivo bloqueo");
		exit(-1);
	}
	
	// Manejador de señal CHILD
	signal(SIGCHLD, manejador);
	
	// Mientras no eof	///////////////////////////////////////////////////////////////////////////////
	while(1){
		read(fd_e, &pid_cliente, sizeof(int));
		
		// Ajustes previos llamada proxy
		if( (proceso = fork()) == 0 ){
			pid_t proxy = getpid();
			char fifo_proxy[50];
			int fd_proxy;
			
			sprintf(fifo_proxy, "fifo.%d", proxy);
			
			if(mkfifo(fifo_proxy, S_IRWXU) < 0){
				perror("fifo salida");
				exit(-1);
			}
			
			write(fd_s, &proxy, sizeof(int));
			if( (fd_proxy = open(fifo_proxy, O_RDONLY)) < 0){
				perror("abriendo fifo del proxy");
				exit(-1);
			}
			
			dup2(fd_proxy, STDIN_FILENO);
			if(execlp("./proxy", "proxy", NULL) < 0){
				perror("¿donde está proxy?");
				exit(-1);
			}
		}
	}

	return 0;
}
