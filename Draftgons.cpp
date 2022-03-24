
//Draftgons
//by Pascal Stephens
//Pokemon stripped down to its bare essentials
//two player game
//made entirely from scratch

#include <iostream> //for cout
#include <string> //for strings
#include <sstream> //for conversion
#include <stdlib.h> //for randomness
#include <time.h> //for seeding
using namespace std;
const int BASE_DAMAGE=5;
double matchup[6][6]={{1  ,1  ,1  ,1  ,1  ,1  }, //type matchups
                      {1  ,1  ,2  ,2  ,1  ,0.5}, //[attacking type][defending type]
                      {2  ,0.5,2  ,1  ,1  ,0.5},
                      {0.5,1  ,1  ,1  ,2  ,0.5},
                      {1  ,2  ,0.5,1  ,1  ,2  },
                      {1  ,1  ,1  ,1  ,0.5,0.5}};

int secondaryType[6]={5,3,0,2,1,4}; //dictates what coverage options each type has
string typeTranslation[]={"Flesh","Fire","Radiated","Electric","Water","Metal"};
//global game controllers
string names[2]; //holds player1's name and player2's name
int activePlayer=0; //player1 starts
int inactivePlayer=1; //player2 second
int currentDragon[2]; //keeps track of which dragon is active
bool availableDragons[2][3]={{1,1,1},{1,1,1}}; //keeps track of which dragons can be switched to
bool canSwitch=true; //for Sprite's technique
int foresight[2]={0,0}; //for predictions
class Dragon{
    public:
    string name;
    int type;
    double health;
    bool isDead=false;
    void (*move1) (Dragon&, Dragon&); //function pointers for easy mixing of moves
    void (*move2) (Dragon&, Dragon&);
    void (*move3) (Dragon&, Dragon&);
    void (*EOT) (Dragon&);
    void (*onSwitchIn) (Dragon&);
    string moveDisplay; //shows the player what moves are available
    void takeDamage(int amount, int aType){ //best way to modify health
        health=health-amount*matchup[aType][type]; //mods health
        cout<<name<<" took "<<amount*matchup[aType][type]<<" damage"; //tells the player the difference
        if(matchup[aType][type]==0.5){
            cout<<"."<<endl;
        } else if(matchup[aType][type]==2){
            cout<<"!!!"<<endl;
        } else {
            cout<<"!"<<endl;
        } //inspired by Golden Sun's "super effective" markers
        cout<<name<<"'s health is "<<health<<endl;
        if(health<=0){
            health=0;
            isDead=true;
            cout<<name<<" died!"<<endl; //for switching purposes
        }
        cout<<endl;
    }
    Dragon(); //constructor
    Dragon(string newName, int newType, int newHealth, void (*m1)(Dragon&, Dragon&), void (*m2)(Dragon&, Dragon&), void (*m3)(Dragon&,Dragon&),
            string newMoveDisplay, void (*newEOT)(Dragon&), void (*newOnSwitchIn)(Dragon&)){ //also constructor
        name=newName;
        type=newType;
        health=newHealth;
        move1=m1;
        move2=m2;
        move3=m3;
        moveDisplay=newMoveDisplay;
        EOT=newEOT;
        onSwitchIn=newOnSwitchIn; //assign everything
    }
    void displayDragon(bool a1, bool a2, bool a3, bool a4){ //for displaying
        cout<<"[";
        if(a1) //modular
            printf("%-10s ",name.c_str());
        if(a2)
            printf(" |%3.1f health",health);
        if(a3)
            printf(" |%11s type", typeTranslation[type].c_str());
        cout<<"]";
        if(a4)
            cout<<endl;
    }
};
//the rest of the global game controlling variables
Dragon playerDragons[2][3]; //holds all active dragons, [player][dragon]
Dragon draft[7]; //for the draft portion

void DUMMY(){ //just a dummy function
    return;
}
void DUMMY(Dragon& a){ //overloaded to be put in any EOT or switchIn function
    return;
}
void DUMMY(Dragon& a, Dragon& b){ //overloaded for moves
    cout<<"But nothing happened!"<<endl; //splash reference :D
    return;
}
Dragon::Dragon(){ //so nothing bad happens when an initiated Dragon is referenced
        name="stock";
        type=1;
        health=100;
        moveDisplay="1. nothing\n2. nothing\n3. nothing";
        move1=DUMMY;
        move2=DUMMY;
        move3=DUMMY;
}

//game events
//meant to be called by EOT and onSwitchIn
bool forceSwitch(int); //forward declaration
void take5(Dragon& d){ 
    cout<<d.name<<" is afflicted! ";
    d.takeDamage(5,0);
    
}
void heal5(Dragon& d){
    d.health=d.health+5;
    cout<<d.name<<" healed 5 health!"<<endl;
}
void forceOpponentSwitch(Dragon& d){ //meant to be called only from EOT
    d.EOT=DUMMY; //reset
    cout<<d.name<<" forces you to switch! ";
    forceSwitch(inactivePlayer);
}
void oneTurnSwitch(Dragon& d){
    cout<<d.name<<" will force you to switch in one turn."<<endl;
    d.EOT=forceOpponentSwitch;
}
void delayedDamage(Dragon& d){
    cout<<"The delayed attack hit! ";
    playerDragons[inactivePlayer][currentDragon[inactivePlayer]].takeDamage(BASE_DAMAGE*5,d.type);
    d.EOT=DUMMY;
}
void oneTurnDamage(Dragon& d){
    cout<<"The delayed attack will hit in one turn."<<endl;
    d.EOT=delayedDamage;
}
void oneForesight(Dragon& d){
    cout<<d.name<<" slows..."<<endl;
    foresight[inactivePlayer]++;
}
void attrition(Dragon& d){
    cout<<"Pestilence affects all!"<<endl;
    for(int i=0;i<2;i++){
        for(int j=0;j<3;j++){ //each dragon
            if(playerDragons[i][j].isDead==0){ //if(alive)
                playerDragons[i][j].takeDamage(1,0);
            }
        }
    }
}

//meant to be called from only move1, move2, move3
//game moves

void basicDamagingMove(Dragon& d, int type, int damage){
    d.takeDamage(damage,type);
}
void baseTypeStrong(Dragon& d1, Dragon& d2){
    basicDamagingMove(d2,d1.type,BASE_DAMAGE*4);
}
void secondaryTypeStrong(Dragon& d1, Dragon& d2){
    basicDamagingMove(d2,secondaryType[d1.type],BASE_DAMAGE*4);
}
void baseTypeMid(Dragon& d1, Dragon& d2){
    basicDamagingMove(d2,d1.type,BASE_DAMAGE*2);
}
void secondaryTypeMid(Dragon& d1, Dragon& d2){
    basicDamagingMove(d2,secondaryType[d1.type],BASE_DAMAGE);
}
void baseTypeWeak(Dragon& d1, Dragon& d2){
    basicDamagingMove(d2,d1.type,BASE_DAMAGE*2);
}
void secondaryTypeWeak(Dragon& d1, Dragon& d2){
    basicDamagingMove(d2,secondaryType[d1.type],BASE_DAMAGE);
}
void entryHazard(Dragon& d1, Dragon& d2){
    cout<<"All dragons now take 5 damage when switching in."<<endl;
    for(int i=0;i<2;i++){
        for(int j=0;j<3;j++){ //each dragon
            playerDragons[i][j].onSwitchIn=take5;
        }
    }
}
void afflict(Dragon& d1, Dragon& d2){
    cout<<d2.name<<" became Afflicted! They will now take 5 damage at the end of their turns."<<endl;
    d2.EOT=take5;
}
void enhanceTypeDamage(Dragon& d1, Dragon& d2){
        for(int i=0;i<6;i++){
        matchup[3][i]=matchup[3][i]*1.2;
    }
    cout<<d1.name<<" boosted "<<typeTranslation[3]<<" type damage permanently! Base multiplier is "<<matchup[3][3]<<"."<<endl;
}
void heal(Dragon& d1, Dragon& d2){
    heal5(d1);
}
void switchBlock(Dragon& d1, Dragon& d2){
    if(canSwitch){
        cout<<"Dragons may no longer switch out manually."<<endl;
        canSwitch=false;
    } else {
        cout<<"Dragons may switch out manually."<<endl;
        canSwitch=true;
    }
}
void twoTurnSwitch(Dragon & d1, Dragon& d2){
    cout<<names[activePlayer]<<" will be forced to switch in two turns."<<endl;
    d2.EOT=oneTurnSwitch;
}
void twoTurnDamage(Dragon& d1, Dragon& d2){
    cout<<names[inactivePlayer]<<" will be hit in two turns."<<endl;
    d1.EOT=oneTurnDamage;
}
void giveForesight(Dragon& d1, Dragon& d2){
    cout<<names[inactivePlayer]<<" has been predicted for the next 3 turns."<<endl;
    foresight[activePlayer]=3;
}
void giveAttrition(Dragon& d1, Dragon& d2){
    d2.takeDamage(1,0);
    cout<<"The pestilence spreads! "<<d2.name<<" has been infected."<<endl;
    d2.EOT=attrition;
}

//dragons

Dragon mr_kill_you("Mr. kill you",1,30,baseTypeStrong,DUMMY,DUMMY,"1. shoot\n2. nothing\n3. nothing",DUMMY,DUMMY);

Dragon karosk("Karosk",4,50,baseTypeWeak,entryHazard,heal,"1. Attack\n2. Entry hazard\n3. Heal",DUMMY,DUMMY);

Dragon sprite("Sprite",5,20,secondaryTypeWeak,switchBlock,afflict,"1. Attack\n2. No more switching\n3. Affliction",DUMMY,DUMMY);

Dragon dynamo("Dynamo",3,25,enhanceTypeDamage,secondaryTypeMid,DUMMY,"1. Enhance Type 3's damage permanently\n2. Attack\n3. Nothing",DUMMY,DUMMY);

Dragon daiad1("Daiad",2,25,baseTypeMid,secondaryTypeStrong,DUMMY,"1. Primary attack\n2. Secondary attack\n3. Nothing",DUMMY,DUMMY);

Dragon daiad2("Daiad",2,25,baseTypeMid,secondaryTypeStrong,DUMMY,"1. Primary attack\n2. Secondary attack\n3. Nothing",DUMMY,DUMMY);

Dragon dule("Dule",1,35,twoTurnDamage,twoTurnSwitch,baseTypeMid,"1. Delayed damage\n2. Delayed switch\n3. Attack",DUMMY,DUMMY);

Dragon mirage("Mirage",0,20,giveForesight,baseTypeMid,DUMMY,"1. For the next 3 times an opponent attacks, you switch out after the choice\n2. Primary attack\n3. Nothing",DUMMY,DUMMY);

Dragon taunt("Taunt",3,35,baseTypeStrong,secondaryTypeStrong,DUMMY,"1. Forseen attack\n2. Forseen attack\n3. Nothing",oneForesight,DUMMY);

Dragon pestilence("Pestilence",0,45,baseTypeMid,afflict,giveAttrition,"1. Flesh attack\n2. Affliction\n3. Spread pestilence",attrition,attrition);

Dragon silo[]={karosk,sprite,dynamo,daiad1,daiad2,dule,mirage,taunt,pestilence}; //length is calculated automatically, later

//game functions

int getIntInput(int min, int max){ //for input purposes
    string input="";
    while(true){
        cin>>input;
        stringstream converter;
        converter<<input;
        int output;
        converter>>output;
        if(output>=min&&output<=max){
            return output;
        } else {
            cout<<"\nPlease give an input listed.\n";
        }
    }
}
bool forceSwitch(int player){
    int total=0;
    for (int i=0;i<3;i++){
        total+=availableDragons[player][i]; //count all available dragons
    }
    if(total==0){ //if there are none
        cout<<"no available switches"<<endl;
        return 0; //return failure
    }
    if(total==1){ //if there is a single one
        for (int i=0;i<3;i++){ //find it
            if(availableDragons[player][i]==1){ //find it
                if(!playerDragons[player][currentDragon[player]].isDead){ //if the switch was voluntary
                    availableDragons[player][currentDragon[player]]=1; //then recognize as such
                }
                currentDragon[player]=i; //make the switch
                availableDragons[player][i]=0; //make it unable to be switched to
                cout<<"Switching to "<<playerDragons[player][i].name<<endl;
                playerDragons[player][currentDragon[player]].onSwitchIn(playerDragons[player][currentDragon[player]]); //call appropriate onSwitchIn function
                return 1; //stop searching, return success
            }
        }
        
    }
    if(total==2){ //if there are two options
        int holder[2]; //make the holder
        int counter=0;
        for (int i=0;i<3;i++){ //find the dragons available
            if(availableDragons[player][i]==1){
                holder[counter]=i;
                counter+=1; //and add it
            }
        }
        cout<<names[player]<<", who will you switch to?"<<endl;
        cout<<"1. "<<playerDragons[player][holder[0]].name<<endl;
        cout<<"2. "<<playerDragons[player][holder[1]].name<<endl;
        int input=getIntInput(1,2); //choice
        if(!playerDragons[player][currentDragon[player]].isDead){ //if the switch was voluntary
            availableDragons[player][currentDragon[player]]=1; //record as such
        }
        currentDragon[player]=holder[input-1]; //make the switch
        availableDragons[player][input]=0; //make the dragon unavailable to switch to
        cout<<"switching to "<<playerDragons[player][input].name<<endl;
        playerDragons[player][currentDragon[player]].onSwitchIn(playerDragons[player][currentDragon[player]]); //trigger appropriate onSwitchIn function
        return 1; //return a success
    }
    return 0;
}
void updateAvailableDragons(){ //for EOT shizzle
    for(int j=0;j<2;j++){
        for(int i=0;i<3;i++){ //for each dragon
            if(playerDragons[j][i].isDead){ //if its dead
                availableDragons[j][i]=0; //its unavailable
                if(currentDragon[inactivePlayer]==i&&inactivePlayer==j){ //if its the  player in question's dragon
                    forceSwitch(inactivePlayer);
                }
                if(currentDragon[activePlayer]==i&&activePlayer==j){ //if its the  player in question's dragon
                    forceSwitch(activePlayer);
                }
            } else {
                availableDragons[j][i]=1; //otherwise it is available
            }
        }
    }
    availableDragons[activePlayer][currentDragon[activePlayer]]=0; //the active dragos are unavalable
    availableDragons[inactivePlayer][currentDragon[inactivePlayer]]=0;
}
void toggleActive(){ //switch active and inactive players
    if(activePlayer){
        activePlayer=0;
    } else {
        activePlayer=1;
    }
    if(inactivePlayer){
        inactivePlayer=0;
    } else {
        inactivePlayer=1;
    }
}
bool gameStillPlayable(){ //for completions sake
    int total1=0;
    for (int j=0;j<3;j++){
        total1+=playerDragons[0][j].isDead; //total p1's dead dragons
    }
    if(total1==3){ //if they're all dead
        return false; //then no it ain't playable
    }
    int total2=0;
    for (int j=0;j<3;j++){ //same
        total2+=playerDragons[1][j].isDead;
    }
    if(total2==3){
        return false;
    }
    return true; //if both pass, it's still goin
}
int main(){
    srand(time(NULL)); //seed random function
    currentDragon[0]=0;
    currentDragon[1]=0;
    int draftFillLevel=0;
    int siloSize=*(&silo+1)-silo; //for calculating how many elements there are since cpp doesn't have array.size()
    while(draftFillLevel<7){ //while draft aint full
        int rando = rand()%siloSize; //pick an element of silo
        Dragon temp=silo[rando]; //store it
        if(temp.name!="taken"){ //if it isn't marked as taken
            draft[draftFillLevel]=temp; //add it
            draftFillLevel++; //draft is fuller
            silo[rando].name="taken"; //and mark it taken
        }
    }
    cout<<"welcome to draftgons\ndraft away"<<endl;
    cout<<"p1 name: ";
    cin>>names[0];
    cout<<endl<<"p2 name: ";
    cin>>names[1];
    cout<<endl; //get names
    //draft
    int j=7; //counter
    while (j>1){ //while the draft ain't done
        cout << "\033[2J\033[1;1H"; //clear the screen
        toggleActive(); //switch players
        cout<<names[activePlayer]<<" pick a dragon:"<<endl;
        for (int i=0;i<j;i++){ //display the available picks
            cout<<i+1<<". ";
            draft[i].displayDragon(1,1,1,1);
            cout<<draft[i].moveDisplay<<endl<<endl;
        }
        int input=getIntInput(1,j); //input
        playerDragons[activePlayer][currentDragon[activePlayer]]=draft[input-1]; //store it
        currentDragon[activePlayer]=currentDragon[activePlayer]+1; //next dragon's spot
        for(int i=input-1;i<j-1;i++){ //for each element above it
            draft[i]=draft[i+1]; //shift it down one
        }
        j--; //decrement counter
    }
    currentDragon[0]=0; //reset current dragons
    currentDragon[1]=0;
    cout<<names[0]<<" 's dragons are "<<playerDragons[0][0].name<<" "<<playerDragons[0][1].name<<" "<<playerDragons[0][2].name<<" \n";
    cout<<names[1]<<" 's dragons are "<<playerDragons[1][0].name<<" "<<playerDragons[1][1].name<<" "<<playerDragons[1][2].name<<" \n";
    //play
//--------------------------------------------------------------------------------------------------------------------------------------------------//
    //play
    updateAvailableDragons(); //init dragon availability
    while (gameStillPlayable()){
        toggleActive(); //swap players
        cout<<names[activePlayer]<<"'s turn"<<endl<<endl; //display
        cout<<"Your Dragons:"<<endl<<"|[ ";
        playerDragons[activePlayer][currentDragon[activePlayer]].displayDragon(1,1,1,0);
        cout<<" ]|"<<endl<<"   ";
        playerDragons[activePlayer][(currentDragon[activePlayer]+1)%3].displayDragon(1,1,1,1);
        cout<<"   ";
        playerDragons[activePlayer][(currentDragon[activePlayer]+2)%3].displayDragon(1,1,1,1);
        cout<<endl<<"Their Dragons:"<<endl<<"|[ ";
        playerDragons[inactivePlayer][currentDragon[inactivePlayer]].displayDragon(1,1,1,0);
        cout<<" ]|"<<endl<<"   ";
        playerDragons[inactivePlayer][(currentDragon[inactivePlayer]+1)%3].displayDragon(1,1,1,1);
        cout<<"   ";
        playerDragons[inactivePlayer][(currentDragon[inactivePlayer]+2)%3].displayDragon(1,1,1,1);
        cout<<"What will you do?"<<endl;
        cout<<playerDragons[activePlayer][currentDragon[activePlayer]].moveDisplay<<endl;
        cout<<"4. Switch"<<endl;
        int input=getIntInput(0,5); //input
        cout << "\033[2J\033[1;1H"; //clear the screen
        cout<<"|[--------------------------------------------------------------------]|"<<endl;
        if(input==1){ //move 1
            if(foresight[inactivePlayer]){
                foresight[inactivePlayer]=foresight[inactivePlayer]-1;
                cout<<"Forseen! ";
                forceSwitch(inactivePlayer);
            }
            playerDragons[activePlayer][currentDragon[activePlayer]].move1(playerDragons[activePlayer][currentDragon[activePlayer]],playerDragons[inactivePlayer][currentDragon[inactivePlayer]]);
        }
        if(input==2){ //move 2
            if(foresight[inactivePlayer]){
                foresight[inactivePlayer]=foresight[inactivePlayer]-1;
                cout<<"Forseen! ";
                forceSwitch(inactivePlayer);
            }
            playerDragons[activePlayer][currentDragon[activePlayer]].move2(playerDragons[activePlayer][currentDragon[activePlayer]],playerDragons[inactivePlayer][currentDragon[inactivePlayer]]);
        }
        if(input==3){ //move 3
            if(foresight[inactivePlayer]){
                foresight[inactivePlayer]=foresight[inactivePlayer]-1;
                cout<<"Forseen! ";
                forceSwitch(inactivePlayer);
            }
            playerDragons[activePlayer][currentDragon[activePlayer]].move3(playerDragons[activePlayer][currentDragon[activePlayer]],playerDragons[inactivePlayer][currentDragon[inactivePlayer]]);
        }
        if(input==4){ //switch
            if(canSwitch){
                if(forceSwitch(activePlayer)==0){ //try to switch and record if it failed or not. if it did fail,
                    cout<<"No available switches."<<endl;
                    toggleActive(); //retry turn
                    continue; //skip EOT and updates cuz nothing happened
                }
            } else {
                cout<<"Switching isn't allowed!"<<endl;
                toggleActive(); //nothing happened, skip too
                continue;
            }
        }
        if(input==5){ //force quit
            break;
        }
        if(input==0){ //info
            cout<<"| Matchups |";
            for(int i=0;i<6;i++){
                printf("| %8s |",typeTranslation[i].c_str());
            }
            cout<<endl;
            for(int j=0;j<6;j++){
                printf("| %8s |",typeTranslation[j].c_str());
                for(int i=0;i<6;i++){
                    printf("| %8.3f |",matchup[j][i]);
                }
                cout<<endl;
            }
            cout<<"Terms:\nForesight = defending player must switch before damage is dealt."<<endl<<endl;
            for(int i=0;i<6;i++){
                cout<<typeTranslation[i]<<" can use "<<typeTranslation[secondaryType[i]]<<" moves"<<endl;
            }
            toggleActive();
            continue;
        }
        
        playerDragons[activePlayer][currentDragon[activePlayer]].EOT(playerDragons[activePlayer][currentDragon[activePlayer]]); //end of turn stuff
        updateAvailableDragons(); //update
        cout<<"|[--------------------------------------------------------------------]|"<<endl; //p r e t t y   s t u f f 
    }
    cout << "\033[2J\033[1;1H"; //clear the screen
    if(availableDragons[activePlayer][0]==0&&availableDragons[activePlayer][2]==0&&availableDragons[activePlayer][2]==0){ //check for active's dragons first
        cout<<names[inactivePlayer]<<" won!!! congratulations"<<endl; //if theyre all dead, inactive wins
    } else { //check active first, they're the one who would be able to forfeit
        cout<<names[activePlayer]<<" won!!! congratulations"<<endl;
    }
    return 0; //the end!!!
}
