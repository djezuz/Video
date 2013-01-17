//para ejecutar basta con compilar usando todas las librerias de opencv y ejecutar sin parametros

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include "cv.h"
#include "highgui.h"

#define HELLO_PORT 1234 //puerto de recepcion


//definimos la estructura tal cual la envia el emisor
typedef struct pixel{
     CvScalar s[18][22];     
     int i;
     int j;
     unsigned long frame;
}pixel;

pixel punto;

unsigned long frame_actual;


main(int argc, char *argv[]){

  struct sockaddr_in addr;
	int fd;
	struct ip_mreq mreq;
	socklen_t addrlen;
	int a, n;

	if ((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
		perror("socket");
		exit(1);
	}       

     memset(&addr,0,sizeof(addr));
     addr.sin_family=AF_INET;
     addr.sin_addr.s_addr=htonl(INADDR_ANY); /* N.B.: differs from sender */
     addr.sin_port=htons(1234);
     
     if (a=bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	  perror("bind");
	  exit(1);
     }

     cvNamedWindow("Recepcion", CV_WINDOW_AUTOSIZE );
     IplImage* gris;
     IplImage* llega;
     

     CvScalar s;     
     
     printf("%i Se conecto",a);
     IplImage* p = cvCreateImage(cvSize(352,288),8,1); //creamos una imagen de las dimensiones de cada imagen
     int i, j, auxi = 0, auxj = 0; 
     int bandera=0;
     while (1) {         
	 addrlen=sizeof(addr);
	 
	    
            n=recvfrom(fd,(char*)&punto,sizeof(punto),0,(struct sockaddr *) &addr,&addrlen); //recepcion de paquetes
             printf("i=%d j=%d\n",punto.i,punto.j);


	    if(bandera==0) //inicialmente la bandera es cero esta parte solo lo hace la primera vez
	    {
 		frame_actual=punto.frame; //toma el valor de secuencia y lo guarda 
		bandera=1; 
	     }

            if(punto.frame>frame_actual){ //almacena ya la secuencia se determina si el paquete que llega tenga una secuencia siempre mayor
		    frame_actual=punto.frame;
	            cvShowImage("Recepcion", p); //refresca la imagen siempre que llega una secuencia mayor
		    cvWaitKey(3);
            }
            punto.i = punto.i * 18; 
            punto.j = punto.j * 22; 


             //reconstruimos cada cuadro de 18x22 y con la funcion cvSet2D los almacenamos en nuestro frame p
            for(i=punto.i-18;i<punto.i;i++){
               for(j=punto.j-22;j<punto.j;j++){
                cvSet2D(p,i,j,punto.s[auxi][auxj]);
                auxj++;
              }
             auxi++;
             auxj=0;
           } 
         auxi=0;
         auxj=0;
	  
	}  
   cvDestroyWindow("Recepcion");	
}



