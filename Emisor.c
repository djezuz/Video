//Servidor, lleva parametros del cliente que es la direccion y el puerto por default = 1234
/*De acuerdo a lo solicitado se logro la implementacion de video streaming al 100%, logrando implementar
la fragmentación de cada imagen transmitida, el envio y recepcion mediante UDP, la reconstrucción en el destino
y la comparativa de pixeles que determinan si se envia cada fragmente siempre y cuando cambie en un porcentaje 
del 10%*/

#include "cv.h"
#include "highgui.h"
#include "stdio.h"
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/soundcard.h>

//parametros del tamaño de frame que contendra el video
void cvResizeWindow(
    const char* name,
    int         width,
    int         height
);


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

//Estructura de la que esta conformado nuestros paquetes
typedef struct pixel{
     CvScalar s[18][22];     
     int i;
     int j;
     unsigned long frame;
}pixel;

pixel punto;

int main( int argc, char* argv[] ) { 


   int sock, n;
   int i,j;

   unsigned int length;

   struct sockaddr_in server, from;
  struct hostent *hp;


    if (argc != 3) { 
		printf("Usage: server port\n");
	        exit(1);
	}



    sock= socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) error("socket");
	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]);
	if (hp==0) error("Host Desconocido");
   
	bcopy((char *)hp->h_addr,(char *)&server.sin_addr,hp->h_length);
	server.sin_port = htons(atoi(argv[2]));
	length=sizeof(struct sockaddr_in);
   
    
    cvNamedWindow("Envio", CV_WINDOW_AUTOSIZE); //Definimos un nombre al frame que contendra las imagenes

    CvCapture* capture; //Definimos un puntero de tipo CvCapture para la captura de video
   
    capture = cvCreateCameraCapture( 0 ); //Por medio de la funcion definida capturamos de la webcam de la pc es 0 cuando solo hay una webcam
 
    assert( capture != NULL );
    //Definimos los frame que vamos a manipular cada frame puede contener a la vez una imagen
    IplImage* frame; 
    IplImage* gris;
    IplImage* copy;
    IplImage* p = cvCreateImage(cvSize(352,288),8,1);

    //definimos dos valores de tipo cvScalar que es tipo que devuelve la funcion mas adelante del valor de cada pixel
    CvScalar s, s2;

    int bandera=0;
    int umbral = 0;
    int aux, aux2, bol, f;
    int cont=0;
    int flag;
    int cont2=0;
    while(1) {
        
       
        
        frame = cvQueryFrame( capture ); //Guardamos en frame por medio de la funcion mostrada la primer imagen del video a color
  
        int auxi = 0;
        int auxj = 0;
        int auxk = 0;
     

        gris = cvCreateImage(cvSize(frame->width,frame->height),frame->depth,1); //Determinamos que el frame gris es del mismo tamaño que frame
       // y de un canal
                  
        
        cvCvtColor(frame, gris ,CV_BGR2GRAY); //definimos que lo que contenga frame va a estar en gris pero en escala de grises
        
        if(cont2==15){
          cont2=0;
          bandera=0;
         }

         if(bandera==0){
         copy = cvCloneImage(gris); //se hace un clone de la imagen a escala de grises
        bandera=1;
        flag=0;
        
         }
        



        if( !frame ) break;
         cvWaitKey(5);
        

      // Definimos cuatro for anidados el centro construye cada fragmento que se va a enviar
     // de 18 por 22 dado que nuestra camara captura con una dimension de 288 x 352

       for(int x=0;x<16;x++){
        
         for(int k=0;k<16;k++){

           for(i=auxi; i<18+auxi;i++){
               
              for(j=auxj; j<22+auxj;j++){
                

             s=cvGet2D(gris,i,j); //obtenemos el valor del pixel i, j de la imagen frame
             aux = s.val[0]; //guardamos el valor del pixel en un auxiliar entero
             s2=cvGet2D(copy,i,j); // obtenemos el valor del pixel i, j de la imagen copy
             aux2 = s2.val[0];//guardamos el valor del pixel en un auxiliar entero
             
              f = abs(aux-aux2); //Guardamos la diferencia absoluta de los valores obtenidos de cada imagen

 
              if(f>25){ //   si la diferencia es mayor a 25 incremento un umbral
              // printf("s =%d  s2=%d umbral=%d   f=%d\n", aux, aux2, umbral, f);
                umbral++;
              }


             punto.s[i-auxi][j-auxj]=s; //aqui guardamos para el paquete el valor del pixel de la imagen en grises
            
               }
             
           
             }
             
             punto.i = x+1; //enviamos la posicion i de el fragmento 16x16
             punto.j = k+1; //enviamos la posicion j de el fragmento 16x16
              
             auxj = auxj + 22;

              punto.frame=cont;//enviamos una secuencia de paquetes por cada 255 paquetes de una imagen hay una secuencia

           
            //Si las diferencias acumuladas son igual o mayor a 40 se envia el paquete
               if((umbral>=40)or (flag==0)){
          
               n=sendto(sock,(char*)&punto,sizeof(punto),0,(const struct sockaddr *)&server,length);
               printf("umbral=%d\n",umbral);
             
             }
             

             
             
             umbral=0; //reiniciamos el umbral
   
             
         
             
              

             }
             auxi = auxi + 18;
             auxj = 0;
             
           }
          cont++;
          cont2++;
        
     

          
         cvShowImage("Envio", gris); //mostramos el video streaming localmente
         copy = cvCloneImage(gris); //realizamos una copia de la imagen gris nuevamente para compararla con la imagen que llega
         flag=1;
       
       
        
 
          char c = cvWaitKey(2);
        if( c == 27 ) break;
    }
    close(sock);
    cvReleaseCapture( &capture ); //liberamos captura
    cvDestroyWindow("Envio"); //liberamos el frame que muestra el streaming
  
}
