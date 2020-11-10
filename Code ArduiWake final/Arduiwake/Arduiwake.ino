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

//import des bibliotheques:

//programme de Théophane
//*************************************************
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
//*************************************************

//Programme d'Éléonore
//=================================================
#include <SPI.h>
#include <WiFi101.h>
//=================================================

//réglage des GIPO:

//programme de Théophane
//*************************************************
#define HEURES 2
#define MINUTES 9
#define AM 24
#define PM 25
#define LEDREVEILACTIF 26
#define BTNHEUREMIN 28
#define BTNHORLOGEREVEIL 29
#define BTNREVEILACTIF 31
#define NEOPIXEL 4
#define POTENTIOMETRE A15
#define BUZZER 42
//*************************************************

//Initialisation des variables:

//programme de Théophane
//*************************************************
boolean switchHeureMinute = false;
boolean modeHorloge = true;
boolean modeReveil = false;
boolean reveilActif = false;
boolean modeWifiManuel;
int heurePrecedente;
int heureDuReveil;
int minuteDuReveil;
int decalageHoraire;
int heureActuelleEnMinute;
int minuteAvantHeureDuReveil;
int heureDuReveilEnMinute;
int heureActuelle;
int minuteActuelle;
boolean leReveilASonne = 0;
int rouge = 255;
int vert = 255;
int bleu = 255;
int intensite []= {5,7,8,10,13,15,19,23,28,34,42,51,62,76,93,114,139,170,207,255};

//initialisation de la guirlande de LED
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, NEOPIXEL, NEO_GRB + NEO_KHZ800);
//*************************************************

//Programme d'Éléonore
//=================================================
char ssid[] = "arduiwake"; //  nom du reseau 
char pass[] = "arduiwake";    // nom du mot de passe

String Chaine = "";// ce qui est reçu du site web/enregistrement des parametres sous forme de chaine

String SN = "123456"; // No de serie de l'appareil a lire dans l'EPROM (a faire). 
                      // Il faudra prealablement l'inscrire en EPROM (autre programme)


//----------- Variables de gestion des parametres du reveil-----------

//Stockage des paramètres
String HH; // Heure du reveil 
String MN ; // Minutes du reveil
String FH; // Fuseau horaire
String R;  // composante R
String V;  // composante V
String B; // Composante B
String H_HH; // Heures de l'horloge
String H_MN; // Minutes de l'horloge
String R_A; //réveil actif

boolean HH_Modifiee;//  Heure du reveil modifiee
boolean MN_Modifiee;// Minutes du reveil modifiees
boolean FH_Modifiee; // Fuseau horaire
boolean R_Modifiee;  // composante R modifiee
boolean V_Modifiee;  // composante V modifiee
boolean B_Modifiee; // Composante B modifiee
boolean H_HH_Modifiee; // heures de l'horloge modifiees
boolean H_MN_Modifiee; // minutes de l'horloge modifiees
boolean R_A_Modifiee; //réveil actif

String w_SN = ""; // pour le controle
String w_HH = ""; // Heures du reveil
String w_MN = ""; //Minutes du reveil
String w_FH = ""; // Fuseau horaire
String w_R = ""; // Rouge
String w_V = ""; // Vert
String w_B = ""; // Bleu
String w_H_HH = ""; // Heures actuelle
String w_H_MN = ""; //Minutes actuelle
String w_R_A; //réveil actif
//---------------------------------------------------

int status = WL_IDLE_STATUS;//Statut du Wifi

char serveur[] = "arduiwake.ddns.net";    // nom de l'adresse pour le site ArduiWake
int port=80 ;

WiFiClient client; //Client wifi = celui qui permet d'etablir la connexion avec le site web
//=================================================

void setup() {
  Serial.println("je suis bien en mode manuel");
  //SPI.begin();
  Serial.begin(9600); //pour le débug, ouvre la console
  
  //Programme d'Éléonore
  //=================================================
  // verifier la presence du shield ainsi que s'il est fonctionel
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield non present");
    // ne pas continuer
    while (true);
  }
  //=================================================

  //Programme de Théophane
  //*************************************************
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
  strip.begin();
  strip.show(); 
  //*************************************************
}

void loop() {
    reveilEnModeWifi();
    //Serial.println("je suis bien en mode wifi");
    reveilEnModeManuel();
   // Serial.println("je suis bien en mode manuel");
  if (reveilActif == true){
  //on test s'il est l'heure de reveiller l'utilisateur
  reveilerUtilisateur(heureDuReveil, minuteDuReveil);
  digitalWrite(LEDREVEILACTIF, HIGH);
 }
 else {
  stopReveil();
  Serial.println("Le reveil est desactive");
  digitalWrite(LEDREVEILACTIF, LOW);
 }
 Serial.print("heure du réveil :  ");
 Serial.print(heureDuReveil);
 Serial.print(":");
 Serial.println(minuteDuReveil);
}

//Programme de Théophane
//*************************************************
// Mode Réveil manuel
void reveilEnModeManuel() {
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
    AffichageHeure(heureActuelle, minuteActuelle);
  }
  //Si c'est sur la position Réglage de l'heure du réveil
  else{
    reglageHeureMinuteDuReveil();  
   }
  //Si le réveil est activé
  boolean wReveilActif = changementValeurBouton(reveilActif, BTNREVEILACTIF);
  if (wReveilActif != reveilActif){
    reveilActif = wReveilActif;
  setWifiReglages("R_A", String(reveilActif));
    }
  delay (125);
}

//Affichage de l'heure sur les galvanometres
void AffichageHeure(int heureAAfficher, int minuteAAfficher) { 
    //si l’heure est entre 0 et 11
    if (heureAAfficher < 12 ){
     //on utilise directement la valeur
      analogWrite(HEURES, 255/12*(heureAAfficher)+decalageHoraire);
      Serial.print("Heure : ");
      Serial.println((heureAAfficher)+decalageHoraire);
      digitalWrite(AM, HIGH);
      digitalWrite(PM, LOW);
    }
    else{
     //Sinon on est l’apres midi, on retranche 12
      analogWrite(HEURES, 255/12*(heureAAfficher-12)+decalageHoraire);
      Serial.print("Heure : ");
      Serial.println((heureAAfficher-12)+decalageHoraire);
      digitalWrite(AM, LOW);
      digitalWrite(PM, HIGH);
    }
     //on utilise la valeur pour les minutes directement
     analogWrite(MINUTES, 255/60*(minuteAAfficher));
     Serial.print("Minute : ");
     Serial.println(minuteAAfficher);
   delay (75);
}

//fonction qui s'occupe du réglage de l'heure du réveil en deux temps (heures puis minutes)
void reglageHeureMinuteDuReveil(){
  //On entre dans la fonction changementValeurBouton () pour dire si on regle l'heure ou les minutes
    switchHeureMinute = changementValeurBouton (switchHeureMinute, BTNHEUREMIN);
    //si switchHeureMinute est à 1 (en mode heure)
    if (switchHeureMinute == false){
      while (switchHeureMinute == false && modeHorloge == false){
        //récupération de la valeur du potentiometre
        // reglage des Heures:
        heureDuReveil = analogRead (POTENTIOMETRE)/(1023/23);
        Serial.println(heureDuReveil);
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
     switchHeureMinute = changementValeurBouton (switchHeureMinute, BTNHEUREMIN);
     modeHorloge = changementValeurBouton (modeHorloge, BTNHORLOGEREVEIL);
     //on reste dans cette boucle tant que l'utilisateur n'a pas appuyé sur le bouton pour changer de mode
     delay (125);
     }
     setWifiReglages("HH", String(heureDuReveil));
    }
  //sinon est en mode minute
  else{
      while (switchHeureMinute == true && modeHorloge == false){
       //récupération de la valeur du potentiometre
       //réglage des Minutes:
       minuteDuReveil = analogRead (POTENTIOMETRE)/17.338983051;
       Serial.println(minuteDuReveil);
       analogWrite(MINUTES, 255/60*(minuteDuReveil));
       //on regarde si l'utilisateur a changé de mode (heure/minute)
       switchHeureMinute = changementValeurBouton (switchHeureMinute, BTNHEUREMIN);
       modeHorloge = changementValeurBouton (modeHorloge, BTNHORLOGEREVEIL);
       //on reste dans cette boucle tant que l'utilisateur n'a pas appuyé sur le bouton pour changer de mode
       delay (125);
     }
     setWifiReglages("MN", String(minuteDuReveil));
   }
}

//Fonction qui sert à réveiller l'utilisateur progressivement durant 20 min
void reveilerUtilisateur(int heure, int minute) {
  //on chage le format de l'heure du reveil en une seule variable
  heureDuReveilEnMinute = minute+60*heure;
    //on chage le format de l'heure actuelle en une seule variable
    heureActuelleEnMinute = heureActuelle*60+minuteActuelle;
    // nombre de minutes restantes avant l'heure du reveil
    minuteAvantHeureDuReveil = heureDuReveilEnMinute-heureActuelleEnMinute; 
      if (minuteAvantHeureDuReveil > -1 && minuteAvantHeureDuReveil < 20){
        leReveilASonne = 1;
        int c=19-minuteAvantHeureDuReveil;
        if (heurePrecedente != heureActuelleEnMinute){
          //on allume la guirelande progressivement
          for(int i = 0; i<=59; i++){
            strip.setPixelColor(i, (intensite[c]*rouge)/255, (intensite[c]*vert)/255, (intensite[c]*bleu)/255);
          }
          strip.show();
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

//Fonction qui sert à stopper le réveil
void stopReveil (){
  Serial.println("c'est sencé sonner");
  noTone(BUZZER);
  for(int i = 0; i<=59; i++){
        strip.setPixelColor(i, 0, 0, 0);
        }
  strip.show();
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
  delay (175);
  return x;
}
//*************************************************

//Programme d'Éléonore
//=================================================

//Mode Réveil WiFi
void reveilEnModeWifi (){
  Check_Wifi_Reglages();// Recuperer les reglages et les separer

  delay(10);// on peut modifier le delai ca depend de la qualite de la connexion
  if (HH_Modifiee || MN_Modifiee || FH_Modifiee || R_Modifiee || V_Modifiee || B_Modifiee || H_HH_Modifiee || H_MN_Modifiee || R_A_Modifiee){

    //exploiter_les_nouveaux_reglages();
   // Serial.println("Nouveaux réglages détectés");
  }
  else{
   // Serial.println("Reglages non modifies");
  }
}

void setWifiReglages(String var, String val){
  if (!WifiConnect())//Creation liaison avec le wifi/ si on a pas reussi a se connecter au wifi arret
    return;          //de la fonction

  if (client.connect(serveur, port)) {// si le client arrive a se connecter au server et au port
    Serial.println("envoi des valeurs au site" );
    //Serial.print("connecte au serveur ");
   // Serial.println(serveur);
    String req = "GET /?SN=123456&#var=#val HTTP/1.1";
    req.replace("#var",var);
    req.replace("#val",val);
    // Faire une requete HTTP:
    client.println(req);// On va chercher le fichier texte
    client.print("Host: ");
    client.println(serveur);
    client.println("Connection: close");
    client.println();
    String res = client.readString();
    delay(300);
    Serial.print("valeurs envoyées ");
    Serial.println(res);
  }
}

void Check_Wifi_Reglages(){ 
  
  HH_Modifiee = false;
  MN_Modifiee = false;
  FH_Modifiee = false;
  R_Modifiee = false;
  V_Modifiee = false;
  B_Modifiee = false;

  //Serial.println("Se connecter au serveur ");

  /*if (!WifiConnect())//Creation liaison avec le wifi/ si on a pas reussi a se connecter au wifi arret
    return;          //de la fonction*/
    WifiConnect();

  if (client.connect(serveur, port)) {// si le client arrive a se connecter au server et au port

    //Serial.print("connecte au serveur ");
   // Serial.println(serveur);

    // Faire une requete HTTP:
    client.println("GET /?SN=123456 HTTP/1.1");// On va chercher le fichier texte
    client.print("Host: ");
    client.println(serveur);
    client.println("Connection: close");
    client.println();
    Chaine = "";
    delay(500);
    // Chaine = client.readString();
    while (client.available())// Tant que la reponse du server n'est pas completement recu:
    {
      char c = client.read();// recevoir un caractere du fichier txt.
      Chaine += c; // lire la chaine CARACTERE FINAL COMPRIS
    }
    if (Chaine == ""){ 
    Chaine = client.readString();
    }
    Serial.println("Chaine=" + Chaine); // imprimer le contenu de la requete (a supprimer lorsque  tout marchera)

    w_SN = Valeur("SN"); // extraire la valeur de SN de la chaine de caractère

    if (w_SN != "")
    {

      w_HH = Valeur("HH"); // Heures du reveil
      w_MN = Valeur("MN"); // Minutes du reveil
      w_FH = Valeur("FH"); // Fuseau horaire
      w_R = Valeur("R"); // Rouge
      w_V = Valeur("V"); // Vert
      w_B = Valeur("B"); // Bleu
      w_H_HH = Valeur("H_HH"); // Heure de l'horloge
      w_H_MN = Valeur("H_MN"); // Minute de l'horloge
      w_R_A = Valeur("R_A"); //Reveil actif

      // Comparaison des valeurs : (peut etre a stocker ds l' EPROM)
      HH_Modifiee = (w_HH != HH); // Heures reveil modifiees ou pas
      MN_Modifiee = (w_MN != MN); // Minutes modifiees ou pas
      FH_Modifiee = (w_FH != FH); // Fuseau horaire modifie ou pas
      R_Modifiee = (w_R != R); // ...
      V_Modifiee = (w_V != V); // ...
      B_Modifiee = (w_B != B); // ...
      H_HH_Modifiee = (w_H_HH != H_HH); // ...
      H_MN_Modifiee = (w_H_MN != H_MN); // ...
      R_A_Modifiee = (w_R_A != R_A); // ... 

      HH = w_HH;
      MN = w_MN;
      FH = w_FH;
      R = w_R;
      V = w_V;
      B = w_B;
      H_HH = w_H_HH;
      H_MN = w_H_MN;
      R_A = w_R_A;

      // Unification des deux codes
      //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

      heureDuReveil = HH.toInt(); // .toInt() permet de transformer une String en int
      minuteDuReveil = MN.toInt();
      decalageHoraire = FH.toInt();
      rouge = R.toInt();
      vert = V.toInt();
      bleu = B.toInt();
      heureActuelle = H_HH.toInt();
      minuteActuelle = H_MN.toInt();
      reveilActif = R_A.toInt();
     /* Serial.print("Heure : ");  // pour les tests
      Serial.println(heureActuelle); //...
      Serial.print("minute : ");//...
      Serial.println(minuteActuelle);//...*/
      //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    }// Mettre en Eprom aussi
  }
  else
  {
    Serial.print("Pas connecte au serveur ");
    
  }

}

bool WifiConnect(){
  // tente de se connecter au resau wifi:
  status = WiFi.status();//renvoi le statut du wifi
  if (status == WL_CONNECTED) {
    return true;// renvoi un valeur vrai/faux --> boolean
  }

  Serial.print("Tente de se connecter au SSID: ");
  Serial.println(ssid); //nom du reseau
  // Connexion au reseau WPA/WPA2. A changer si on utilise un reaseau WEP:    
  status = WiFi.begin(ssid, pass);// demande de connaxion wifi

  // attendre 10 secondes pour la connion:

  //printWifiStatus();
  status = WiFi.status();//reccuperation du statut wifi
  if (status == WL_CONNECTED){ // Si connecté
    Serial.println("Connecté");
    return true; //Connecté / Youpi !!!!!
  }
  else {
    Serial.println("Pas connecté au Wifi");
    return false; //Sinon on est pas connecté, il y a retour a la fonction CheckWifiReglages 
  }


}

//----------SEPARATION DES VARIABLES----------------------------------------------------

// recherche la variable dans Chaine 
// et retourne le contenu de la ligne suivante de la ligne contenant la variable
// ex : dans le fichier txt on a :
//SN
//123456
// On va donc retourner le contenu de la ligne (123456 dans la ligne contenant la variable (SN)

String Valeur(String Variable) {// Identification d'une Variable

  unsigned int i = 0; // index du caractere courant dans la Chaine
  String var = ""; // sera la variable Token lorsqu'elle sera identifiee dans Chaine ex: SN
  String val = ""; // sera la valeur retournee qui correspondra a la variable ex: val=123456 pour var=SN

  // chaque ligne est delimitee 1 ou 2 caracteres de fin de ligne CR (\r) et LF(\n)
  Chaine.replace("\r", ""); //suppression du caractere parasite \r (retour a la ligne)
  String temp = "";
  while (i < Chaine.length()) //tant que tous les caracteres de la chaine ne sont pas traites
  {
    String t = Chaine.substring(i, i + 1);// t est le i-eme caractere de chaine 
    if (t == "\n"){// si ce caractere est une fin de ligne alors
      if (var == ""){//tout ce qui a ete lu depuis le dernier caractere de fin de ligne est ""
        if (temp.compareTo(Variable) == 0)
          var = temp;
        temp = "";
      }
      else{
        val = temp;
        Serial.println(var + "=" + val);
        return val; // si la variable et sa valeur sont trouvees alors c'est bon, 
                    //on retourne la valeur trouvee
      }
    }
    else
      temp += t;// sinon on passe au caractère suivant

    i++;
  }

  return val;
}
// ---------------------------------------------------------------------------------------


// ------- Pour debug uniquement, on pourrait l'utiliser pour indiquer sur une led le statut 
//de la connection WIFI par ex : rouge pour "pas conecté" et vert pour "connecté"-----------

void printWifiStatus() {
  //print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
//=================================================
