# Arrosage_pelouse

Ceci est un programme pour l'arrosage de la pelouse et des plates-bandes.  On peut utiliser un mode manuel ou un mode automatique pour l'arrosage. 

En mode automatique, on configure l'heure de départ de l'arrosage et les durées (dans ma municipalité, on ne peut arroser la pelouse que le jeudi).
En mode manuel, peut arroser une zone à la fois.
Le mode annulation permet de suspendre totalement l'arrosage, tant qu'il est actif.

Une page d'accueil permet de connaitre le status du système et de changer son mode.  Une autre page permet d'ajuster l'heure et la date du DS3231.

Matériel:
- ESP32
- Valves
- Relais Smain-Smart
- Horloge : RTC DS3231

Code inspiré de celui de Rui Santos (https://randomnerdtutorials.com)

Le système est indépendant de mon WiFi qui ne se rend pas à l'endroit ou est le contrôleur.  J'utilise donc le mode AP (Acces Point) dans le ESP32.

Guy
