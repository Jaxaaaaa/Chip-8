#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include <math.h>
#define MAX_RANGEES 10
#define MAX_COLONNES 20
int taille_fenetre_x = 800;
int taille_fenetre_y = 600;
Color bleu_nuit = {15, 20, 60, 255};
Color violet = {70, 30, 110, 255};
const char *message_debut = "Appuyez sur espace pour commencer la partie !";
int taille_police_debut = 14;
int vies = 3;
int score = 0;
//var du mur de briques
int nb_rangees = 5;
int nb_colonnes = 10;
int marge_haut = 40;
int ecart = 5;
int largeur_brique = 70;
int epaisseur_brique = 10;
int briques_restantes;
typedef enum {
    ACCUEIL,
    EN_COURS,
    PERDU,
    GAGNE
} Etat;

Etat etat = ACCUEIL;
typedef struct {
    float coord_x;
    float coord_y;
    float rayon;
    Color couleur;
    float vitesse_x;
    float vitesse_y;
    float vitesse_x_cible;
} Balle;
Balle balle = {.vitesse_y = 5, .rayon = 5, .couleur = {200, 90, 20 ,255}, .vitesse_x_cible = 5 };

typedef struct {
    int taille;
    int epaisseur;
    int coord_x;
    int coord_y;
    int vitesse;
    Color couleur;
    float proportion_hauteur;
} Raquette;
Raquette raquette = {.taille = 60, .epaisseur = 10, .vitesse = 10, .couleur = WHITE, .proportion_hauteur = 0.83};

typedef struct {
    int coord_x;
    int coord_y;
    Color couleur;
    bool intacte;
} Brique;
Brique briques[MAX_RANGEES][MAX_COLONNES];

typedef enum{
    FACE_HAUT,
    FACE_BAS,
    FACE_GAUCHE,
    FACE_DROITE
} Face;

bool detection_rebond_raquette(){
    if(raquette.coord_y <= balle.coord_y + balle.rayon &&
        balle.coord_y + balle.rayon <= raquette.coord_y + raquette.epaisseur &&
        raquette.coord_x <= balle.coord_x + balle.rayon &&
        balle.coord_x - balle.rayon <= raquette.coord_x + raquette.taille && balle.vitesse_y > 0){
        return true;
    }
    return false;
}

bool detection_rebond_brique(Brique b){
    if(balle.coord_x+balle.rayon >= b.coord_x &&
        balle.coord_x - balle.rayon <= b.coord_x+largeur_brique &&
        balle.coord_y + balle.rayon >= b.coord_y &&
        balle.coord_y - balle.rayon <= b.coord_y + epaisseur_brique){
        return true;
    }
    return false;
}

Face face_touchee(Brique b){
    if(balle.coord_y < b.coord_y){
        return FACE_HAUT;
    }
    else if(balle.coord_y > b.coord_y + epaisseur_brique) {
        return FACE_BAS;
    }
    else{ //si ce n'est pas la haut ou bas
        if(b.coord_x + largeur_brique/2 > balle.coord_x){
            return FACE_GAUCHE;
        }
        else{
            return FACE_DROITE;
        }
    }
}
void initialiser_briques(){
    int largeur_mur = nb_colonnes * largeur_brique + ecart * (nb_colonnes -1);
    int decalage = (taille_fenetre_x - largeur_mur) /2;
    Color couleurs[] = {RED, ORANGE, YELLOW, GREEN, SKYBLUE};
    for (int r = 0; r<nb_rangees; r++){
        for (int c = 0; c<nb_colonnes; c++){
            briques[r][c].couleur = couleurs[r];
            briques[r][c].intacte = true;
            briques[r][c].coord_x = decalage + c * (largeur_brique + ecart);
            briques[r][c].coord_y = marge_haut + r * (epaisseur_brique + ecart);
        }
    }
    briques_restantes = nb_rangees*nb_colonnes;
}

void reinitialiser_partie(){
    initialiser_briques();
    vies = 3;
    score = 0;
    balle.coord_x = (taille_fenetre_x/2);
    balle.coord_y = (taille_fenetre_y/2);
    balle.vitesse_y = 5;
    balle.vitesse_x = 0;
    raquette.coord_x = (taille_fenetre_x/2)-(raquette.taille/2); //initialisation des coordonnées de la raquette
    raquette.coord_y = (taille_fenetre_y * raquette.proportion_hauteur)-(raquette.epaisseur/2);
}
void dessiner_briques(){
    for (int r = 0; r<nb_rangees; r++){
        for (int c = 0; c<nb_colonnes; c++){
            if (briques[r][c].intacte){
                DrawRectangle(briques[r][c].coord_x, briques[r][c].coord_y, largeur_brique, epaisseur_brique, briques[r][c].couleur);
                DrawRectangleLines(briques[r][c].coord_x, briques[r][c].coord_y, largeur_brique, epaisseur_brique, ColorBrightness(briques[r][c].couleur, -0.4f));
            }
        }
    }
}
int main()
{
    InitWindow(taille_fenetre_x, taille_fenetre_y, "Casse-Brique");
    SetTargetFPS(60);
    while (!WindowShouldClose()){
        if((etat == ACCUEIL || etat == PERDU || etat == GAGNE) && IsKeyPressed(KEY_SPACE)){ //lancement de la partie
            reinitialiser_partie();
            etat = EN_COURS;
        }

        // Déplacement de la raquette
        if(etat == EN_COURS){
            if (IsKeyDown(KEY_LEFT)){ //détection appuis fleche gauche
                raquette.coord_x = raquette.coord_x - raquette.vitesse;
            }
            if (IsKeyDown(KEY_RIGHT)){ //détection appuis fleche droite
                raquette.coord_x = raquette.coord_x + raquette.vitesse;
            }
            if (raquette.coord_x < 0){ //detection si la raquette sort du bord gauche
                raquette.coord_x = 0;
            }
            if (raquette.coord_x+raquette.taille > taille_fenetre_x){ //detection si la raquette sort du bord droit
                raquette.coord_x = taille_fenetre_x-raquette.taille;
            }
        }
        int centre_raquette_x = raquette.coord_x + raquette.taille/2;
        //déplacement de la balle
        if(etat == EN_COURS){
            balle.coord_x = balle.coord_x + balle.vitesse_x;//calcul de la position x de la balle
            balle.coord_y = balle.coord_y + balle.vitesse_y;//calcul de la position y de la balle
            if(balle.coord_x + balle.rayon >= taille_fenetre_x ){ //si la balle touche le bord droit, inversion de sa vitesse x
                balle.vitesse_x = -balle.vitesse_x;
                balle.coord_x = taille_fenetre_x - balle.rayon;
            }
            else if(balle.coord_x - balle.rayon <= 0){ //si la balle touche le bord gauche, inversion de sa vitesse x
                balle.vitesse_x = -balle.vitesse_x;
                balle.coord_x = 0 + balle.rayon;
            }
            if(balle.coord_y - balle.rayon <= 0){//si la balle touche le bord haut, inversion de sa vitesse y
                balle.vitesse_y = -balle.vitesse_y;
                balle.coord_y = 0 + balle.rayon;
            }
            else if(balle.coord_y + balle.rayon >= taille_fenetre_y){//si la balle touche le bord bas
                balle.coord_x = (taille_fenetre_x/2); //réinitialisation des coordonnées de la balle
                balle.coord_y = (taille_fenetre_y/2);
                vies = vies -1;
                if(vies <= 0){
                    etat = PERDU;
                }
                balle.vitesse_x = 0;
            }
            if (detection_rebond_raquette()){
                balle.vitesse_y = -balle.vitesse_y;
                balle.coord_y = raquette.coord_y - balle.rayon;
                balle.vitesse_x = (balle.coord_x - centre_raquette_x)/(raquette.taille/2.0f)*balle.vitesse_x_cible;
            }
            for(int r = 0; r<nb_rangees; r++){
                for(int c = 0; c<nb_colonnes; c++){
                    if (briques[r][c].intacte && detection_rebond_brique(briques[r][c])){
                        switch(face_touchee(briques[r][c])){
                            case FACE_HAUT:
                                balle.vitesse_y = -fabsf(balle.vitesse_y);
                                break;
                            case FACE_BAS:
                                balle.vitesse_y = fabsf(balle.vitesse_y);
                                break;
                            case FACE_GAUCHE:
                                balle.vitesse_x = -fabsf(balle.vitesse_x);
                                break;
                            case FACE_DROITE:
                                balle.vitesse_x = fabsf(balle.vitesse_x);
                                break;
                        }
                        briques[r][c].intacte = false;
                        briques_restantes--;
                        score++;
                        if(briques_restantes == 0){
                            etat = GAGNE;
                        }
                    }
                }
            }
        }

        BeginDrawing();
        DrawRectangleGradientV(0, 0, taille_fenetre_x, taille_fenetre_y, bleu_nuit, violet);
        if(etat == ACCUEIL){
            int taille_texte_debut = MeasureText(message_debut, taille_police_debut);
            DrawText(message_debut, (taille_fenetre_x/2)-(taille_texte_debut/2), (taille_fenetre_y/2)-(taille_police_debut/2), taille_police_debut, RAYWHITE);
        }
        else if(etat == EN_COURS){
            DrawText(TextFormat("Vies : %d", vies), 1, 1, 14, RED);
            DrawText(TextFormat("Score : %d", score), 1, 20, 14, RED);
            DrawRectangle(raquette.coord_x,raquette.coord_y, raquette.taille,raquette.epaisseur,raquette.couleur);//dessin de la raquette
            dessiner_briques();
            DrawCircle(balle.coord_x, balle.coord_y, balle.rayon, balle.couleur); //dessin de la balle
        }
        else if (etat == PERDU){
            int taille_texte_debut = MeasureText(message_debut, taille_police_debut);
            DrawText(TextFormat("Perdu ! Score : %d", score), (taille_fenetre_x/2)-(taille_texte_debut/2), (taille_fenetre_y/2)-(taille_police_debut/2), taille_police_debut, RAYWHITE);
            DrawText(TextFormat("Pour recommencer, appuyez sur Espace"), (taille_fenetre_x/2)-(taille_texte_debut/2), (taille_fenetre_y/2+14)-(taille_police_debut/2), taille_police_debut, RAYWHITE);
        }
        else if (etat == GAGNE){
            int taille_texte_debut = MeasureText(message_debut, taille_police_debut);
            DrawText(TextFormat("Gagne ! Score : %d", score), (taille_fenetre_x/2)-(taille_texte_debut/2), (taille_fenetre_y/2)-(taille_police_debut/2), taille_police_debut, RAYWHITE);
            DrawText(TextFormat("Pour recommencer, appuyez sur Espace"), (taille_fenetre_x/2)-(taille_texte_debut/2), (taille_fenetre_y/2+14)-(taille_police_debut/2), taille_police_debut, RAYWHITE);
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
