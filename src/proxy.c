#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define archivo_bloqueo "bloqueo"

int main (int argc, char * argv[]) 
{

	struct flock cerrojo;
	int dbloqueo, bytes_leidos;
	FILE *temporal = tmpfile();
	char buffer[1024];
	char eliminar_archivo[50];
	
	while((bytes_leidos = read(STDIN_FILENO, buffer, 1024)) > 0)
		fwrite(buffer, sizeof(char), bytes_leidos, temporal);
	
	umask(0);
	if ((dbloqueo=open( archivo_bloqueo, O_RDWR|O_CREAT, S_IRWXU)) == -1 ){
		perror("open fallo");
		exit(-1);
	}
	
	
	
	cerrojo.l_type=F_WRLCK;
	cerrojo.l_whence=SEEK_SET;
	cerrojo.l_start=0;
	cerrojo.l_len=0;
	
	/* intentamos un bloqueo de escritura del archivo completo */
	while (fcntl (dbloqueo, F_SETLK, &cerrojo) == -1) {
		/* si el cerrojo falla, vemos quien lo bloquea */
		while (fcntl (dbloqueo, F_SETLK, &cerrojo) == -1 && cerrojo.l_type != F_UNLCK) {
	//		printf ("%s bloqueado por %d desde %ld hasta %ld para %c", *argv, cerrojo.l_pid, cerrojo.l_start, cerrojo.l_len, cerrojo.l_type==F_WRLCK ? 'w':'r');
			if (!cerrojo.l_len) break;
			cerrojo.l_start += cerrojo.l_len;
			cerrojo.l_len = 0;
		} /*mientras existan cerrojos de otros procesos */
	} /*mientras el bloqueo no tenga exito */

	/* ahora el bloqueo tiene exito y podemos procesar el archivo*/
	printf ("Proxy %d adquiere cerrojo pantalla\n", getpid());
	fseek(temporal, SEEK_SET, 0);
	int leidos;
	while( (leidos = fread(buffer, sizeof(char), 1024, temporal)) )
		write(STDOUT_FILENO, buffer, leidos);
	
	char salto = '\n';
	write(STDOUT_FILENO, &salto, sizeof(char));
	
	/* una vez finalizado el trabajo, desbloqueamos el archivo entero*/
	cerrojo.l_type=F_UNLCK;
	cerrojo.l_whence=SEEK_SET;
	cerrojo.l_start=0;
	cerrojo.l_len=0;
	if (fcntl (dbloqueo, F_SETLKW, &cerrojo) == -1) perror ("Error Desbloqueo");
	
	sprintf(eliminar_archivo, "fifo.%d", getpid());
	
	unlink(eliminar_archivo);

	return 0;
}
