#include <stdio.h>
#include <math.h>
#define maxsize 4
#define maxarraysize 50
#define NORTHCHECK 7
#define EASTCHECK 11
#define SOUTHCHECK 13
#define WESTCHECK 14
#define ACCESSIBLE 15

struct Point
{
    short x;
    short y;
};

struct QueueItem
{
    struct Point nodeName;
    struct Point startNode;
    short currWeight;
    short edgeWeight;
};

struct GraphNode
{
    short checked; // if -1,not checked, if 1, checked, if 3, check for weight to point in A star
    short directions; // directions in binary in NESW eg 1111 1 = open, 0 = closed off
    short routes;    //verify routes that are open and close 
};

void DummyNodes();
void AStar();
short CheckIsPoint(struct Point currPoint, struct Point endPoint);
void SortQueue(struct QueueItem queue[], short end);
void Swapper(struct QueueItem queue[], short currIndex);
void CalcDirection(char currDir, char intendedDir);
short dirShort(char direction);

    short xPoints = 5; //x points = (ultrasonic sensor distance in front + car length) /27
    short yPoints = 4; //y points = (ultrasonic sensor longer distance L/R + car width) /27

    struct GraphNode graphArray[5][4];

int main()
{

    // every time car moves (forward (depends on direction)): increase (x / y)
    // keep track x,y axis
    // if go backwards -1 from corresponding axis
    // check how many nodes are left to check
    // BFS


    // CalcDirection('N', 'N');
    // CalcDirection('N', 'E');
    // CalcDirection('N', 'S');
    // CalcDirection('N', 'W');

    // CalcDirection('E', 'N');
    // CalcDirection('E', 'E');
    // CalcDirection('E', 'S');
    // CalcDirection('E', 'W');

    // CalcDirection('S', 'N');
    // CalcDirection('S', 'E');
    // CalcDirection('S', 'S');
    // CalcDirection('S', 'W');

    // CalcDirection('W', 'N');
    // CalcDirection('W', 'E');
    // CalcDirection('W', 'S');
    // CalcDirection('W', 'W');

    DummyNodes();
    AStar();

}

void DummyNodes()
{
    // N E S W
    struct GraphNode a = {1, 0b0110, 0b0110};
    graphArray[0][0] = a;
    struct GraphNode b = {1, 0b0111, 0b0111};
    graphArray[1][0] = b;
    struct GraphNode c = {1, 0b0111, 0b0111};
    graphArray[2][0] = c;
    struct GraphNode d = {1, 0b0101, 0b0101};
    graphArray[3][0] = d;
    struct GraphNode e = {1, 0b0011, 0b0011};
    graphArray[4][0] = e;

    struct GraphNode f = {1, 0b1010, 0b1010};
    graphArray[0][1] = f;
    struct GraphNode g = {1, 0b1010, 0b1010};
    graphArray[1][1] = g;
    struct GraphNode h = {1, 0b1100, 0b1100};
    graphArray[2][1] = h;
    struct GraphNode i = {1, 0b0001, 0b0001};
    graphArray[3][1] = i;
    struct GraphNode j = {1, 0b1010, 0b1010};
    graphArray[4][1] = j;

    struct GraphNode k = {1, 0b1010, 0b1010};
    graphArray[0][2] = k;
    struct GraphNode l = {1, 0b1100, 0b1100};
    graphArray[1][2] = l;
    struct GraphNode m = {1, 0b0101, 0b0101};
    graphArray[2][2] = m;
    struct GraphNode n = {1, 0b0101, 0b0101};
    graphArray[3][2] = n;
    struct GraphNode o = {1, 0b1011, 0b1011};
    graphArray[4][2] = o;

    struct GraphNode p = {1, 0b1100, 0b1100};
    graphArray[0][3] = p;
    struct GraphNode q = {1, 0b0101, 0b0101};
    graphArray[1][3] = q;
    struct GraphNode r = {1, 0b0101, 0b0101};
    graphArray[2][3] = r;
    struct GraphNode s = {1, 0b0101, 0b0101};
    graphArray[3][3] = s;
    struct GraphNode t = {1, 0b1001, 0b1001};
    graphArray[4][3] = t;
}

void AStar(){

    for (short j = 0; j < xPoints; j++)
    { 
        printf(((graphArray[j][0].directions | NORTHCHECK) != ACCESSIBLE) ? " ___ " : "    ");
    }

    for (short i = 0; i < yPoints; i++)
    {
        printf("\n");
        for (short j = 0; j < xPoints; j++)
        {
            printf(((graphArray[j][i].directions | WESTCHECK) != ACCESSIBLE) ? "|" : " ");
            printf(((graphArray[j][i].directions | SOUTHCHECK) != ACCESSIBLE) ? "___ " : "    ");
        }

        printf(((graphArray[xPoints - 1][i].directions | EASTCHECK) != ACCESSIBLE) ? "|" : " ");
    }

    short currQueue = 0;                        // The number of items in the queue
    short paths = 0;                            // Number of paths travelled
    struct QueueItem queuedNode = {0, 0, 0, 0}; // The node that needs to be checked
    struct Point startPoint;
    struct Point endPoint;

    // To get the starting node
    printf("\nSelect Starting Node x pos: ");
    scanf("%hd", &startPoint.x);
    printf("Select Starting Node y pos: ");
    scanf("%hd", &startPoint.y);

    // To get the goal node
    printf("Select Ending Node x pos: ");
    scanf("%hd", &endPoint.x);
    printf("Select Ending Node y pos: ");
    scanf("%hd", &endPoint.y);

    queuedNode.nodeName= startPoint;
    struct Point pointToAdd = {0,0};
    struct Point neighbourNode[maxsize];                 //Array to hold the neighbours of the node
    short neighbours = 0;                         //Number of neighbours in the node
    struct QueueItem queue[maxarraysize];              //Initialise the queue array with the QueueItem struct, this will hold the queue for the paths to travel
    struct QueueItem pathTravelled[maxarraysize];      //Initialise the pathTravelled array with the QueueItem struct, this will hold the QueueItems of the paths that have been popped from the queue
    graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].checked = 3;  

    while (CheckIsPoint(queuedNode.nodeName, endPoint) == 0) // If the current node to check is not the goal node
    {
        printf("\n\nChecking Node (%i, %i): %i", queuedNode.nodeName.x, queuedNode.nodeName.y, graphArray[queuedNode.nodeName.x][queuedNode.nodeName.x].directions); //    (For checking)

        if ((graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].directions | NORTHCHECK) == ACCESSIBLE) // Checks if north node can be travelled to
        {
            pointToAdd.x = queuedNode.nodeName.x;
            pointToAdd.y = queuedNode.nodeName.y - 1;
            
            if (graphArray[pointToAdd.x][pointToAdd.y].checked != 3) // Checks if north node has been checked before
            {
                neighbourNode[neighbours] = pointToAdd;             // Add the north node of the current point to the neighbours array
                neighbours++;                                       // Increment the number of neighbours
                graphArray[pointToAdd.x][pointToAdd.y].checked = 3; // Set the current point as checked as it will be checked for its weight, prevents point from being checked again
                printf("\nAdd neighbour north: (%i, %i)", pointToAdd.x, pointToAdd.y);
            }
        }

        if ((graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].directions | EASTCHECK) == ACCESSIBLE) // Checks if east node can be travelled to
        {
            pointToAdd.x = queuedNode.nodeName.x + 1;
            pointToAdd.y = queuedNode.nodeName.y;
            
            if (graphArray[pointToAdd.x][pointToAdd.y].checked != 3) // Checks if east node has been checked before
            {
                neighbourNode[neighbours] = pointToAdd;             // Add the east node of the current point to the neighbours array
                neighbours++;                                       // Increment the number of neighbours
                graphArray[pointToAdd.x][pointToAdd.y].checked = 3; // Set the current point as checked as it will be checked for its weight, prevents point from being checked again
                printf("\nAdd neighbour east: (%i, %i)", pointToAdd.x, pointToAdd.y);
            }
        }

        if ((graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].directions | SOUTHCHECK) == ACCESSIBLE) // Checks if south node can be travelled to
        {
            pointToAdd.x = queuedNode.nodeName.x;
            pointToAdd.y = queuedNode.nodeName.y + 1;
            
            if (graphArray[pointToAdd.x][pointToAdd.y].checked != 3) // Checks if south node has been checked before
            {
                neighbourNode[neighbours] = pointToAdd;             // Add the south node of the current point to the neighbours array
                neighbours++;                                       // Increment the number of neighbours
                graphArray[pointToAdd.x][pointToAdd.y].checked = 3; // Set the current point as checked as it will be checked for its weight, prevents point from being checked again
                printf("\nAdd neighbour south: (%i, %i)", pointToAdd.x, pointToAdd.y);
            }
        }

        if ((graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].directions | WESTCHECK) == ACCESSIBLE) // Checks if west node can be travelled to
        {
            pointToAdd.x = queuedNode.nodeName.x - 1;
            pointToAdd.y = queuedNode.nodeName.y;
            
            if (graphArray[pointToAdd.x][pointToAdd.y].checked != 3) // Checks if west node has been checked before
            {
                neighbourNode[neighbours] = pointToAdd;             // Add the west node of the current point to the neighbours array
                neighbours++;                                       // Increment the number of neighbours
                graphArray[pointToAdd.x][pointToAdd.y].checked = 3; // Set the current point as checked as it will be checked for its weight, prevents point from being checked again
                printf("\nAdd neighbour west: (%i, %i)\n", pointToAdd.x, pointToAdd.y);
            }
        }

        for (short i = neighbours - 1; i >= 0; i--) // For each neighbour in the neighbourNode array
        {

            short heuCost = abs(neighbourNode[i].x - endPoint.x) + abs(neighbourNode[i].y - endPoint.y);
            short currWeight = queuedNode.edgeWeight + 1;
            short totalWeight = currWeight + heuCost;

            struct QueueItem newItem = {neighbourNode[i], queuedNode.nodeName, totalWeight, currWeight};
            queue[currQueue] = newItem; // Adds the new item in the queue
            currQueue++;                // Increments the number of items in the queue
        }

        SortQueue(queue, currQueue); // To sort the queue from highest to lowest weight

        // After queue is sorted
        neighbours = 0;                // Resets the number of neighbours
        currQueue -= 1;                // Subtracts 1 from the number of items in the queue
        queuedNode = queue[currQueue]; // Sets the next node to check to the last item in the array (The one with the lowest weight)                                                                               //          (For checking)
        pathTravelled[paths] = queuedNode; // Adds the next node to the paths that have been travelled to
        paths++;                           // Increments the number of paths

        //          (For checking)
        for (short i = paths - 1; i >= 0; i--)
        {
            printf("\nPath %i, travelled: (%i, %i) from: (%i, %i)", i, pathTravelled[i].nodeName.x, pathTravelled[i].nodeName.y, pathTravelled[i].startNode.x, pathTravelled[i].startNode.y);
        }
    }
    // After the goal has been reached
    struct Point finalPath[paths];                // Initialise an array for the final path to take
    paths -= 1;                                   // Subtract 1 from the number of paths
    short route = 1;                              // Initialise the number of routes
    finalPath[0] = pathTravelled[paths].nodeName; // Set the first index of the finalPath array to the last item in the pathTravelled array
    
    // Appends the routes to take to the finalPath array
    // while (CheckIsPoint(finalPath[route - 1], startPoint) == 0) // While the last index in the finalPath array is not the starting node
    // {
        for (short i = paths; i >= 0; i--) // Iterates through the pathTravelled array backwards
        {
            if (CheckIsPoint(pathTravelled[i].nodeName, finalPath[route - 1]) == 1) // If the name of the current iteration of the pathTravelled array matches the name of the last index of the finalPath array
            {
                finalPath[route] = pathTravelled[i].startNode; // Appends the current pathTravelled value to the finalPath array
                route++;                                       // Increments the number of routes

            }
        }
    //}

    // To print out the route to take
    printf("\n\nFinal Path: ");
    for (short i = route - 1; i >= 0; i--)
    {
        printf("(%i,%i), ", finalPath[i].x, finalPath[i].y);
    }

}

short CheckIsPoint(struct Point currPoint, struct Point endPoint)
{
    if (currPoint.x == endPoint.x && currPoint.y == endPoint.y)
    {
        return 1;
    }
    return 0;
}

void SortQueue(struct QueueItem queue[], short end) // To sort the queue from highest to lowest
{
    for (short sorted = end - 1; sorted >= 0; sorted--) // iterates through queue array
    {
        for (short i = end - 1; i > 0; i--) // When the unsorted portion is not at the end yet
        {
            if (queue[i].currWeight >= queue[i - 1].currWeight) // if the curr weight at the current index is greater or equal to the weight of the current index - 1, swap places
            {
                Swapper(queue, i);
            }
        }
    }
}

void Swapper(struct QueueItem queue[], short currIndex) // To swap the values around in the queue
{
    // printf("\n curr index content: %i, %i, %i", queue[currIndex].nodeName, queue[currIndex].startNode, queue[currIndex].currWeight );
    // printf("\n curr index +1 content: %i, %i, %i", queue[currIndex+1].nodeName, queue[currIndex+1].startNode, queue[currIndex+1].currWeight );
    struct QueueItem temp = queue[currIndex]; // Saves the value of the unsorted item into temp
    //  printf("\n tempcontent: %i, %i, %i", temp.nodeName, temp.startNode, temp.currWeight );
    queue[currIndex] = queue[currIndex - 1];
    queue[currIndex - 1] = temp;
}

void CalcDirection(char currDir, char intendedDir)
{
    short dirToTurn = (dirShort(intendedDir)-dirShort(currDir));
    //printf("%d", dirToTurn);

    if(dirToTurn == 0)
    {
        //stay
        printf("STAY  ");
    }
    else if (dirToTurn == 1 || dirToTurn == -3)
    {
        //right
        printf("right  ");
    }
    else if (dirToTurn == 2 || dirToTurn == -2)
    {
        //u turn
        printf("u turn  ");
    }
    else if (dirToTurn == 3 || dirToTurn == -1)
    {
        //left
        printf("left  ");
    }

}

short dirShort(char direction)
{

    if(direction == 'N')
    {
        return 0;
    }
    else if (direction == 'E')
    {
        return 1;
    }
    else if (direction == 'S')
    {
        return 2;
    }
    else if (direction == 'W')
    {
        return 3;
    }
}