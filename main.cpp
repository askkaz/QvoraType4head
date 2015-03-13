//
//  main.cpp
//  Qvora Type 4head Search Challenge
//
//  Created by Adam Kaczmarek on 1/27/15.
//  Copyright (c) 2015 Adam Kaczmarek. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
using namespace std;

//Use a hash entry to store details of each ID
class HashEntry {
private:
    string stringValue;
    bool active;
    char category;
    float weightValue;

public:
    HashEntry(string value,char type,float weight,int age) {
        this->stringValue = value;
        this->category=type;
        this->active=true;
        this->weightValue=weight;
    }
    char getType(){
        return category;
    }
    string getValue() {
        return stringValue;
    }
    bool isActive(){
        return active;
    }
    void deActivate(){
        this->active=false;
    }
    float getWeight(){
        return weightValue;
    }
};

const int TABLE_SIZE = 80021; //Table size is ~double max required size

//Simple hash function for string
int hashString(string word){
    int sum = 0;
    for (int index=0;index<word.length();index++)
        sum = 10*sum + (int)(word[index]);
    return  sum % TABLE_SIZE;
};

//Create a hashmap to store the details of the IDs
class HashMap {
private:
    HashEntry **table;
public:
    HashMap() {
        table = new HashEntry*[TABLE_SIZE];
        for (int i = 0; i < TABLE_SIZE; i++)
            table[i] = NULL;
    }
    HashEntry get(int hash) {
        return *table[hash];
    }
    void remove(string value){
        int hash=hashString(value);
        while (table[hash]!=NULL && table[hash]->getValue() != value)
            hash = (hash + 1) % TABLE_SIZE;
        if(table[hash]!=NULL&&table[hash]->getValue()==value)
            table[hash]->deActivate();
    }
    int put(string value,char type,float weight,int age) {
        int hash = hashString(value);
        while (table[hash] != NULL && table[hash]->getValue() != value)
            hash = (hash + 1) % TABLE_SIZE;
        if (table[hash] != NULL)
            delete table[hash];
        table[hash] = new HashEntry(value,type,weight,age);
        return hash; //return hash to be used as an index to find this ID quickly later
    }
    ~HashMap() {
        for (int i = 0; i < TABLE_SIZE; i++)
            if (table[i] != NULL)
                delete table[i];
        delete[] table;
    }
};

//Structure to store each ID.  Include Age comparator.
struct idEntry{
    idEntry(int _value, int _age){
        this->age=_age;
        this->value=_value;
    }
    idEntry(){
        this->age=-1;
        this->value=-1;
    }
    int value;
    int age;
    bool operator<(idEntry const &other) const {
        if (age < other.age)
            return true;
        return false;
    }
    ~idEntry(){}
};

//Node for the word dictionary
//Each Node has a child (next letter in word) and sibling (alternate letter in same position of word)
//Storing the id_list at each node was more efficient than storing at Node at end of word
struct Node
{
    Node(){
        this->char_val=-1;
        this->child=NULL;
        this->sibling=NULL;
    }
    Node(int data){
        this->char_val=data;
        this->child=NULL;
        this->sibling=NULL;
    }
    int char_val;
    Node *child;
    Node *sibling;
    vector<idEntry> id_list;
    ~Node(){}
};

//Function to navigate the dictionary and add Nodes as appropriate
void Add(Node *root, string wordIn,int idHash,int age){
    Node* curNode=root;
    int curVal=0;

    for(int index=0;index<wordIn.length();index++){
        curVal=wordIn[index];
        if(curNode->char_val==curVal){
            if(curNode->child==NULL){
                curNode->child=new Node;
            }
        }
        else if(curNode->char_val<0){
            curNode->char_val=curVal;
            curNode->child=new Node;
        }
        else{
            //keep moving until the sibling is null or what you want
            if(curNode->sibling==NULL){
                curNode->sibling=new Node;
            }
            while(curNode->sibling->char_val!=curVal&&curNode->sibling->char_val>0){
                curNode=curNode->sibling;
                if(curNode->sibling==NULL){
                    curNode->sibling=new Node;
                }
            }
            curNode=curNode->sibling;
            if(curNode->char_val<0)
            {
                curNode->char_val=curVal;
                //Already know the rest of word is not there, so add rest of word
                curNode->id_list.push_back(idEntry(idHash,age));
                while(index<wordIn.length()){
                    index++;
                    curNode->child=new Node(wordIn[index]);
                    curNode=curNode->child;
                    curNode->id_list.push_back(idEntry(idHash,age));
                }
                index--;


            }
        }
        curNode->id_list.push_back(idEntry(idHash,age));
        if(index!=wordIn.length()-1){
            curNode=curNode->child;
        }

    }

}

//Function to find intersection of two id lists
vector<idEntry> setIntersection(vector<idEntry> setOne,vector<idEntry> setTwo){
    vector<idEntry> setThree;
    vector<idEntry>::iterator it1;
    vector<idEntry>::iterator it2;
    it1=setOne.begin();
    it2=setTwo.begin();
    while (it1!=setOne.end() && it2!= setTwo.end()) {
        if (*it2<  *it1) {
            it2++;
        } else if (*it1<  *it2) {
            it1++;
        } else {
            setThree.push_back(*it1);
            it1++;
            it2++;
        }
    }
    return setThree;
}

//Function to traverse dictionary and return the appropriate id list
vector<idEntry> findValidIds(Node *root, string wordIn){
    vector<idEntry> id_list;
    int curVal=0;
    Node* curNode=root;
    for(int index=0;index<wordIn.length()-1;index++){
        curVal=wordIn[index];
        while(curNode->char_val!=curVal&&curNode->sibling!=NULL ){
            curNode=curNode->sibling;
        }
        if(curNode->char_val!=curVal){
            return id_list;
        }
        curNode=curNode->child;
    }
    curVal=wordIn[wordIn.length()-1];
    while(curNode->char_val!=curVal&&curNode->sibling!=NULL){
        curNode=curNode->sibling;
    }
    if(curNode->char_val!=curVal){

        return id_list;
    }
    return curNode->id_list;

}

//Function to find all common ids
vector<idEntry> findPhraseIds(Node *dictionary,string phraseIn){
    vector<idEntry> resulting_ids;
    string currentWord;
    istringstream iss(phraseIn);
    iss>>currentWord;
    resulting_ids=findValidIds(dictionary, currentWord);
    while (iss>>currentWord){
        if(resulting_ids.empty()){
            return resulting_ids;
        }
        resulting_ids=setIntersection(resulting_ids, findValidIds(dictionary, currentWord));
    }
    return resulting_ids;
}

//Function to add phrase, word by word, to the dictionary
void addPhrase(string phrase,char type,string id, float weight, Node *root,HashMap *idList,int age){
    string currentSubString;
    int idhash=idList->put(id,type,weight,age);
    istringstream iss(phrase);
    while (iss>>currentSubString){
        Add(root,currentSubString,idhash,age);
    }
}

//Structure to organize results and allow for sorting
struct results{
    string resultText;
    float resultScore;
    int resultAge;
    bool operator<(results const &other) const {
        if (resultScore > other.resultScore)
            return true;
        if (resultScore < other.resultScore)
            return false;
        if (resultAge > other.resultAge)
            return true;
        return false;
    }
    ~results(){}
};

//Structure to store boost entries
struct boost{
    string boostText;
    float boostVal;
    bool isID;
    ~boost() {}
};

//Function to print out QUERY results
void printQuery(vector<idEntry> ids,HashMap *idList,int numToPrint){
    //iterate through ids....add them to vector with their corresponding data
    vector<results> myResults;
    vector<idEntry>::iterator it;
    results thisResult;
    for (it=ids.begin(); it!=ids.end(); ++it){
        if (idList->get(it->value).isActive()){
            thisResult.resultAge=it->age;
            thisResult.resultScore=idList->get(it->value).getWeight();
            thisResult.resultText=idList->get(it->value).getValue();
            myResults.push_back(thisResult);
        }
    }
    //sort vector
    sort(myResults.begin(), myResults.end());
    //print numToPrint elements of vector
    for (int i=0; i<min((int)myResults.size(),numToPrint);i++){
        if(i>0&&myResults[i].resultText==myResults[i-1].resultText){
            numToPrint++;
        }
        else{
            cout << myResults[i].resultText<<" ";
        }
    }
    cout<<"\n";
}

//Function to print out WQUERY results
void printWQuery(vector<idEntry> ids,HashMap *idList,int numToPrint,vector<boost> boostList){
    //iterate through ids....add them to vector with their corresponding data
    vector<results> myResults;
    vector<idEntry>::iterator it;
    char boostType;
    float thisScore =1.0;
    string thisType;

    results thisResult;
    for (it=ids.begin(); it!=ids.end(); ++it){
        if (idList->get(it->value).isActive()){
            thisResult.resultAge=it->age;
            thisResult.resultText=idList->get(it->value).getValue();
            thisScore=idList->get(it->value).getWeight();
            for(int j=0;j<boostList.size();j++){
                if(boostList[j].isID){
                    if(boostList[j].boostText==thisResult.resultText){
                        thisScore =thisScore*boostList[j].boostVal;
                    }
                }
                else{
                    boostType=tolower(boostList[j].boostText.at(0));
                    if(boostType==idList->get(it->value).getType()){
                        thisScore =thisScore*boostList[j].boostVal;
                    }
                }
            }
            thisResult.resultScore=thisScore;
            myResults.push_back(thisResult);
        }
    }
    //sort vector
    sort(myResults.begin(), myResults.end());
    //print numToPrint elements of vector
    for (int i=0; i<min((int)myResults.size(),numToPrint);i++){
        if(i>0&&myResults[i].resultText==myResults[i-1].resultText){
            numToPrint++;
        }
        else{
            cout << myResults[i].resultText<<" ";
        }
    }
    cout<<"\n";
}
void printVector(vector<int> toPrint){
    vector<int>::iterator it;
    for (it=toPrint.begin(); it!=toPrint.end(); ++it){
        cout<<*it<<" ";
    }
    cout<<"\n";
}

//MAIN
int main(){
    Node* dictionary =new Node;
    HashMap* idList= new HashMap;
    vector<boost> boostList;
    boost thisBoost;
    int numLines;
    int boostNum=0;
    int age=0;
    string lineIn;
    string token;
    string currentSubString;
    string identifier;
    vector<idEntry> resulting_ids;
    string phrase;
    float weight;
    char type;
    bool isId;
    int numResults=0;
    cin>>numLines;
    for(int i=0;i<numLines;i++){
        cin>>lineIn;
        if("ADD"==lineIn){
            cin>>lineIn;//category (needed for boost application)
            type=lineIn.at(0);
            cin>>identifier; //id
            cin>>lineIn; //weight
            weight=::atof(lineIn.c_str());
            getline(cin, phrase); //phrase
            transform(phrase.begin(), phrase.end(), phrase.begin(), ::toupper);
            addPhrase(phrase, type, identifier,weight, dictionary,idList,age);
            age++;
        }
        else if("QUERY"==lineIn){
            cin>>numResults; //number of results
            getline(cin, lineIn);//phrase
            if (numResults==0){
                cout<<"\n";
            }
            else{
                transform(lineIn.begin(), lineIn.end(), lineIn.begin(), ::toupper);
                //tokenize phrase and intersect all results
                resulting_ids=findPhraseIds(dictionary,lineIn);
                printQuery(resulting_ids,idList,numResults);
            }
        }
        else if("WQUERY"==lineIn){
            boostList.clear();
            cin>>numResults; //number of results
            cin>>boostNum;
            for(int j=0;j<boostNum;j++){
                isId=true;
                cin>>lineIn; //readboosts
                thisBoost.boostText=lineIn.substr(0,lineIn.find(":"));
                if(thisBoost.boostText=="topic"||thisBoost.boostText=="user"||thisBoost.boostText=="board"||thisBoost.boostText=="question")
                    isId=false;
                thisBoost.boostVal=::atof(lineIn.substr(lineIn.find(":")+1,lineIn.size()).c_str());
                thisBoost.isID=isId;
                boostList.push_back(thisBoost);
            }
            getline(cin, lineIn);//phrase
            if (numResults==0){
                cout<<"\n";
            }
            else{
                transform(lineIn.begin(), lineIn.end(), lineIn.begin(), ::toupper);
                resulting_ids=findPhraseIds(dictionary,lineIn);
                printWQuery(resulting_ids,idList,numResults,boostList);
            }
        }
        else if("DEL"==lineIn){  //DELETE
            cin>>lineIn;
            idList->remove(lineIn);
        }
    }
    return 0;
}




