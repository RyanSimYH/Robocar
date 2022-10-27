#include <stdio.h>
#include <math.h>
#define maxsize 4      
#define queuesize 50     

struct Node {
    int nodeName;
    int heuristic;
};

struct Edge
{
    int node1;
    int node2;
    int weight;
    int checked;
};

struct QueueItem
{
    int nodeName;
    int startNode;
    int currWeight;
    int edgeWeight;
};

struct QueueItem CalculateEdgeCost(struct Edge edgeArray[], int edges, struct QueueItem startNode, int targetNode);
int GetHeuristics(struct Node nodeArray[], int nodes, int node);
void SortQueue(struct QueueItem queue[], int end);


int main()
{
    int edges;
    int nodes;

    struct QueueItem queue[queuesize];              //Initialise the queue array with the QueueItem struct, this will hold the queue for the paths to travel
    struct QueueItem pathTravelled[queuesize];      //Initialise the pathTravelled array with the QueueItem struct, this will hold the QueueItems of the paths that have been popped from the queue
    
    /*
    struct QueueItem a = {4, 1, 8,8 };
    queue[0] = a;
    struct QueueItem b = {3, 1, 4, 4};
    queue[1] = b;
    struct QueueItem c  = {2, 1, 2, 2};
    queue[2] = c;
    struct QueueItem d = {5, 1, 1, 1};
    queue[3] = d;
    struct QueueItem e = {6, 1, 9, 9};
    queue[4] = e;

     for(int i = 0; i < 5; i++)              
    {
        printf("\nQueue %i node: %i, starting node: %i, weight: %i", i, queue[i].nodeName, queue[i].startNode, queue[i].currWeight);
    }
    printf("\n\n");

    SortQueue(queue, 3, 5);                 //To sort the queue from highest to lowest weight
    */
    
    // printf("\nNumber of Edges: ");              
    // scanf("%i",&edges);                           //To get the number of edges in the map
   edges = 11;
    struct Edge edgeArray[edges];
    struct Edge a = {1 ,2, 9, 0};
    edgeArray[0] = a;
    struct Edge b = {2, 3, 3, 0};
    edgeArray[1] = b;
    struct Edge c = {3, 4, 3, 0};
    edgeArray[2] = c;
    struct Edge d = {4, 7, 3, 0};
    edgeArray[3] = d;
    struct Edge e = {7, 8, 6, 0};
    edgeArray[4] = e;
    struct Edge k = {8, 9, 3, 0};
    edgeArray[5] = k;
    struct Edge l = {9, 1, 9, 0};
    edgeArray[6] = l;
    struct Edge m = {3, 5, 6, 0};
    edgeArray[7] = m;
    struct Edge n = {5, 8, 6, 0};
    edgeArray[8] = n;
    struct Edge o = {4, 6, 2, 0};
    edgeArray[9] = o;
    struct Edge p = {6,10, 2, 0};
    edgeArray[10] = p;
    
    
    // for(int i = 0; i < edges; i++)                  //for each edge in the array get the from and to node and their edge weight
    // {
    //     printf("\nNext Edge: ");
    //     scanf("%i",&edgeArray[i].node1); 
    //     printf("\nTo Edge: ");
    //     scanf("%i",&edgeArray[i].node2); 
    //     printf("\nEdge weight: ");
    //     scanf("%i",&edgeArray[i].weight); 
    // }

    // printf("\nNumber of nodes : ");
    // scanf("%i",&nodes);                             //gets the total number of nodes in the map
    nodes = 10;
    struct Node nodeArray[nodes];                   //instantiates the array
    struct Node f = {1,18};
    nodeArray[0] = f;
    struct Node g = {2,9};
    nodeArray[1] = g;   
    struct Node h = {3,7};
    nodeArray[2] = h;    
    struct Node i = {4,4};
    nodeArray[3] = i;     
    struct Node j = {5,13};
    nodeArray[4] = j;     
    struct Node q = {6,2};
    nodeArray[5] = q;
    struct Node r = {7,7};
    nodeArray[6] = r;   
    struct Node s = {8,13};
    nodeArray[7] = s;    
    struct Node t = {9,16};
    nodeArray[8] = t;     
    struct Node u = {10,0};
    nodeArray[9] = u;       

    // for(int i = 0; i < nodes; i++)                  //for each node in the array
    // {
    //     printf("\nNext node name : ");                   //get the name of the node
    //     scanf("%i",&nodeArray[i].nodeName);
    //     printf("Heuristics : ");                  //gets the number of neighbours this node has
    //     scanf("%i",&nodeArray[i].heuristic);

    // }

    for(int i = 0; i < nodes; i++)                  //for each node in the array print the node name and heuristic (For checking)
    {
        printf("\nNode name: %i    ", nodeArray[i].nodeName);
        printf("Heuristic: %i    ", nodeArray[i].heuristic);
    }

    for(int i = 0; i < edges; i++)                  //for each edge in the array print the nodes they are attached to and the weight (for checking)
    {
        printf("\nEdge of: %i  &  %i      Weight: %i", edgeArray[i].node1, edgeArray[i].node2, edgeArray[i].weight);
    }

    int currQueue = 0;                              //The number of items in the queue
    int paths = 0;                                  //Number of paths travelled
    int startNode;                                  //Starting node 
    int targetNode;                                 //Goal node
    struct QueueItem queuedNode = {0,0,0,0};        //The node that needs to be checked
    printf("\nCalculate edge between starting node: ");                  
    scanf("%i",&startNode);                         //To get the starting node
    queuedNode.nodeName = startNode;
    printf("and target: ");                  //gets the number of neighbours this node has
    scanf("%i",&targetNode);                    //To get the goal node
    int neighbourNode[maxsize];                 //Array to hold the neighbours of the node
    int neighbours = 0;                         //Number of neighbours in the node

    while (queuedNode.nodeName != targetNode)       //If the current node to check is not the goal node
    {
        printf("\n\nChecking Node %i", queuedNode.nodeName);  //    (For checking)
        for (int i = 0; i < edges;i++)                  //Iterates through the edge array
        {
            if(edgeArray[i].checked == 0)               //If the edge hasnt been checked yet
            {
                if(edgeArray[i].node1 == queuedNode.nodeName)      //If the first node of the current interation of the edge array matches the current node to check
                {
                    neighbourNode[neighbours] = edgeArray[i].node2;         //Add the second node of the current interation of the edge array to the neighbours array, it is the node that the current node can travel to 
                    neighbours++;                                   //Increment the number of neighbours
                    edgeArray[i].checked = 1;                       //Set the current edge as checked as it will be checked for its weight
                    printf("    Add nieghbour1: %i",edgeArray[i].node2 ); //    (for checking)
                }
                else if(edgeArray[i].node2 == queuedNode.nodeName)  //If the second node of the current interation of the edge array matches the current node to check
                {
                    neighbourNode[neighbours] = edgeArray[i].node1; //Add the first node of the current interation of the edge array to the neighbours array, it is the node that the current node can travel to 
                    neighbours++;                                    //Increment the number of neighbours
                    edgeArray[i].checked = 1;                            //Increment the number of neighbours
                    printf("    Add nieghbour2: %i",edgeArray[i].node1 );
                }
            }
        }
            

        for(int i = 0; i < neighbours;i++)                  //For each neighbour in the neighbourNode array
        {
            struct QueueItem newItem = CalculateEdgeCost(edgeArray, edges, queuedNode, neighbourNode[i]);       //Calculate the edge cost of the edge connected to the current node and sets it in newItem
            printf("\nEdge Cost: %i", newItem.edgeWeight);              //          (for checking)

             int heuCost = GetHeuristics(nodeArray, nodes, neighbourNode[i]);             //Gets the heurisitics of the node and sets it in newItem
            printf("\nHeuristics of target node: %i", heuCost);              //          (for checking)
            
            newItem.currWeight = newItem.edgeWeight+ heuCost;               //Calculates the current weight of the node and sets it in newItem
            printf("\nTotal weight: %i",  newItem.currWeight);              //          (for checking)
            queue[currQueue] = newItem;                             //Adds the new item in the queue
            currQueue++;                                            //Increments the number of items in the queue
            printf("    Curr Queue: %i\n", currQueue);              //          (for checking)
        }
         for(int i = 0; i < currQueue; i++)              
        {
            printf("\nQueue %i node: %i, starting node: %i, weight: %i", i, queue[i].nodeName, queue[i].startNode, queue[i].currWeight);
        }
        SortQueue(queue, currQueue);                 //To sort the queue from highest to lowest weight
        printf("cur q: %i", currQueue);
        //          (For checking)
        printf("\nSorted array");
        for(int i = 0; i < currQueue; i++)              
        {
            printf("\nQueue %i node: %i, starting node: %i, weight: %i", i, queue[i].nodeName, queue[i].startNode, queue[i].currWeight);
        }

        //After queue is sorted
        neighbours = 0;                     //Resets the number of neighbours
        currQueue -= 1;                     //Subtracts 1 from the number of items in the queue
        queuedNode = queue[currQueue];      //Sets the next node to check to the last item in the array (The one with the lowest weight)
        printf("\nLowest cost: %i, from: %i, with: %i", queuedNode.nodeName, queuedNode.startNode, queuedNode.currWeight);      //          (For checking)
        printf("    Curr Queue: %i", currQueue);                                                                                //          (For checking)
        pathTravelled[paths] = queuedNode;              //Adds the next node to the paths that have been travelled to
        paths++;                                        //Increments the number of paths
            
        //          (For checking)
        for (int i = paths- 1; i >=0; i--)
        {
            printf("\n Path %i, travelled: %i from: %i", i, pathTravelled[i].nodeName, pathTravelled[i].startNode);
        }
        
    }

    //After the goal has been reached
    int finalPath[paths];               //Initialise an array for the final path to take 
    paths -= 1;                         //Subtract 1 from the number of paths
    int route = 1;                      //Initialise the number of routes
    finalPath[0] = pathTravelled[paths].nodeName;           //Set the first index of the finalPath array to the last item in the pathTravelled array
    printf("\nFinal: %i", pathTravelled[paths].nodeName);       //          (for checking)

    //Appends the routes to take to the finalPath array
    while ((finalPath[route-1] != startNode))       //While the last index in the finalPath array is not the starting node
    {
        //printf("Node now: %i", finalPath[route-1]);             //          (for checking)   
        for (int i = paths; i >=0; i--)                             //Iterates through the pathTravelled array backwards
        {
            //printf("Node name: %i", pathTravelled[i].nodeName);       //          (for checking)
            if (pathTravelled[i].nodeName == finalPath[route-1])        //If the name of the current iteration of the pathTravelled array matches the name of the last index of the finalPath array
            {
                //printf("add: %i\n", pathTravelled[i].nodeName);       //          (for checking)
                finalPath[route] = pathTravelled[i].startNode;          //Appends the current pathTravelled value to the finalPath array
                route ++;                                               //Increments the number of routes
                break;
            }
        }
    }
    
    //To print out the route to take 
    printf("\nFinal Path: ");
    for(int i = route -1; i >=0; i--)
    {
        printf("%i, ", finalPath[i]);
    }
}


struct QueueItem CalculateEdgeCost(struct Edge edgeArray[], int edges, struct QueueItem startNode, int targetNode)          //Calculates the edge costs between 2 nodes
{
    struct QueueItem newItem;                       //Initialises a new QueueItem
    int edgeCost;                                   //To hold the value of the edge weight
    newItem.nodeName = targetNode;                  //Sets the name of newItem to the target node
    newItem.startNode = startNode.nodeName;         //Sets the name of newItem to the start node
    int startingNode = startNode.nodeName;          //initialises a starting node and sets it to the start node
    for(int i = 0; i < edges; i++)                  //Iterates through the edgeArray
    {
        if ((edgeArray[i].node1 == startingNode && edgeArray[i].node2 == targetNode) ||(edgeArray[i].node1 == targetNode && edgeArray[i].node2 == startingNode))            //If the edge is between the start and target node
        {
            edgeCost = edgeArray[i].weight;             //gets the edge weight of the current iteration of the edgeArray
            printf("\nEdge weight of node %i and %i: %i", startingNode, targetNode, edgeCost);          // (for checking)
            newItem.edgeWeight = edgeCost + startNode.edgeWeight;                           //Sets the edge weight of the newItem to the edge cost and the edge cost of the previous edges
            return newItem;                                                     //Returns the newItem
        }
    }
}

int GetHeuristics(struct Node nodeArray[], int nodes, int node)               //To get the heuristic value of the node
{
    int heuCost;
    for(int i = 0; i < nodes; i++)                                          //Iterates through the nodeArray
    {
        if(nodeArray[i].nodeName == node)                                   //If the current iteration of the nodeArray matches the node to check
        {
            heuCost = nodeArray[i].heuristic;                               //Gets the value of the heuristic and returns that value
            printf("\nHeuristic of node %i: %i", nodeArray[i].nodeName, heuCost);    //(for checking)
            return heuCost;
        }
    }

}

void SortQueue(struct QueueItem queue[], int end) //To sort the queue from highest to lowest
{
    int currIndex = 0;
    int sorted = 0;

    while (sorted < end)
    {
        while (currIndex +1 < end)                          //When the unsorted portion is not at the end yet
        {  
            if( queue[currIndex].currWeight < queue[currIndex + 1].currWeight)       //When the current unsorted item is more than the one before it, decrease the currIndex by 1
            {
                struct QueueItem temp = queue[currIndex];                                                //Saves the value of the unsorted item into temp
                queue[currIndex] =  queue[currIndex+1];
                queue[currIndex+1] = temp;                                                                //sets the queue at the current index to temp                                            
            }
            currIndex ++;

        }
        sorted++;
        currIndex = 0;
    }
    
}



