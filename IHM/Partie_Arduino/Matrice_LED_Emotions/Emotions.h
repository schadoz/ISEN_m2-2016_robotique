
/* déclaration de la structure emotion comprtant : 
- le nom de l emotion 
- le nombre de pattern qu il contiendra 
- le temps d attente entre 2 pattern 
- un tableau contenant les indices du pattern*/

typedef struct
{
   String name;
   int pattern_number;
   int pattern_delay;
   int emotion_array_eye_1[23];

} Emotion;

// On crée un tableau d'Emotion 
Emotion emotion[14];

// Tableau à double dimensions des differents pattern possibles des yeux 
int Eyes[55][8]={
 {60,66,129,153,153,129,66,60}, //0
 {60,66,129,141,141,129,66,60}, //1 
 {60,66,129,135,135,129,66,60}, //2 
 {60,66,129,177,177,129,66,60}, //3 
 {60,66,129,225,225,129,66,60}, //4
 {0,60,66,153,153,66,60,0}, //5
 {0,0,126,153,153,126,0,0}, //6
 {0,0,0,255,255,0,0,0}, //7
 {0,0,129,195,126,60,0,0}, //8
 {60,66,129,129,129,153,102,0}, //9
 {24,36,66,129,129,153,102,0}, //10
 {0,24,36,66,129,129,153,102}, //11
 {60,66,225,225,255,0,0,0}, //12
 {60,66,153,153,129,126,0}, //13
 {60,66,177,177,129,255,0,0}, //14
 {60,66,177,177,255,0,0,0}, //15
 {60,66,153,153,255,0,0,0}, //16
 {60,66,141,141,255,0,0,0}, //17
 {60,66,135,135,255,0,0,0}, //18
 {60,66,129,129,225,225,66,60}, //19
 {60,66,129,129,129,177,114,60}, //20
 {60,66,129,129,129,153,90,60}, //21
 {60,66,129,129,129,141,78,60}, //22
 {60,66,129,129,135,135,66,60}, //23
 {60,66,129,135,135,129,66,60}, //24
 {60,66,135,135,129,129,66,60}, //25
 {60,78,141,129,129,129,66,60}, //26
 {60,90,153,129,129,129,66,60}, //27
 {60,114,177,129,129,129,66,60}, //28
 {60,66,225,225,129,129,66,60}, //29
 {64,0,124,90,90,60,0,0}, //30
 {0,0,124,90,90,60,0,0}, //31
 {0,64,60,90,90,60,0,0}, //32
 {64,0,60,90,90,60,0,0}, //33
 {0,0,60,90,90,60,0,0}, //34
 {0,0,0,126,126,0,0,0}, //35
 {60,66,153,153,129,129,66,60}, //36
 {0,0,62,90,90,60,0,0}, //37
 {0,2,60,90,90,60,0,0}, //38
 {2,0,62,90,90,60,0,0}, //39
 {2,0,60,90,90,60,0,0}, //40
 {255,64,32,16,8,4,2,1}, //41
 {28,34,73,34,156,32,8,2}, //42
 {255,127,63,31,15,7,3,1}, //43
 {0,129,66,60,60,66,129,0}, //44
 {129,66,36,24,24,36,66,129},//45
 {0,8,42,28,127,28,42,8}, //46
 {0,0,0,24,24,0,0,0}, //47
 {0,0,0,28,28,28,0,0}, //48
 {0,0,42,28,62,28,42,0}, //49
 {0,8,42,28,127,28,42,8}, //50
 {64,232,72,28,127,28,8,8}, //51
 {112,248,112,36,14,31,14,4}, //52
 {16,16,56,254,56,18,23,2}, //53
 {255,255,255,255,255,255,255,255}, //54
};






