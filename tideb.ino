#include <Keyboard.h>

typedef struct branche * Branche;
struct branche{
  int pin;
  int val;
  int* vals;
  int resistivite;
  int led;
  int toggle;
  char lettre;
};

/* Seuil d'activation
 * Le seuil d'activation est la valeur lue au pin A0
 */
int seuil = 10000;
int seuilPin = A0;

void update_seuil(){
  int tmp = analogRead(seuilPin);
  tmp = analogRead(seuilPin);
  seuil = tmp * 10;
  if(seuil > 10000){
    seuil = 10000;
  }
}

/* Constructeur de branche
 * "pin" correspond au pin analogique (A1, A2, A3, A4 ou A5 ici. A0 est réservé au seuil d'activation dans ce montage)
 * "led" correspond au pin digital sur lequel est branchée la LED (9, 10, 11, 12 ou 13 ici)
 * "frame" correspond à la taille du tableau utilisé pour le calcul de la résistivité.
 *         Un tableau plus grand réduit le bruit et les activations non nécessaires, un tableau plus petit est plus réactif.
 * "lettre" correspond au caractère qui sera écrit lors d'une activation de la branche.
 */
Branche creer_branche(int pin, int led, int frame, char lettre){
  Branche b = (Branche) malloc(sizeof(struct branche));
  b->pin = pin;
  b->val = 0;
  b->vals = (int *) malloc(sizeof(int)*frame);
  b->resistivite = 0;
  b->led = led;
  pinMode(b->led, OUTPUT);
  b->toggle = 0;
  b->lettre = lettre;
  return b;
}

// Variables globales
int frame = 10;
Branche b1, b2, b3, b4, b5;

void update_branche(Branche b){
  /* Mise à jour de la valeur du pin.
   *  Il est nécessaire de le faire plusieurs fois car les arduinos agissent parfois de manière surnaturelle
   *  et seuls des méthodes alternatives permettent de les exorciser.
   *  (NDA : Les pins ne peuvent être lus qu'un à la fois et il reste parfois des artéfacts des lectures précédentes. Il faut donc forcer un peu)
   */
  b->val = analogRead(b->pin);
  b->val = analogRead(b->pin);
  b->val = analogRead(b->pin);
  for (int n = 0; n < frame-1 ; n++){
    b->vals[n] = b->vals[n+1];
  }
  b->vals[frame-1] = b->val;

  b->resistivite = 0;

  for (int n = 0 ; n < frame ; n++){
    b->resistivite = b->resistivite + b->vals[n];
  }
}

/* Mise à jour de la LED
 * Allume la LED et appuie sur la touche correspondants à la branche b si la résistivité est inférieure au seuil.
 * La variable "toggle" sert à éviter le spam de la fonction "press" qui peut entraîner des problèmes de "touches rémanentes"
 * sur certains systèmes d'exploitations maudits (Qui commencent par un W et se terminent par un S).
 */
void update_led(Branche b){
  if(b->resistivite < seuil){
    if(b->toggle == 0){
      b->toggle = 1;
      Keyboard.press(b->lettre);
    }
    digitalWrite(b->led, HIGH);
  }
  else{
    if(b->toggle){
      b->toggle = 0;
      Keyboard.release(b->lettre);
    }
    digitalWrite(b->led, LOW);
  }
}

void setup() {
  Serial.begin(9600);

  //Initialisation des branches
  b1 = creer_branche(A1, 13, frame,  'T');
  b2 = creer_branche(A2, 12, frame, 'I');
  b3 = creer_branche(A3, 11, frame, 'D');
  b4 = creer_branche(A4, 10, frame, 'E');
  b5 = creer_branche(A5, 9, frame, 'B');
  
}

void loop() {
  // put your main code here, to run repeatedly:
  update_seuil();

  //Mise à jour des valeurs des branches
  update_branche(b1);
  update_branche(b2);
  update_branche(b3);
  update_branche(b4);
  update_branche(b5);
  //Activation des leds et des touches
  update_led(b1);
  update_led(b2);
  update_led(b3);
  update_led(b4);
  update_led(b5);

  //Affichage (Tout est mis sur la même ligne pour observer avec plus d'aisance)
  Serial.print("[");
  Serial.print(seuil);
  Serial.print(", ");
  Serial.print(b1->resistivite);
  Serial.print(", ");
  Serial.print(b2->resistivite);
  Serial.print(", ");
  Serial.print(b3->resistivite);
  Serial.print(", ");
  Serial.print(b4->resistivite);
  Serial.print(", ");
  Serial.print(b5->resistivite);
  Serial.print("]\n");
}
