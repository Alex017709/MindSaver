# MindSaver

MindSaver is a desktop web application for daily monitoring of your psycho-emotional state. 
The main functionality is to keep a daily personal diary to record your (mood, energy, stress level) 
and rate them on a ten-point scale.

## USERS

* User Entity
This entity describes a registered user of the system.
Attributes:
• User ID (key attribute)
• First Name
• Last Name
• Date of Birth
• Registration Date
“State Record” Entity
* The entity contains information about the daily psychological state of the user.
Attributes:
• Record ID (key attribute)
• Mood (1–10)
• Stress (1–10)
• Energy Level (1–10)
• Note
• Hashtag
• Creation Date

##  TECHNOLOGY

* *beckend:* C++
* *frontend:* htlm/css/javascript
* *database:* save file on .bin

## ARCHITECTURE
```text
├───.vscode  
├───database/           #database
│   └───Data.bin
│   └───idb.bin
│   └───ids.bin
│   └───users.bin
├───include/            #required libraries for the server
│   └───asio/..
│   └───Crow.h
├───src/                #beckend
│   ├───data/           #data record entity
│   │   └───full.h
│   │   └───input_data.cpp
│   │   └───input_data.h
│   │   └───stat.h
│   ├───date/
│   │   └───date.h
│   │   └───date.cpp
│   └───user/           #user entity 
│   │   └───user_data.h
│   │   └───user_data.cpp
│   │───main.cpp        #main file
├───static/             #frontend
│   ├───css/            #css style
│   │   └───style.css
│   ├───image/..        #static image
│   └───js/             #javasckript logik
│   │   └───main.js
├───templates/..        #html page
└───main.exe            #exe file
```

## STRUCT

* input_data

```text 
class input_data 
private:
    int ID;
    int hetch;
    short nastry; 
    short energy;
    short stress;
    char note[300];
```
* users

```text
class user 
private:
    int ID;
    char name[250];
    date birhtday;
```
## GIT CLONE

```bash
git clone https://github.com/Alex017709/MindSaver.git
```

## COMPILATOR

```bash
g++ src/*.cpp src/data/*.cpp src/user/*.cpp src/date/*.cpp -Iinclude -Isrc -std=c++17 -o main.exe -lws2_32 -lmswsock
```


