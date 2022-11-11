#include <stdio.h>
#include <math.h>
#define maxsize 4      
#define queuesize 20     

struct Point
{
    int x;
    int y;
};

struct QueueItem
{
    struct Point nodeName;
    struct Point startNode;
    int currWeight;
    int edgeWeight;
};

struct GraphNode
{
    int checked;      //if -1,not checked, if 1, checked, if 3, check for weight to point in A star
    int north;      //if -1, not checked.  if 0, checked but cannot be travelled to. if 1, checked can be travelled to
    int east;
    int south;
    int west;
};


int CheckIsPoint(struct Point currPoint, struct Point endPoint);
int CalculateHeuristics(int x, int y, int endX, int endY);
void SortQueue(struct QueueItem queue[], int end);
void Swapper(struct QueueItem queue[], int currIndex);

int main()
{
    int edges;
    int nodes;
    int xPoints;
    int yPoints;
    printf("Enter number of points on the x axis: ");
    scanf("%i", &xPoints);
    printf("Enter number of points on the y axis: ");
    scanf("%i",&yPoints);

    // struct QueueItem queue[queuesize];              //Initialise the queue array with the QueueItem struct, this will hold the queue for the paths to travel
    // struct QueueItem pathTravelled[queuesize];      //Initialise the pathTravelled array with the QueueItem struct, this will hold the QueueItems of the paths that have been popped from the queue
    // struct GraphNode graphArray[queuesize][queuesize];

    struct QueueItem queue[queuesize];              //Initialise the queue array with the QueueItem struct, this will hold the queue for the paths to travel
    struct QueueItem pathTravelled[queuesize];      //Initialise the pathTravelled array with the QueueItem struct, this will hold the QueueItems of the paths that have been popped from the queue
    struct GraphNode graphArray[xPoints][yPoints];
    struct GraphNode z = {1, 0, 0, 0, 0};

    
    for (int i = 0; i < xPoints; i++)
    {
        for(int j = 0; j < yPoints; j++)
        {
            printf("\nEnter north of (%i,%i): ",i ,j);
            scanf("%i", &z.north);
            printf("Enter east of (%i,%i): ",i ,j);
            scanf("%i", &z.east);
            printf("Enter south of (%i,%i): ",i ,j);
            scanf("%i", &z.south);
            printf("Enter west of (%i,%i): ",i ,j);
            scanf("%i", &z.west);
            graphArray[i][j]=z;
        }
    }

    /*
    //FOR TESTING
    xPoints = 5;
    yPoints = 4;
    struct GraphNode a = {1, 0, 1, 1, 0};
    graphArray[0][0]= a;
    struct GraphNode b = {1, 0, 1, 1, 1};
    graphArray[1][0]= b;
    struct GraphNode c = {1, 0, 1, 1, 1};
    graphArray[2][0]= c;
    struct GraphNode d = {1, 0, 1, 0, 1};
    graphArray[3][0]= d;
    struct GraphNode e = {1, 0, 0, 1, 1};
    graphArray[4][0]= e;

    struct GraphNode f = {1, 1, 0, 1, 0};
    graphArray[0][1]= f;
    struct GraphNode g = {1, 1, 0, 1, 0};
    graphArray[1][1]= g;
    struct GraphNode h = {1, 1, 1, 0, 0};
    graphArray[2][1]= h;
    struct GraphNode i = {1, 0, 0, 0, 1};
    graphArray[3][1]= i;
    struct GraphNode j = {1, 1, 0, 1, 0};
    graphArray[4][1]= j;

    struct GraphNode k = {1, 1, 0, 1, 0};
    graphArray[0][2]= k;
    struct GraphNode l = {1, 1, 1, 0, 0};
    graphArray[1][2]= l;
    struct GraphNode m = {1, 0, 1, 0, 1};
    graphArray[2][2]= m;
    struct GraphNode n = {1, 0, 1, 0, 1};
    graphArray[3][2]= n;
    struct GraphNode o = {1, 1, 0, 1, 1};
    graphArray[4][2]= o;
    
    struct GraphNode p = {1, 1, 1, 0, 0};
    graphArray[0][3]= p;
    struct GraphNode q = {1, 0, 1, 0, 1};
    graphArray[1][3]= q;
    struct GraphNode r = {1, 0, 1, 0, 1};
    graphArray[2][3]= r;
    struct GraphNode s = {1, 0, 1, 0, 1};
    graphArray[3][3]= s;
    struct GraphNode t = {1, 1, 0, 0, 1};
    graphArray[4][3]= t;
    */
 

    for (int j = 0; j < xPoints; j++)
    {
        printf(graphArray[j][0].north == 0?  "___ " :"    ");
    }

    for(int i = 0; i < yPoints;i++)
    {
        printf("\n");
        for (int j = 0; j < xPoints; j++)
        {
            printf(graphArray[j][i].west == 0?  "|" :" ");
            //printf(graphArray[j][i].north == 0?  "-" :" ");
            //printf("(%i,%i)",j,i);
            printf(graphArray[j][i].south == 0?  "___" :"   ");

        }            
        
        printf(graphArray[xPoints-1][i].east == 0?  "|" :" ");

    }


    int currQueue = 0;                              //The number of items in the queue
    int paths = 0;                                  //Number of paths travelled
    struct QueueItem queuedNode = {0,0,0,0};        //The node that needs to be checked

    //To get the starting node
    struct Point startPoint;
    printf("\nSelect Starting Node x pos: "); 
    scanf("%i",&startPoint.x);
    printf("Select Starting Node y pos: ");
    scanf("%i",&startPoint.y);

    //To get the goal node
    struct Point endPoint;
    printf("Select Ending Node x pos: ");
    scanf("%i",&endPoint.x);
    printf("Select Ending Node y pos: ");
    scanf("%i",&endPoint.y);
    //printf("heu test: %i", CalculateHeuristics(startPoint.x,startPoint.y,endPoint.x,endPoint.y));
     
    queuedNode.nodeName = startPoint;
    struct Point pointToAdd = {0,0};
    struct Point neighbourNode[maxsize];                 //Array to hold the neighbours of the node
    int neighbours = 0;                         //Number of neighbours in the node
    graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].checked = 3;  

    while (CheckIsPoint(queuedNode.nodeName, endPoint) == 0)       //If the current node to check is not the goal node
    {
        printf("\n\nChecking Node (%i, %i): {%i, %i, %i, %i}", queuedNode.nodeName.x, queuedNode.nodeName.y, graphArray[queuedNode.nodeName.x][queuedNode.nodeName.x].north, graphArray[queuedNode.nodeName.x][queuedNode.nodeName.x].east, graphArray[queuedNode.nodeName.x][queuedNode.nodeName.x].south, graphArray[queuedNode.nodeName.x][queuedNode.nodeName.x].west);  //    (For checking)
       
        
        if(graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].north == 1) //Checks if north node can be travelled to
        {
            pointToAdd.x = queuedNode.nodeName.x;
            pointToAdd.y = queuedNode.nodeName.y-1;
            //printf("\nChecking Node (%i, %i)", pointToAdd.x, pointToAdd.y);  //    (For checking)

            if(graphArray[pointToAdd.x][pointToAdd.y].checked != 3)       //Checks if north node has been checked before
            {
                neighbourNode[neighbours] = pointToAdd;         //Add the north node of the current point to the neighbours array
                neighbours++;                                   //Increment the number of neighbours
                graphArray[pointToAdd.x][pointToAdd.y].checked = 3;                       //Set the current point as checked as it will be checked for its weight, prevents point from being checked again
                printf("Add nieghbour north: (%i, %i)",pointToAdd.x, pointToAdd.y );
            }
        }

        if(graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].east == 1) //Checks if north node can be travelled to
        {
            pointToAdd.x = queuedNode.nodeName.x+1;
            pointToAdd.y = queuedNode.nodeName.y;
            //printf("\nChecking Node (%i, %i)", pointToAdd.x, pointToAdd.y);  //    (For checking)
            
            if(graphArray[pointToAdd.x][pointToAdd.y].checked != 3)       //Checks if east node has been checked before
            {
                neighbourNode[neighbours] = pointToAdd;         //Add the east node of the current point to the neighbours array
                neighbours++;                                   //Increment the number of neighbours
                graphArray[pointToAdd.x][pointToAdd.y].checked = 3;                       //Set the current point as checked as it will be checked for its weight, prevents point from being checked again
                printf("Add nieghbour east: (%i, %i)",pointToAdd.x, pointToAdd.y );
            }
        }

        if(graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].south == 1) //Checks if south node can be travelled to
        {
            pointToAdd.x = queuedNode.nodeName.x;
            pointToAdd.y = queuedNode.nodeName.y+1;
            //printf("\nChecking Node (%i, %i)", pointToAdd.x, pointToAdd.y);  //    (For checking)
            
            if(graphArray[pointToAdd.x][pointToAdd.y].checked != 3)       //Checks if south node has been checked before
            {
                neighbourNode[neighbours] = pointToAdd;         //Add the south node of the current point to the neighbours array
                neighbours++;                                   //Increment the number of neighbours
                graphArray[pointToAdd.x][pointToAdd.y].checked = 3;                       //Set the current point as checked as it will be checked for its weight, prevents point from being checked again
                printf("Add nieghbour south: (%i, %i)",pointToAdd.x, pointToAdd.y );
            }
        }

        if(graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].west == 1) //Checks if west node can be travelled to
        {
            pointToAdd.x = queuedNode.nodeName.x-1;
            pointToAdd.y = queuedNode.nodeName.y;
            //printf("\nChecking Node (%i, %i)", pointToAdd.x, pointToAdd.y);  //    (For checking)
            
            if(graphArray[pointToAdd.x][pointToAdd.y].checked != 3)       //Checks if west node has been checked before
            {
                neighbourNode[neighbours] = pointToAdd;         //Add the west node of the current point to the neighbours array
                neighbours++;                                   //Increment the number of neighbours
                graphArray[pointToAdd.x][pointToAdd.y].checked = 3;                       //Set the current point as checked as it will be checked for its weight, prevents point from being checked again
                printf("Add nieghbour west: (%i, %i)\n",pointToAdd.x, pointToAdd.y );
            }
        }
     

        for(int i = 0; i < neighbours;i++)                  //For each neighbour in the neighbourNode array
        {

            int heuCost = abs(neighbourNode[i].x - endPoint.x) + abs(neighbourNode[i].y - endPoint.y);
            //printf("\nHeuristics of target node(%i,%i): %i", neighbourNode[i].x, neighbourNode[i].y, heuCost);              //          (for checking)
            int currWeight = queuedNode.edgeWeight + 1;
            int totalWeight = currWeight + heuCost;

            struct QueueItem newItem = {neighbourNode[i], queuedNode.nodeName, totalWeight, currWeight};
            //printf("\nTotal weight: %i",  newItem.currWeight);              //          (for checking)
            queue[currQueue] = newItem;                             //Adds the new item in the queue
            currQueue++;                                            //Increments the number of items in the queue
            //printf("    Curr Queue: %i\n", currQueue);              //          (for checking)
            
        }
        
   
        
        //  for(int i = 0; i < currQueue; i++)              
        // {
        //     printf("\nQueue %i node: (%i, %i), starting node: (%i, %i), weight: %i", i, queue[i].nodeName.x, queue[i].nodeName.y, queue[i].startNode.x, queue[i].startNode.y, queue[i].currWeight);
        // }
        SortQueue(queue, currQueue);                 //To sort the queue from highest to lowest weight

        //          (For checking)
        // printf("\nSorted array");
        // for(int i = 0; i < currQueue; i++)              
        // {
        //     printf("\nQueue %i node: (%i, %i), starting node: (%i, %i), weight: %i", i, queue[i].nodeName.x, queue[i].nodeName.y, queue[i].startNode.x, queue[i].startNode.y, queue[i].currWeight);
        // }
        

        

        //After queue is sorted
        neighbours = 0;                     //Resets the number of neighbours
        currQueue -= 1;                     //Subtracts 1 from the number of items in the queue
        queuedNode = queue[currQueue];      //Sets the next node to check to the last item in the array (The one with the lowest weight)
       // printf("\nLowest cost:(%i, %i), from: (%i, %i), with: %i", queuedNode.nodeName.x, queuedNode.nodeName.y, queuedNode.startNode.x, queuedNode.startNode.y, queuedNode.currWeight);      //          (For checking)
       // printf("    Curr Queue: %i\n", currQueue);                                                                                //          (For checking)
        pathTravelled[paths] = queuedNode;              //Adds the next node to the paths that have been travelled to
        paths++;                                        //Increments the number of paths
            
        //          (For checking)
        for (int i = paths- 1; i >=0; i--)
        {
            printf("\n Path %i, travelled: (%i, %i) from:(%i, %i)", i, pathTravelled[i].nodeName.x, pathTravelled[i].nodeName.y, pathTravelled[i].startNode.x, pathTravelled[i].startNode.y);
        }
    
    }
    //After the goal has been reached
    struct Point finalPath[paths];               //Initialise an array for the final path to take 
    paths -= 1;                         //Subtract 1 from the number of paths
    int route = 1;                      //Initialise the number of routes
    finalPath[0] = pathTravelled[paths].nodeName;           //Set the first index of the finalPath array to the last item in the pathTravelled array
    printf("\nFinal: (%i, %i)", pathTravelled[paths].nodeName.x, pathTravelled[paths].nodeName.y);       //          (for checking)

    //Appends the routes to take to the finalPath array
    while (CheckIsPoint(finalPath[route-1], startPoint) ==0)       //While the last index in the finalPath array is not the starting node
    {
        //printf("Node now: %i", finalPath[route-1]);             //          (for checking)   
        for (int i = paths; i >=0; i--)                             //Iterates through the pathTravelled array backwards
        {
            //printf("Node name: %i", pathTravelled[i].nodeName);       //          (for checking)
            if (CheckIsPoint(pathTravelled[i].nodeName, finalPath[route-1]) == 1)        //If the name of the current iteration of the pathTravelled array matches the name of the last index of the finalPath array
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
        printf("(%i,%i), ", finalPath[i].x,  finalPath[i].y);
    }
    
    
}

int CheckIsPoint(struct Point currPoint, struct Point endPoint)
{
    if(currPoint.x == endPoint.x && currPoint.y == endPoint.y)
    {
        return 1;
    }

    return 0;
}

int CalculateHeuristics(int x, int y, int endX, int endY)
{
    int heu = 0;
    heu = abs(endX - x) + abs(endY - y);
    return heu;
}


void SortQueue(struct QueueItem queue[], int end) //To sort the queue from highest to lowest
{
    int currIndex = 0;
    int sorted = 0;

    while (sorted < end)
    {
        while (currIndex +1  < end)                          //When the unsorted portion is not at the end yet
        {  
           // printf("\nunsorted: %i , %i",currIndex, queue[currIndex]);
           // printf("\ncurr index: %i", currIndex);
            if( queue[currIndex].currWeight <= queue[currIndex + 1].currWeight)       //When the current unsorted item is more than the one before it, decrease the currIndex by 1
            {
                Swapper(queue, currIndex) ;                                                         //sets the queue at the current index to temp                                            
            }
            //printf("curr index: %i",currIndex);
            currIndex ++;

        }
        sorted++;
        currIndex = 0;
    }
}

void Swapper(struct QueueItem queue[], int currIndex)          //To swap the values around in the queue
{
   // printf("\n curr index content: %i, %i, %i", queue[currIndex].nodeName, queue[currIndex].startNode, queue[currIndex].currWeight );
   // printf("\n curr index +1 content: %i, %i, %i", queue[currIndex+1].nodeName, queue[currIndex+1].startNode, queue[currIndex+1].currWeight );
    struct QueueItem temp = queue[currIndex];                                                //Saves the value of the unsorted item into temp
  //  printf("\n tempcontent: %i, %i, %i", temp.nodeName, temp.startNode, temp.currWeight );
    queue[currIndex] =  queue[currIndex+1];
    queue[currIndex+1] = temp;    
    
}



