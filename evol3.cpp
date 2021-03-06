#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <math.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <random>

#include <unistd.h>
#include <termios.h>

#define WIDTH           165
#define HEIGHT          35
#define PLANT_ENERGY    80
#define REP_ENERGY      200         //energy required for reproduction


#ifndef _COLORS_
#define _COLORS_

/* FOREGROUND */
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

#endif  /* _COLORS_ */

//using namespace std;

char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}

using std::cout;
using std::cin;
using std::vector;
using std::endl;
using std::thread;

int plants[HEIGHT][WIDTH];          //this contains the positions of the plants
int animalPos[HEIGHT][WIDTH];       //"      "           "               animals
int jungle[4] = {45, 10, 10, 10};    //this defines the jungle area
int sizeg;

int end = 0;
char command;

std::atomic<bool> terminate(false);

//PLANT RELATED FUNCTIONS


int random(int a, int b){
    std::random_device rdev{};
    static std::default_random_engine e{rdev()};
    static std::uniform_int_distribution<int> d{a,b};
    
    return d(e);
}


void randomPlant (int left, int top, int width, int height){
    int x,y;
    x = left + (rand()%width);
    y = top + (rand()%height);
    
    //this thing marks the pos. of plant as 1 in the plants array
    plants[y][x] = 1;
}

void growPlant () {
    randomPlant(jungle[0], jungle[1], jungle[2], jungle[3]);    //grows plant in jungle
    randomPlant(0, 0, WIDTH, HEIGHT);                           //grows plant elsewhere

    //randomPlant(0, 0, WIDTH, HEIGHT);
    //randomPlant(0, 0, WIDTH, HEIGHT);
}







// ANIMAL RELTED FUNCTIONS

class animal{
public:
    int x, y, dir, energy, age, speed;      //dir is the direction currently facing
    int gene[8];
    void moveAnimal ();
    void eatAnimal ();
    void turnAnimal ();
    void repAnimal (animal &a);
    void geneDisplay ();
    animal();                       //default constructor
    //animal(animal &a);              //copy constructor
};

animal::animal () {
    //x = WIDTH/2;
    //y = HEIGHT/2;
    
    x=50; y=15;
    age = 0;
    
    dir = 0;
    energy = 1000;
    
    speed = rand()%4 + 1; 
    
    for(int i=0; i<8; i++){         //randomly allocates the genes      //may cause difficulty in deciding reference (initial) traits.  
        gene[i]=random(1,10);//abs(rand()%10 + 1);
    }
}

int max(int a, int b){
    if(a>b) return a;
    if(b>a) return b;
    return 0;
}

void animal::repAnimal (animal &a) {

    
    x = a.x + 1;                        //copies stuff
    y = a.y + 1;
    dir = a.dir;
    
    energy = a.energy / 2;          //half energy to parent and child
    a.energy = a.energy / 2;
    
    for(int i=0; i<8; i++){         
        gene[i]=a.gene[i];
    }
    
    
    int geneChng, genePos;
    
                      //randomly chooses gene and in(de)crements by 1
    genePos = abs(rand()%8);
    geneChng = abs((rand()%5)) - 2;
    
    gene[genePos] = abs(gene[genePos] + geneChng);     //cant be negative
    
    if(gene[genePos]==0) gene[genePos] = 1;
}



void animal::turnAnimal () {
     
    int sum=0, num;
    
    for (int i=0; i<8; i++){
        sum += gene[i];
    }
    
    num = abs(rand()%sum + 1);
    for (int i=0; i<8; i++){
        num = num - gene[i];
        if (num <= 0){
            dir = (dir + i)%8; 
            break;
        }
    }
}



void animal::moveAnimal () {
    if (dir > 1 && dir < 5)            x = abs((x + 1)%WIDTH);     //this thing caused a lot of trouble
    if (dir==0 || dir==7 || dir==6)    x = abs((x - 1)%WIDTH);
    
    if (dir > 3 && dir < 7)            y = abs((y + 1)%HEIGHT);
    if (dir==0 || dir==1 || dir==2)    y = abs((y - 1)%HEIGHT);
    
    energy = energy - 1; 
}

void animal::eatAnimal () {
    if(plants[y][x] == 1){
        energy = energy + PLANT_ENERGY;
        plants[y][x] = 0;
    }
}



//WORLD RELATED FUNCTIONS
vector<animal> animals;                 //make a vector animal

void updateWorld(){
    int size;                                   //i do not know why i did this
    
    
    for(auto i=animals.begin(); i != animals.end(); ++i) {
        if ((i->energy) <= 0) {
            animals.erase(i);
            i--;
        }
    }
    
    size = animals.size();
    
    for(int i=0; i<size; i++){
        animals[i].turnAnimal();
        animals[i].moveAnimal();
        animals[i].eatAnimal();
        animals[i].age++;
        
        
        if(animals[i].energy >= REP_ENERGY){
            animals.emplace_back(animal());
            animals[animals.size()-1].repAnimal(animals[i]);
        }
    }
    
    
    
    growPlant();
    sizeg=size;
}
    


//UI related functions
void animal::geneDisplay() {
    cout<<"X:"<<"\033[1;34m"<<x<<"\033[0m";
    cout<<"     Y:"<<"\033[1;34m"<<y<<"\033[0m";
    cout<<"     DIR:"<<"\033[1;31m"<<dir<<"\033[0m";
    cout<<"     ENERGY:"<<"\033[1;31m"<<energy<<"\033[0m";
    //cout<<"     SPEED:"<<"\033[1;31m"<<x<<"\033[0m";
    cout<<"     AGE:"<<"\033[1;31m"<<age<<"\033[0m";
    cout<<"     GENE : ";
    for (int i=0; i<8; i++) cout<<"\033[1;37m"<<gene[i]<<"\033[0m"<<" ";
    cout<<endl;
}


//Layout Related Functions
void posDisplay()
{
    int i,j,k,flag=1;
    for(i=0; i<HEIGHT; i++) {
        for(j=0; j<WIDTH; j++){
            
            flag=1;                                //resetting the flag
            if(plants[i][j]==1) cout<<BOLD(FGRN("\u2633"));
            
            else {
                for(k=0;k<animals.size();k++){
                    if((animals[k].x==j)&&(animals[k].y==i)){
                        if(animals[k].energy < (0.2*REP_ENERGY)){
                            cout<<FRED("\u2688");
                        }
                        else if(animals[k].energy > (0.75*REP_ENERGY)){
                            cout<<BOLD(FBLU("\u2688"));
                        }
                        else{
                            cout<<BOLD(FWHT("\u2688"));
                        }
                        flag = 0;
                        break;
                    }
                }
                
            if(flag) cout<<" ";
            }         
        }
        cout<<endl;
    }
    for(j=0; j<WIDTH; j++) cout<<"_";
    cout<<"\nEnter Command \n";
}

void fun(){
    
    while(!terminate.load()){
        
        command = getch();
        if(command=='x'){
            end = 1;
            break;
        }
    }
}



void run(){
    
    long delay=100000;
    thread th1(fun);             //Multithreading !!!!!!!!!!!!
    
    while(1){
        //cout<<animals.size()<<endl;  animalPos[animals[i].y][animals[i].x] = 1;
        updateWorld();
        posDisplay();
        
        if (command == 'p'){                        //p - pause
            while(command != 's'){                  //s - start
                if(command == 'd'){
                    command = 0;
                    for(int i=0; i<animals.size(); i++) animals[i].geneDisplay();
                }
                if(command == 'x') break;           //x - end program
                usleep(100000);
            }
        }
        
        if (command == 'g'){                        //if you press g, it will increment by 1000 generatons
            command = 0;
            for(int i=0; i<1000; i++){ updateWorld(); }
            posDisplay();
        }
        
        if (command == '.'){
            command = 0;
            delay -= 10000;
        }
        
         if (command == ','){
            command = 0;
            delay += 10000;
        }
        
        if (delay<0) delay = 10000;
        usleep(delay);
        system("clear");
        
        if (animals.size()==0){
            cout<<"All animals died. Press x to end the program"<<endl;
            terminate = true;
            break;
        }
        
        
        if (end == 1) break;
    }
    
    th1.join();                           //waits for the thread function to end
}


int main(){
    srand(time(0));
    
    animals.reserve(700);
    animals.emplace_back(animal());         //the initial animal
    
    /* animals[0].geneDisplay();
    
    for(int i=0; i<5; i++){
         animals.emplace_back(animal());
         animals[i+1].repAnimal(animals[i]);
         animals[i+1].geneDisplay();
    }
    
    cout<<animals.size()<<endl;
    
    animals.erase(animals.begin() + 2);
    
    
    for(int i=0; i<animals.size(); i++){
         animals[i].geneDisplay();
    }*/
    
    run();
    
    posDisplay();
    
    return 0;
    
} 
