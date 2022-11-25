#include <stdio.h>
#include <math.h>
#define maxsize 4
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
    // change to int eg 0000
    short directions; // directions in binary in NESW eg 1111 1 = open, 0 = closed off
    short routes; //verify routes that are open and close 
};

short CheckIsPoint(struct Point currPoint, struct Point endPoint);
void SortQueue(struct QueueItem queue[], short end);
void Swapper(struct QueueItem queue[], short currIndex);
void CalcDirection(struct Point currPoint, struct Point endPoint);

int main()
{
    short xPoints;
    short yPoints;
    // printf("Enter number of points on the x axis: ");
    // scanf("%i", &xPoints);
    // printf("Enter number of points on the y axis: ");
    // scanf("%i",&yPoints);

    // FOR TESTING
    xPoints = 5; // x points = (ultrasonic sensor distance in front + car length) /27
    yPoints = 4; // y points = (ultrasonic sensor longer distance L/R + car width) /27

    short queuesize = xPoints * yPoints;

    struct QueueItem queue[queuesize];         // Initialise the queue array with the QueueItem struct, this will hold the queue for the paths to travel
    struct QueueItem pathTravelled[queuesize]; // Initialise the pathTravelled array with the QueueItem struct, this will hold the QueueItems of the paths that have been popped from the queue
    struct GraphNode graphArray[xPoints][yPoints];

    // for (int i = 0; i < xPoints; i++)
    // {
    //     for(int j = 0; j < yPoints; j++)
    //     {
    //         printf("\nEnter north of (%i,%i): ",i ,j);
    //         scanf("%i", &z.north);
    //         printf("Enter east of (%i,%i): ",i ,j);
    //         scanf("%i", &z.east);
    //         printf("Enter south of (%i,%i): ",i ,j);
    //         scanf("%i", &z.south);
    //         printf("Enter west of (%i,%i): ",i ,j);
    //         scanf("%i", &z.west);
    //         graphArray[i][j]=z;
    //     }
    // }

    // //FOR TESTING
    // xPoints = 5; //x points = (ultrasonic sensor distance in front + car length) /27
    // yPoints = 4; //y points = (ultrasonic sensor longer distance L/R + car width) /27

    // every time car moves (forward (depends on direction)): increase (x / y)
    // keep track x,y axis
    // if go backwards -1 from corresponding axis
    // check how many nodes are left to check
    // BFS

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

    for (short j = 0; j < xPoints; j++)
    { // printf((graphArray[j][0].directions| NORTHCHECK) != ACCESSIBLE)
        printf(((graphArray[j][0].directions | NORTHCHECK) != ACCESSIBLE) ? " ___ " : "    ");
    }

    for (short i = 0; i < yPoints; i++)
    {
        printf("\n");
        for (short j = 0; j < xPoints; j++)
        {
            // printf(graphArray[j][i].west == 0?  "|" :" ");
            printf(((graphArray[j][i].directions | WESTCHECK) != ACCESSIBLE) ? "|" : " ");
            // printf(graphArray[j][i].north == 0?  "-" :" ");
            // printf("(%i,%i)",j,i);
            // printf(graphArray[j][i].south == 0?  "___" :"   ");
            printf(((graphArray[j][i].directions | SOUTHCHECK) != ACCESSIBLE) ? "___ " : "    ");
        }

        // printf(graphArray[xPoints-1][i].east == 0?  "|" :" ");
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
    // printf("heu test: %i", CalculateHeuristics(startPoint.x,startPoint.y,endPoint.x,endPoint.y));

    queuedNode.nodeName = startPoint;
    struct Point pointToAdd = {0, 0};
    struct Point neighbourNode[maxsize]; // Array to hold the neighbours of the node
    short neighbours = 0;                // Number of neighbours in the node
    graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].checked = 3;

    while (CheckIsPoint(queuedNode.nodeName, endPoint) == 0) // If the current node to check is not the goal node
    {
        printf("\n\nChecking Node (%i, %i): %i", queuedNode.nodeName.x, queuedNode.nodeName.y, graphArray[queuedNode.nodeName.x][queuedNode.nodeName.x].directions); //    (For checking)

        if ((graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].directions | NORTHCHECK) == ACCESSIBLE) // Checks if north node can be travelled to
        {
            pointToAdd.x = queuedNode.nodeName.x;
            pointToAdd.y = queuedNode.nodeName.y - 1;
            // printf("\nChecking Node (%i, %i)", pointToAdd.x, pointToAdd.y);  //    (For checking)

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
            // printf("\nChecking Node (%i, %i)", pointToAdd.x, pointToAdd.y);  //    (For checking)

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
            // printf("\nChecking Node (%i, %i)", pointToAdd.x, pointToAdd.y);  //    (For checking)

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
            // printf("\nChecking Node (%i, %i)", pointToAdd.x, pointToAdd.y);  //    (For checking)

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
            // printf("\nHeuristics of target node(%i,%i): %i", neighbourNode[i].x, neighbourNode[i].y, heuCost);              //          (for checking)
            short currWeight = queuedNode.edgeWeight + 1;
            short totalWeight = currWeight + heuCost;

            struct QueueItem newItem = {neighbourNode[i], queuedNode.nodeName, totalWeight, currWeight};
            // printf("\nTotal weight: %i",  newItem.currWeight);              //          (for checking)
            queue[currQueue] = newItem; // Adds the new item in the queue
            currQueue++;                // Increments the number of items in the queue
            // printf("    Curr Queue: %i\n", currQueue);              //          (for checking)
        }

        //  for(int i = 0; i < currQueue; i++)
        // {
        //     printf("\nQueue %i node: (%i, %i), starting node: (%i, %i), weight: %i", i, queue[i].nodeName.x, queue[i].nodeName.y, queue[i].startNode.x, queue[i].startNode.y, queue[i].currWeight);
        // }
        SortQueue(queue, currQueue); // To sort the queue from highest to lowest weight

        //          (For checking)
        // printf("\nSorted array");
        // for(int i = 0; i < currQueue; i++)
        // {
        //     printf("\nQueue %i node: (%i, %i), starting node: (%i, %i), weight: %i", i, queue[i].nodeName.x, queue[i].nodeName.y, queue[i].startNode.x, queue[i].startNode.y, queue[i].currWeight);
        // }

        // After queue is sorted
        neighbours = 0;                // Resets the number of neighbours
        currQueue -= 1;                // Subtracts 1 from the number of items in the queue
        queuedNode = queue[currQueue]; // Sets the next node to check to the last item in the array (The one with the lowest weight)
        // printf("\nLowest cost:(%i, %i), from: (%i, %i), with: %i", queuedNode.nodeName.x, queuedNode.nodeName.y, queuedNode.startNode.x, queuedNode.startNode.y, queuedNode.currWeight);      //          (For checking)
        // printf("    Curr Queue: %i\n", currQueue);                                                                                //          (For checking)
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
    // printf("\nFinal: (%i, %i)", pathTravelled[paths].nodeName.x, pathTravelled[paths].nodeName.y);       //          (for checking)

    // Appends the routes to take to the finalPath array
    while (CheckIsPoint(finalPath[route - 1], startPoint) == 0) // While the last index in the finalPath array is not the starting node
    {
        // printf("Node now: %i", finalPath[route-1]);             //          (for checking)
        for (short i = paths; i >= 0; i--) // Iterates through the pathTravelled array backwards
        {
            // printf("Node name: %i", pathTravelled[i].nodeName);       //          (for checking)
            if (CheckIsPoint(pathTravelled[i].nodeName, finalPath[route - 1]) == 1) // If the name of the current iteration of the pathTravelled array matches the name of the last index of the finalPath array
            {
                // printf("add: %i\n", pathTravelled[i].nodeName);       //          (for checking)
                finalPath[route] = pathTravelled[i].startNode; // Appends the current pathTravelled value to the finalPath array
                route++;                                       // Increments the number of routes
                break;
            }
        }
    }

    // To print out the route to take
    printf("\n\nFinal Path: ");
    for (short i = route - 1; i >= 0; i--)
    {
        printf("(%i,%i), ", finalPath[i].x, finalPath[i].y);
    }

    // To print out the direction to take to next nodes
    printf("\nDirections: ");
    for (short i = route - 1; i > 0; i--)
    {
        CalcDirection(finalPath[i], finalPath[i - 1]);
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

void CalcDirection(struct Point currPoint, struct Point endPoint)
{
    if (currPoint.x > endPoint.x)
    {
        // go westW
        printf("WEST ");
    }
    else if (endPoint.x > currPoint.x)
    {
        // go east
        printf("EAST ");
    }
    else if (endPoint.y > currPoint.y)
    {
        // go south
        printf("SOUTH ");
    }
    else if (currPoint.y > endPoint.y)
    {
        // go north
        printf("NORTH ");
    }
    else
    {
        // stay
        printf("STAY ");
    }
}
