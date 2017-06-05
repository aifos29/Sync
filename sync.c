/*Lo que esta:Conecta con el servidor y el cliente. Si se pasa solo un parametro = servidor
 Dos parametros = cliente.
 El cliente usa, primero carpeta y luego ip
 Función getFiles(),almacena en una lista enlazada los archivos
 SearchData compara el nombre de los archivos.
 Ya hay una función que toma el tiempo actual y otra que va a tirar la diferencia horaria.
 
 * Lo que falta: 
 * Que se comparen archivos
 * Que se actualizen los datos
 * Que se pasen transfieran los archivos.
 
 */




#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <stdint.h>
#include <stdbool.h>

int difMinute;
int difHour;

struct list{
    char nameFile[256];
    char directoryName[256];
    struct list * next;
};

struct list * header;
struct list * finish;

struct dirent *dp;
struct stat statbuf;
struct passwd *pwd;
struct group *grp;
struct tm *tm;
char datestring[256];

void showList(){
    struct list * aux = header;
    while (aux!=NULL){
        printf("%s/%s\n"aux->nameFile);
        aux = aux->next;
    }
}

void addData (char * fileName, char * directoryName){
    
    struct list * tmp;
    tmp = malloc(sizeof(struct list));
    strcpy(tmp->nameFile,fileName);
    strcpy(tmp->directoryName,directoryName);
    if (header == NULL){
        header = tmp;
        finish = tmp;
    }
    else{
        finish->next = tmp;
        finish = tmp;
    }
    
}

int searchData(char * name,char *directory){
    struct list * aux = header;
    while (aux != NULL){
        if (strcmp(name,aux->nameFile)==0 && strcmp(directory,aux->directoryName)== 0){
            return 0;
        }
    }
    return 1;
}

void getFiles(char * name,int level){
  DIR *dir;
  struct dirent *entry;

  if (!(dir = opendir(name)))
      return;
  if (!(entry = readdir(dir)))
      return;
  do {
      if (entry->d_type == DT_DIR) {
          char path[1024];
          int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
          path[len] = 0;
          if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
              continue;
          getFiles(path, level + 1);
      }
      else
          addData(entry->d_name,name);
      
  } while (entry = readdir(dir));
  closedir(dir);
}



int server(){
    int socket_desc;
    int client_sock;
    int c; 
    int read_size;
    struct sockaddr_in server ;
    struct sockaddr_in client;
    char client_message[2000];

    
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
    }
    puts("Socket created");
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8889 );
  
    if( bind(socket_desc,(struct sockaddr *)&server,
        sizeof(server)) < 0) {
            perror("bind failed. Error");
            return 1;
    }
    puts("bind done");
   
    listen(socket_desc , 3);
   
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

   
    client_sock = accept(socket_desc,
    (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");
    
    while( (read_size = recv(client_sock ,client_message , 2000 , 0)) > 0 ) {
        if (strcmp(client_message,"¿Hora?")==0){
            time_t rawtime;
            struct tm * timeinfo;

            time ( &rawtime );
            timeinfo = localtime ( &rawtime );

            strcpy(client_message,asctime (timeinfo) );
        }


        send(client_sock , client_message , strlen(client_message),0);
      }
     if(read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
        } 
     else if(read_size == -1) {
        perror("recv failed");
     }
}

int client(char * ip){
    int sock;
    struct sockaddr_in server;
    char message[1000];
    char server_reply[2000];

    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1) {
        printf("Could not create socket");
    }
     puts("Socket created");
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons( 8889);

    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }
    puts("Connected\n");
   
    while(1) {
        printf("Enter message : ");
        scanf("%s" , message);
       
        if (send(sock,message,strlen(message),0) < 0) {
            puts("Send failed");
            return 1;
        }
       

        if( recv(sock , server_reply , 2000 , 0) < 0) {
            puts("recv failed");
            break;
        }
        puts("Server reply :");
        puts(server_reply);
}
    close(sock);
    return 0;
}

void calculateHour(){
  time_t t;
  struct tm *tm;
  char fechayhora[100];
  t=time(NULL);
  tm=localtime(&t);
  
  int hour = tm->tm_hour;
  int minute = tm->tm_min;
  int second = tm->tm_sec;
 
  printf ("Hora: %d:%d:%d\n", hour, minute, second);
    
    
}

void calculateDif(int hourS,int hourC){
    difHour = hourS - hourC;
}

int main(int argc, char** argv) {
    if (argc == 1){
        printf("No se especificaron parametros \n");
        calculateHour();
    }
    if(argc == 2){
        getFiles(argv[1],0);
        showList();
        fflush(stdout);
        server();
        
    }
    
    if (argc == 3){
        printf("Cliente \n");
        int ipSize = strlen(argv[2]);
        char ip[ipSize];
        strcpy(ip,argv[2]);
        
        client(ip);
    }
    
    
    
    
    return (EXIT_SUCCESS);
}

