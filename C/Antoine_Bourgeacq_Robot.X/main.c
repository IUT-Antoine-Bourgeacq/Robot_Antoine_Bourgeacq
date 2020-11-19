#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <libpic30.h>
#include "ChipConfig.h"
#include "ADC.h"
#include "IO.h"
#include "timer.h"
#include "PWM.h"
#include "Toolbox.h"
#include "Robot.h"
#include "main.h"
#include "UART.h"
#include "CB_RX1.h"
#include "CB_TX1.h"


unsigned char stateRobot;

int main(void) {
    /****************************************************************************************************/
    // Initialisation des fonctions
    /****************************************************************************************************/

    InitIO();
    InitPWM();
    InitADC1();
    InitTimer1();
    InitTimer4();
    InitOscillator();
    InitTimer23();
    InitUART(); 
    
    /****************************************************************************************************/
    // Boucle Principale
    /****************************************************************************************************/
    while (1) {
        
        SendMessage((unsigned char*) "Bonjour", 7);
        __delay32(40000000);
        
        if (ADCIsConversionFinished() == 1) {
            ADCClearConversionFinishedFlag();
            unsigned int * result = ADCGetResult();
            
            float volts = ((float) result[2])*3.3 / 4096 * 3.2;
            robotState.distanceTelemetreDroit = 34 / volts - 5; // DROIT
            
            volts = ((float) result[1])*3.3 / 4096 * 3.2;
            robotState.distanceTelemetreCentre = 34 / volts - 5; // CENTRE
            
            volts = ((float) result[0])*3.3 / 4096 * 3.2; // GAUCHE
            robotState.distanceTelemetreGauche = 34 / volts - 5;
            
            volts = ((float) result[3])*3.3 / 4096 * 3.2; // EXTREME GAUCHE
            robotState.distanceTelemetreXGauche = 34 / volts - 5;
            
            volts = ((float) result[4])*3.3 / 4096 * 3.2; // EXTREME DROITE
            robotState.distanceTelemetreXDroit = 34 / volts - 5;



            if (robotState.distanceTelemetreDroit <= 25)
                LED_BLANCHE = 1;
            else {
                LED_BLANCHE = 0;
            }
            if (robotState.distanceTelemetreCentre <= 25)
                LED_BLEUE = 1;
            else {
                LED_BLEUE = 0;
            }
            if (robotState.distanceTelemetreGauche <= 25)
                LED_ORANGE = 1;
            else {
                LED_ORANGE = 0;
            }
        }
    }
}

void OperatingSystemLoop(void) {
    switch (stateRobot) {
        case STATE_ATTENTE:
            timestamp = 0;
            PWMSetSpeedConsigne(0, MOTEUR_DROIT);
            PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
            stateRobot = STATE_ATTENTE_EN_COURS;

        case STATE_ATTENTE_EN_COURS:
            if (timestamp > 1000)
                stateRobot = STATE_AVANCE;
            break;

        case STATE_AVANCE:      // AVANCER
            PWMSetSpeedConsigne(-20, MOTEUR_DROIT);
            PWMSetSpeedConsigne(-20, MOTEUR_GAUCHE);
            stateRobot = STATE_AVANCE_EN_COURS;
            break;
        case STATE_AVANCE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_TOURNE_GAUCHE:   //TOURNE GAUCHE
            PWMSetSpeedConsigne(-20, MOTEUR_DROIT);
            PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_GAUCHE_EN_COURS;
            break;
        case STATE_TOURNE_GAUCHE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_TOURNE_DROITE:   //TOURNE DROIT
            PWMSetSpeedConsigne(0, MOTEUR_DROIT);
            PWMSetSpeedConsigne(-20, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_DROITE_EN_COURS;
            break;
        case STATE_TOURNE_DROITE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_TOURNE_SUR_PLACE_GAUCHE:
            PWMSetSpeedConsigne(-20, MOTEUR_DROIT);
            PWMSetSpeedConsigne(20, MOTEUR_GAUCHE); 
            stateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS;
            break;
        case STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_TOURNE_SUR_PLACE_DROITE:
            PWMSetSpeedConsigne(20, MOTEUR_DROIT);
            PWMSetSpeedConsigne(-20, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS;
            break;
        case STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;
            
        default:
            stateRobot = STATE_ATTENTE;
            break;
    }
}

unsigned char nextStateRobot = 0;

void SetNextRobotStateInAutomaticMode() {
    unsigned char positionObstacle = PAS_D_OBSTACLE;

    //Détermination de la position des obstacles en fonction des télémètres
    if (robotState.distanceTelemetreDroit < 30 &&
            robotState.distanceTelemetreCentre > 20 &&
            robotState.distanceTelemetreGauche > 20) //Obstacle à droite
        positionObstacle = OBSTACLE_A_DROITE;
    
    else if (robotState.distanceTelemetreDroit > 20 &&
            robotState.distanceTelemetreCentre > 20 &&
            robotState.distanceTelemetreGauche < 30) //Obstacle à gauche
        positionObstacle = OBSTACLE_A_GAUCHE;
    
    else if (robotState.distanceTelemetreCentre < 20 &&
             robotState.distanceTelemetreDroit < 20 &&
            robotState.distanceTelemetreGauche < 20) 
        positionObstacle = OBSTACLE_EN_FACE; //Obstacle en face
    
    else if (robotState.distanceTelemetreCentre < 20 && 
            robotState.distanceTelemetreDroit < 20  &&
            robotState.distanceTelemetreGauche > 20 )
        positionObstacle = OBSTACLE_EN_FACE_DROIT; //Obstacle en face et à droit 
    
    else if (robotState.distanceTelemetreCentre < 20 && 
            robotState.distanceTelemetreGauche < 20 &&
            robotState.distanceTelemetreDroit > 20 )
        positionObstacle = OBSTACLE_EN_FACE_GAUCHE; //Obstacle en face et à gauche
    
    else if (robotState.distanceTelemetreDroit > 20 &&
            robotState.distanceTelemetreCentre > 20 &&
            robotState.distanceTelemetreGauche > 20) //pas d?obstacle
        positionObstacle = PAS_D_OBSTACLE;
    
     else if (robotState.distanceTelemetreDroit < 20 &&
            robotState.distanceTelemetreCentre > 20 &&
            robotState.distanceTelemetreGauche < 20) //Problème face
        positionObstacle = PROBLEME_FACE;
    
     else if (robotState.distanceTelemetreDroit > 20 &&
            robotState.distanceTelemetreCentre < 20 &&
            robotState.distanceTelemetreGauche > 20) //Problème face 2
        positionObstacle = PROBLEME_FACE;
     
     else if (robotState.distanceTelemetreXDroit < 20 && 
             robotState.distanceTelemetreDroit > 20 &&
            robotState.distanceTelemetreCentre > 20 &&
            robotState.distanceTelemetreGauche > 20 &&
             robotState.distanceTelemetreXGauche > 20 ) //Obstacle Extreme DROIT
         positionObstacle = OBSTACLE_X_DROIT;
     
     else if (robotState.distanceTelemetreXDroit > 20 && 
             robotState.distanceTelemetreDroit > 20 &&
            robotState.distanceTelemetreCentre > 20 &&
            robotState.distanceTelemetreGauche > 20 &&
             robotState.distanceTelemetreXGauche < 20 ) //Obstacle Extreme DROIT
         positionObstacle = OBSTACLE_X_GAUCHE;
    

    //Détermination de l?état à venir du robot
    if (positionObstacle == PAS_D_OBSTACLE)
        nextStateRobot = STATE_AVANCE;
    else if (positionObstacle == OBSTACLE_A_DROITE)
        nextStateRobot = STATE_TOURNE_GAUCHE;
    else if (positionObstacle == OBSTACLE_A_GAUCHE)
        nextStateRobot = STATE_TOURNE_DROITE;
    else if (positionObstacle == OBSTACLE_EN_FACE)
        nextStateRobot = STATE_TOURNE_SUR_PLACE_DROITE;
    else if (positionObstacle == OBSTACLE_EN_FACE_DROIT)
        nextStateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE;
    else if (positionObstacle == OBSTACLE_EN_FACE_GAUCHE)
        nextStateRobot = STATE_TOURNE_SUR_PLACE_DROITE;
    else if (positionObstacle == PROBLEME_FACE)
        nextStateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE;
    else if (positionObstacle == OBSTACLE_X_DROIT)
        nextStateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE;
    else if (positionObstacle == OBSTACLE_X_GAUCHE)
        nextStateRobot = STATE_TOURNE_SUR_PLACE_DROITE;
    //Si l?on n?est pas dans la transition de l?étape en cours
    if (nextStateRobot != stateRobot-1)
        stateRobot = nextStateRobot; // Il y a un retour en arrière sur la dernière étape
 
}