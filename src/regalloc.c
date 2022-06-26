//register allocator

#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include "list.h"
#include "stack.h"

typedef struct node {
    int number;
    int color;
    List logicalInterference;
    List physicalInterference;
}node;

typedef struct interferenceGraph {
    int id;
    int k;
    List nodes; 
}interferenceGraph;

interferenceGraph* createInterferenceGraph(){
    
    interferenceGraph* graph = (interferenceGraph*) malloc(sizeof(interferenceGraph));
    graph->nodes = newList();

    char string[100000];
    fgets(string, 100000, stdin); //reading graph number...
    sscanf(string, "Grafo %d:", &(graph->id));

    fgets(string, 100000, stdin); //reading K...
    sscanf(string, "K=%d", &(graph->k));


    char* substring;
    while (fgets(string, 100000, stdin) != NULL){ //reading graph nodes...

        node* newNode = (node*) malloc(sizeof(node));
        newNode->logicalInterference = newList();
        newNode->physicalInterference = newList();

        substring = strtok(string, " "); //reading node number...
        sscanf(substring, "%d", &(newNode->number));
        newNode->color = -1;

        substring = strtok(NULL, " "); //reading "-->"...
        
        substring = strtok(NULL, " "); //reading interferences...
        while(substring != NULL) {

            //process interference...
            int* interference = (int*) malloc(sizeof(int)); 
            *interference = atoi(substring);

            if((*interference) >= graph->k) //logical interference
                listAdd(newNode->logicalInterference, interference);
            else //physical interference
                listAdd(newNode->physicalInterference, interference);

            substring = strtok(NULL, " ");
        }   
        
       listAdd(graph->nodes, newNode);
    }

    return graph;
}

char* intToString(Data data){
    int* integer = (int*) data;
    char* string = (char*) malloc(10*sizeof(char));
    sprintf(string, "%d", *integer);
    return string;
}

void freeNode(Data NodeToFree){
    node* nodeToFree = (node*) NodeToFree;
    freeList(nodeToFree->logicalInterference, free);
    freeList(nodeToFree->physicalInterference, free);
    free(nodeToFree);
}

void freeInterferenceGraph(interferenceGraph* graph){
    freeList(graph->nodes, freeNode);
    free(graph);
}


void printGraph(interferenceGraph* graph){
    int size = listSize(graph->nodes);
    for(int i = 0; i<size; i++){
        node* nod = (node*) listGet(graph->nodes, i);
        printf("\n\nnode: %d\n", nod->number);
        printf("phy inter:\n");
        printList(nod->physicalInterference, intToString);
        printf("log inter:\n");
        printList(nod->logicalInterference, intToString);
    }
}


node* removeNode(interferenceGraph* graph, int currentK, int* spill){
       
    int nodesListSize = listSize(graph->nodes);
    if(nodesListSize <= 0) return NULL;

    //find node.
    int smallestNodeDegree = 1000000;
    int smallestNodeDegreeIndex = -1;
    node* smallestNodeDegreePointer = NULL;

    int greatestNodeDegree = -1;
    int greatestNodeDegreeIndex = -1;
    node* greatestNodeDegreePointer = NULL;

    for(int i = 0; i<nodesListSize; i++){
        node* nod = (node*) listGet(graph->nodes, i);
        int nodDegree = listSize(nod->logicalInterference) + listSize(nod->physicalInterference);
        
        if (nodDegree < smallestNodeDegree){
            smallestNodeDegree = nodDegree;
            smallestNodeDegreeIndex = i;
            smallestNodeDegreePointer = nod;

        }else if (nodDegree == smallestNodeDegree){
            if (nod->number < smallestNodeDegreePointer->number){
                smallestNodeDegree = nodDegree;
                smallestNodeDegreeIndex = i;
                smallestNodeDegreePointer = nod;
            }
        }

        if (nodDegree > greatestNodeDegree){
            greatestNodeDegree = nodDegree;
            greatestNodeDegreeIndex = i;
            greatestNodeDegreePointer = nod;

        }else if (nodDegree == greatestNodeDegree){
            if (nod->number < greatestNodeDegreePointer->number){
                greatestNodeDegree = nodDegree;
                greatestNodeDegreeIndex = i;
                greatestNodeDegreePointer = nod;
            }
        }

    }

    node* nodeToRemove = NULL;
    int nodeToRemoveIndex = -1;

    if (smallestNodeDegree >= currentK){ //potential spill
        *spill = 1;
        nodeToRemove = greatestNodeDegreePointer;
        nodeToRemoveIndex = greatestNodeDegreeIndex;
    }else{ //no spill
        nodeToRemove = smallestNodeDegreePointer;
        nodeToRemoveIndex = smallestNodeDegreeIndex;
    }

    //remove it from nodes list.
    listRemove(graph->nodes, nodeToRemoveIndex);
    nodesListSize--;

    //go to its neighbors and remove it from their adjacency list.
    int nodeToRemoveLogAdjSize = listSize(nodeToRemove->logicalInterference);
    //printf(" nodeToRemoveLogAdjSize: %d\n",  nodeToRemoveLogAdjSize);
    for(int i = 0; i<nodeToRemoveLogAdjSize; i++){
        
        int* adjNode = (int*) listGet(nodeToRemove->logicalInterference, i);

        for(int j = 0; j<nodesListSize; j++){
            
            node* nod = (node*) listGet(graph->nodes, j);
            if (nod->number == *adjNode){

                int neighborAdjListSize = listSize(nod->logicalInterference);

                for(int k = 0; k<neighborAdjListSize; k++){
                    
                    int* neighborNumber = (int*) listGet(nod->logicalInterference, k);
                    if(*neighborNumber == nodeToRemove->number){
                        int* num = (int*) listRemove(nod->logicalInterference, k);
                        free(num);
                        break;
                    }
                }
                break;
            }
        }
    }

    //return node.
    return nodeToRemove;
}

void insertNode(interferenceGraph* graph, node* nodeToAdd, int* colors){
    int nodesListSize = listSize(graph->nodes);

    //printf("Inserting %d\n", nodeToAdd->number);

    int nodeToAddLogAdjSize = listSize(nodeToAdd->logicalInterference);

    for(int i = 0; i<nodeToAddLogAdjSize; i++){
        
        int* adjNode = (int*) listGet(nodeToAdd->logicalInterference, i);

        for(int j = 0; j<nodesListSize; j++){
            node* nod = (node*) listGet(graph->nodes, j);

            if (nod->number == *adjNode){
                int* interference = (int*) malloc(sizeof(int)); 
                *interference = nodeToAdd->number;
                listAdd(nod->logicalInterference, interference);

                if(colors != NULL)
                    colors[nod->color] = 1;
                break;
            }
        }
    }

    //add it to nodes list.
    listAdd(graph->nodes, nodeToAdd);
}

int reinsertNode(interferenceGraph* graph, node* nodeToAdd, int currentK){
    
   // printf("\n\nreinserting %d- k: %d\n", nodeToAdd->number, currentK);
    
    //available colors array:
    int colorsArray[currentK];
    int* colors = colorsArray;
    for(int i = 0; i<currentK; i++){
        colors[i] = 0;
    }

    int phyAdjListSize = listSize(nodeToAdd->physicalInterference);

    for(int i = 0; i<phyAdjListSize; i++){
        
        int* adjNode = (int*) listGet(nodeToAdd->physicalInterference, i);

        if (*adjNode < currentK)
            colors[*adjNode] = 1;

    }

    //go to its neighbors and add it to their adjacency list (plus, find color for it...).
    insertNode(graph, nodeToAdd, colors);
    

    //find color for nodeToAdd:
    nodeToAdd->color = -1;
    for(int i = 0; i<currentK; i++){
        if(colors[i] == 0){
            nodeToAdd->color = i;
            break;
        }
    }

    //return assigned color.
    return nodeToAdd->color;
}

void reinsertLeftNodes(interferenceGraph* graph, Stack stack){
    node* nod;
    while (!isStackEmpty(stack)) {
        nod = stackPop(stack);
        insertNode(graph, nod, NULL);
    }
}

int main(){
    //1. build...
    interferenceGraph* graph = createInterferenceGraph();
    Stack stack = newStack();


    char* reportStrings[graph->k - 1];
    int j = 0;
    node* currentNode;
    int potentialSpillFlag;
    int spillFlag;
    char status[50];

    int padding = 1;
    if(graph->k > 9)
        padding = 2;
    else if(graph->k > 99)
        padding = 3;
    else if(graph->k > 999)
        padding = 4;
    
    printf("Graph %d -> Physical Registers: %d\n", graph->id, graph->k);
    printf("----------------------------------------");
    for(int i = graph->k; i>1; i--){
       
        potentialSpillFlag = 0;
        spillFlag = 0;

        printf("\n----------------------------------------\nK = %d\n", i);

        //2. simplify (+ potential spill)...
        while((currentNode = removeNode(graph, i, &potentialSpillFlag)) != NULL){
            stackPush(stack, currentNode);
            if(potentialSpillFlag)
                printf("\nPush: %d *", currentNode->number); // potential spill (*)
            else
                printf("\nPush: %d", currentNode->number); 
            potentialSpillFlag = 0;
        }

        //3. select (+ spill)... 
        while (!isStackEmpty(stack)) {
            currentNode = stackPop(stack);
            int color = reinsertNode(graph, currentNode, i);
            if (color < 0){ //spill: no color available
                printf("\nPop: %d -> NO COLOR AVAILABLE", currentNode->number);
                reinsertLeftNodes(graph, stack);
                spillFlag = 1;
                break;
            }else{
                printf("\nPop: %d -> %d", currentNode->number, color);  
            }
        }

        
        if(spillFlag) //spill occurred...   
            sprintf(status, "SPILL");
        else
            sprintf(status, "Successful Allocation");
        
        //saving report string...
        char I[10];
        sprintf(I, "%d", i);

        char* report = (char*) malloc(sizeof(char)*100); //maximum of 100 bytes for each line of the report...
        sprintf(report, "\nGraph %d -> K = %*s: %s", graph->id, padding, I, status);
        reportStrings[j] = report;
        j++;

    }
    
    printf("\n----------------------------------------\n----------------------------------------");
    for(int i = 0; i < graph->k - 1; i++){
        printf("%s", reportStrings[i]);
        free(reportStrings[i]);
    }

    freeInterferenceGraph(graph);

    return 0;
}