/* SETUP :
 * bouton d'activation du reveil  Pin 12
 * bouton mode réveil/heure       Pin 11
 * Neopixel Strip                 Pin 10
 * Buzzer3                        Pin 9
 * bouton poussoir heure/minutes  Pin 8
 * bouton poussoir heure reveil   Pin 7
 * Galvanometre heure             Pin 6
 * Galvanometre minutes           Pin 5
 * LED Matin                      Pin 4
 * LED Apres Midi                 Pin 3
 * LED reveil activé ou non       Pin 2
 * Potentiometre                  Pin A0
 */

//import des bibliotheques
#include <Wire.h>
#include <Time.h>
//#include <DS1307RTC.h>
//#include <Adafruit_NeoPixel.h>

//réglage des GIPO
#define HEURES 7
#define MINUTES 10
#define AM 24
#define PM 25
#define LEDREVEILACTIF 26
#define BTNHEUREMIN 28
#define BTNHORLOGEREVEIL 29
#define BTNREVEILACTIF 31
#define NEOPIXEL 52
#define POTENTIOMETRE A15
#define BUZZER 42

//Initialisation des variables
boolean switchHeureMinute = false;
boolean modeHorloge = true;
boolean modeReveil = false;
boolean reveilActif = false;
int heurePrecedente;
int heureDuReveil;
int minuteDuReveil;
int decalageHoraire;
int heureActuelleEnMinute;
int minuteAvantHeureDuReveil;
int heureDuReveilEnMinute;
boolean leReveilASonne = 0;
int rouge = 255;
int vert = 255;
int bleu = 255;
int intensite []= {5,7,8,10,13,15,19,23,28,34,42,51,62,76,93,114,139,170,207,255};

//initialisation de la guirlande de LED
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, NEOPIXEL, NEO_GRB + NEO_KHZ800);

//Mise en place qui ne s'effectue qu'une seule fois au debut
void setup() { 
  Serial.begin(9600);
  //on défini les pin en tant qu'entrée ou sortie
  pinMode (HEURES, OUTPUT);
  pinMode (MINUTES, OUTPUT);
  pinMode (AM, OUTPUT);
  pinMode (PM, OUTPUT);
  pinMode (LEDREVEILACTIF, OUTPUT);
  pinMode (BTNREVEILACTIF, INPUT);
  pinMode (NEOPIXEL, OUTPUT);
  pinMode (BTNHEUREMIN, INPUT);
  pinMode (BTNHORLOGEREVEIL, INPUT);

  //préparation de la guirlande de LED
//  strip.begin();
//  strip.show(); 
}

//Fonction principale qui se répete en boucle infinie tant que l'arduino est allimenté
void loop() {

 //On vérifie si l'utilisateur a appuié sur le bouton pour passer en mode Affichage de l'heure ou réglage du réveil
 modeHorloge = changementValeurBouton (modeHorloge, BTNHORLOGEREVEIL); //Mettre en constante mode (horloge ou reveil)
 if (modeHorloge==1){
      modeReveil=0;
    }
    else{
      modeReveil=1;
      }
 //si on est en mode affichage de l'heure
 if(modeHorloge == true){
  //affichage de l'heure:
  recuperationAffichageHeureCourrante();
 }
 //Si c'est sur la position Réglage de l'heure du réveil
 else{
  reglageHeureMinuteDuReveil();  
 }
 //Si le réveil est activé
 reveilActif = changementValeurBouton(reveilActif, BTNREVEILACTIF);
 if (reveilActif == true){
  //on test s'il est l'heure de reveiller l'utilisateur
  reveilerUtilisateur(heureDuReveil,minuteDuReveil);
  digitalWrite(LEDREVEILACTIF, HIGH);
 }
 else {
  stopReveil();
  Serial.println("Le reveil est desactive");
  digitalWrite(LEDREVEILACTIF, LOW);
  }
}

//Fonction qui récupere l'heure du module horloge et affiche l'heure sur les galvanometres
void recuperationAffichageHeureCourrante() { 
  //on stock l’heure
  tmElements_t tm;
  //affichage de l'heure
  //puis-je lire l’heure ?
  if(RTC.read(tm)) {
    //oui
    //si l’heure est entre 0 et 11
    if (tm.Hour < 12 ){
     //on utilise directement la valeur
      analogWrite(HEURES, 255/12*(tm.Hour)+decalageHoraire);
      digitalWrite(AM, HIGH);
      digitalWrite(PM, LOW);
    }
    else{
     //Sinon on est l’apres midi, on retranche 12
      analogWrite(HEURES, 255/12*(tm.Hour-12)+decalageHoraire);
      digitalWrite(AM, LOW);
      digitalWrite(PM, HIGH);
    }
     //on utilise la valeur pour les minutes directement
     analogWrite(MINUTES, 255/60*(tm.Minute));
   }
  else {
    //on ne peut pas lire l’heure
    if (RTC.chipPresent()){
        analogWrite (HEURES, 0);
        delay(200);
        analogWrite (HEURES,255);
        delay(200);
      
    } else {
      //probleme avec le module horloge
      //il faut verifier les connexions
      analogWrite (MINUTES, 0);
      delay(200);
      analogWrite (MINUTES,255);
      delay(200);
    }
   }
   delay (150);
}

//fonction qui s'occupe du réglage de l'heure du réveil en deux temps (heures puis minutes)
void reglageHeureMinuteDuReveil(){
  //On entre dans la fonction changementValeurBouton () pour dire si on regle l'heure ou les minutes
//    switchHeureMinute = changementValeurBouton (switchHeureMinute, BTNHEUREMIN);
    //si switchHeureMinute est à 1 (en mode heure)
    if (switchHeureMinute == false){
      while (switchHeureMinute == false && modeHorloge == false){
        //récupération de la valeur du potentiometre
        // reglage des Heures:
        heureDuReveil = analogRead (POTENTIOMETRE)/(1023/23);
        //on regle un heure le matin
        if (heureDuReveil < 12 ){
          analogWrite(HEURES, 255/12*(heureDuReveil));
          digitalWrite(AM, HIGH);
          digitalWrite(PM, LOW);
        }
        //on regle une heure l'apres midi
        else{
         analogWrite(HEURES, 255/12*(heureDuReveil-12));
         digitalWrite(AM, LOW);
         digitalWrite(PM, HIGH);
       }
     //on regarde si l'utilisateur a changé de mode (heure/minute)
//     switchHeureMinute = changementValeurBouton (switchHeureMinute, BTNHEUREMIN);
     modeHorloge = changementValeurBouton (modeHorloge, BTNHORLOGEREVEIL);
     //on reste dans cette boucle tant que l'utilisateur n'a pas appuyé sur le bouton pour changer de mode
     delay (150);
     }
    }
  //sinon est en mode minute
  else{
      while (switchHeureMinute == true && modeHorloge == false){
       //récupération de la valeur du potentiometre
       //réglage des Minutes:
       minuteDuReveil = analogRead (POTENTIOMETRE)/17.338983051;
       analogWrite(MINUTES, 255/60*(minuteDuReveil));
       //on regarde si l'utilisateur a changé de mode (heure/minute)
//       switchHeureMinute = changementValeurBouton (switchHeureMinute, BTNHEUREMIN);
//       modeHorloge = changementValeurBouton (modeHorloge, BTNHORLOGEREVEIL);
       //on reste dans cette boucle tant que l'utilisateur n'a pas appuyé sur le bouton pour changer de mode
       delay (150);
     }
   }
}

//Fonction qui sert à réveiller l'utilisateur progressivement durant 20 min
void reveilerUtilisateur(int heure, int minute) {
  //On stock l'heure
  tmElements_t tm;
  //on chage le format de l'heure du reveil en une seule variable
  heureDuReveilEnMinute = minute+60*heure;
  if(RTC.read(tm)){
    //on chage le format de l'heure actuelle en une seule variable
    heureActuelleEnMinute = tm.Hour*60+tm.Minute;
    // nombre de minutes restantes avant l'heure du reveil
    minuteAvantHeureDuReveil = heureDuReveilEnMinute-heureActuelleEnMinute; 
      if (minuteAvantHeureDuReveil > -1 && minuteAvantHeureDuReveil < 20){
        leReveilASonne = 1;
        int c=19-minuteAvantHeureDuReveil;
        if (heurePrecedente != heureActuelleEnMinute){
          //on allume la guirelande progressivement
          for(int i = 0; i<=59; i++){
//            strip.setPixelColor(i, (intensite[c]*rouge)/255, (intensite[c]*vert)/255, (intensite[c]*bleu)/255);
          }
 //         strip.show();
        }
      }
      Serial.println(minuteAvantHeureDuReveil);
    if (minuteAvantHeureDuReveil <= 0 && minuteAvantHeureDuReveil > -10 && leReveilASonne == 1) {
        tone(BUZZER, 4000);
        delay(100);
        noTone(BUZZER);
        delay (100);
    }
     heurePrecedente = heureActuelleEnMinute ;
  }
}

//Fonction qui sert à stopper le réveil
void stopReveil (){
  noTone(BUZZER);
  for(int i = 0; i<=59; i++){
//        strip.setPixelColor(i, 0, 0, 0);
        }
//  strip.show();
  leReveilASonne = 0;
}

//Fonction qui sert a changer de valeur (1 ou 0) un variable si on appuie sur un bouton
boolean changementValeurBouton (boolean x, int bouton) {
  //test des conditions
  if (digitalRead(bouton) == 0)//si bouton appuyé (donc le pin indique 0 car il est en mode INPUT_PULLUP)
  {
    if (x==1){
      x=0;
    }
    else{
      x=1;
      }
  }
  delay (75);
  return x;
}
