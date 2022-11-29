#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define maxsize 4
#define maxarraysize 25
#define NORTHCHECK 7
#define EASTCHECK 11
#define SOUTHCHECK 13
#define WESTCHECK 14
#define ACCESSIBLE 15

#define NORTHTOGGLE 8
#define EASTTOGGLE 4
#define SOUTHTOGGLE 2
#define WESTTOGGLE 1

struct Point
{
    short x;
    short y;
};

struct node
{
    struct Point point;
    struct node *next;
};

struct GraphNode
{
    short checked; // if -1,not checked, if 1, checked, if 3, check for weight to point in A star
    short directions; // directions in binary in NESW eg 1111 1 = open, 0 = closed off
    short routes;   //verify routes that are open and close 
};

struct QueueItem
{
    struct Point nodeName;
    struct Point startNode;
    short currWeight;
    short edgeWeight;
};


// Function prototypes
void push();         // Push element to the top of the stack
struct Point pop();  // Remove and return the top most element of the stack
struct Point peek(); // Return the top most element of the stack
bool isEmpty();      // Check if the stack is in Underflow state or not
char checkDirection(char turn);
int nextTravelDir(short currX, short currY);
void HowToTurn(char currDir, char intendedDir);
short getDirIndex(char direction);
struct Point getNodeWithUnvisitedRoute();
bool verifyNodeVistited(short currX, short currY);

void AStar(short currX, short currY, struct Point endPoint);
short CheckIsPoint(struct Point currPoint, struct Point endPoint);
void SortQueue(struct QueueItem queue[], short end);
void Swapper(struct QueueItem queue[], short currIndex);
short CheckIfBlocked();
int GetXPoint();
int GetYPoint();

char compass = 'N'; // default North
short directionCount = 0;

char directionString[15]  = {'<','v','J','>','=','.','m','^','`','L','3','l','w','E','+'};
char routeDirString[20];

short xPoints = 5;
short yPoints = 4;
struct GraphNode graphArray[5][4];

struct Point arr[20]; // 20 nodes

short visitedPoints = 0;
struct Point startpoint;

short currX;
short currY;

int main()
{
    short leftSensorDistance = 100;    // test
    short rightSensorDistance = 0; // test
    short forwardSensorDistance;

    char northInput;
    char eastInput;
    char southInput;
    char westInput;

    short direction = 0; // default 0 means closed off routes.
    short routes = 0;

    // if y means flip to 1 (open route), else n is 0 (close route)
    printf("Enter East,South,West (y/n): ");
    scanf("%c%c%c", &eastInput, &southInput, &westInput);
    fflush(stdin);

    if (westInput == 'y')
    {
        direction++;
    }

    if (southInput == 'y')
    {
        direction += 2;
    }

    if (eastInput == 'y')
    {
        direction += 4;
    }

    //printf("%d", direction);

    if (leftSensorDistance > rightSensorDistance)
    {
        // initialization
        // struct Point point;
        startpoint.x, startpoint.y, currX, currY = 0;

        // U turn

        struct GraphNode a = {1, direction, direction};
        graphArray[startpoint.x][startpoint.y] = a;

        arr[visitedPoints] = startpoint;
    }

    else if (leftSensorDistance < rightSensorDistance)
    {
        // initialization
        // struct Point point;
        startpoint.x = 4;
        currX = 4;
        startpoint.y =0;
        currY = 0;
        // U turn

        struct GraphNode a = {1, direction, direction};
        graphArray[startpoint.x][startpoint.y] = a;

        arr[visitedPoints] = startpoint;
    }

    printf("StartPoint: %d, %d\n",startpoint.x, startpoint.y);
    if (nextTravelDir(startpoint.x, startpoint.y))
    {
        visitedPoints++;
    }

    while (visitedPoints < 20)
    {
        printf("\nVisited points: %d",visitedPoints);
        printf("\nNext point to travel to: (%d, %d): ", currX, currY);
        // for (short i = 0; i < 20; i++)
        // {
        //    printf("(%d, %d)  ", arr[i].x, arr[i].y);        
        // }
            printf("\n");
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
     printf("\n");
        if (verifyNodeVistited(currX, currY) == true)
        {
            if (graphArray[currX][currY].routes != 0)
            {
                nextTravelDir(currX, currY);
            }
            else
            {
                printf("routes: %d", graphArray[currX][currY].routes);
                struct Point endpoint = getNodeWithUnvisitedRoute();
                printf("\nA STAR HERE");
                AStar(currX,currY,endpoint);
                currX = endpoint.x;
                currY = endpoint.y;
            }
        }
        else
        {
            direction = 0;
            routes = 0;
            if (compass == 'N')
            {
                // if y means flip to 1 (open route), else n is 0 (close route)
                printf("Enter North (y/n): ");
                scanf("%c", &northInput);
                fflush(stdin);
                printf("Enter East (y/n): ");
                scanf("%c", &eastInput);
                fflush(stdin);
                printf("Enter West (y/n): ");
                scanf("%c", &westInput);
                fflush(stdin);

                if (westInput == 'y')
                {
                    direction++;
                    routes++;
                }

                if (eastInput == 'y')
                {
                    direction += 4;
                    routes += 4;
                }

                if (northInput == 'y')
                {
                    direction += 8;
                    routes += 8;
                }

                direction += 2;
            }

            else if (compass == 'E')
            {
                // if y means flip to 1 (open route), else n is 0 (close route)
                printf("Enter North (y/n): ");
                scanf("%c", &northInput);
                fflush(stdin);
                printf("Enter East (y/n): ");
                scanf("%c", &eastInput);
                fflush(stdin);
                printf("Enter South (y/n): ");
                scanf("%c", &southInput);
                fflush(stdin);

                if (southInput == 'y')
                {
                    direction += 2;
                    routes += 2;
                }

                if (eastInput == 'y')
                {
                    direction += 4;
                    routes += 4;
                }

                if (northInput == 'y')
                {
                    direction += 8;
                    routes += 8;
                }

                direction++;
            }

            else if (compass == 'S')
            {
                // if y means flip to 1 (open route), else n is 0 (close route)
                printf("Enter East (y/n): ");
                scanf("%c", &eastInput);
                fflush(stdin);
                printf("Enter South (y/n): ");
                scanf("%c", &southInput);
                fflush(stdin);
                printf("Enter West (y/n): ");
                scanf("%c", &westInput);
                fflush(stdin);

                if (westInput == 'y')
                {
                    direction++;
                    routes++;
                }

                if (southInput == 'y')
                {
                    direction += 2;
                    routes += 2;
                }

                if (eastInput == 'y')
                {
                    direction += 4;
                    routes += 4;
                }

                direction += 8;
            }

            else if (compass == 'W')
            {
                // if y means flip to 1 (open route), else n is 0 (close route)
                printf("Enter North (y/n): ");
                scanf("%c", &northInput);
                fflush(stdin);
                printf("Enter South (y/n): ");
                scanf("%c", &southInput);
                fflush(stdin);
                printf("Enter West (y/n): ");
                scanf("%c", &westInput);
                fflush(stdin);

                if (westInput == 'y')
                {
                    direction++;
                    routes++;
                }

                if (southInput == 'y')
                {
                    direction += 2;
                    routes += 2;
                }

                if (northInput == 'y')
                {
                    direction += 8;
                    routes += 8;
                }

                direction += 4;
            }

            struct GraphNode a = {1, direction, routes};
            graphArray[currX][currY] = a;
            struct Point currPoint = {currX, currY};
            arr[visitedPoints] = currPoint;
            if (nextTravelDir(currX, currY))
            {
                visitedPoints++;
            }
        }
    }

    // raise and send mapping finished interrupt to comms
    // receive end point from comms
    printf("\n");
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

    short counter = 0;
    printf("\n");
    for (short i = 0; i < yPoints; i++)
    {
        
        for (short j = 0; j < xPoints; j++)
        {
            routeDirString[counter] = directionString[graphArray[j][i].directions - 1];
            counter++;
        }
    }
    printf("\n");
    for (short i = 0; i < 20; i++)
    {
        printf("%c",routeDirString[i]);
    }
}

int nextTravelDir(short xPoint, short yPoint)
{
    if ((graphArray[xPoint][yPoint].routes | NORTHCHECK) == ACCESSIBLE) // Checks if north node can be travelled to
    {
        //Remove north from routes in struct
        graphArray[xPoint][yPoint].routes -= NORTHTOGGLE;
        HowToTurn(compass,'N');
        printf("NORTH\n");
        // move forward 27cm

        currY-=1;
        return 1;
    }

    if ((graphArray[xPoint][yPoint].routes | EASTCHECK) == ACCESSIBLE) // Checks if east node can be travelled to
    {
        // go east
        //Remove east from routes in struct
        graphArray[xPoint][yPoint].routes -= EASTTOGGLE;
        HowToTurn(compass,'E');
        printf("EAST\n");
        // move forward 27cm

        currX +=1;
        return 2;
    }

    if ((graphArray[xPoint][yPoint].routes | SOUTHCHECK) == ACCESSIBLE) // Checks if south node can be travelled to
    {
        // go south
        //Remove south from routes in struct
        graphArray[xPoint][yPoint].routes -= SOUTHTOGGLE;
        HowToTurn(compass,'S');
        printf("SOUTH\n");
        // move forward 27cm


        currY +=1;
        return 3;
    }

    if ((graphArray[xPoint][yPoint].routes | WESTCHECK) == ACCESSIBLE) // Checks if west node can be travelled to
    {
        // go west
        //Remove west from routes in struct
        graphArray[xPoint][yPoint].routes -= WESTTOGGLE;
        HowToTurn(compass,'W');
        printf("WEST\n");
        // move forward 27cm

        currX-=1;
        return 4;
    }

    if (graphArray[xPoint][yPoint].routes == 0) // Checks if west node can be travelled to
    {
        // u-turn
        // change direction(r)
        // change direction(r)

        // move forward 27cm

        // currDir if N (y-1), E(x+1), S(y+1), W(x-1)

        return 4;
    }
}

// current direction takes in from checkDirection
void HowToTurn(char currDir, char intendedDir)
{
    short dirToTurn = (getDirIndex(intendedDir)-getDirIndex(currDir));
    //printf("%d", dirToTurn);
    compass = intendedDir;

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

// get the index of the direction
short getDirIndex(char direction)
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
        // u turn
        return 3;
        // getDirection
        // check through
    }
}

struct Point getNodeWithUnvisitedRoute()
{

    for (int i = 0; i < 20; i++)
    {
        if (graphArray[arr[i].x][arr[i].y].routes != 0)
        {
            return arr[i];
        }
    }
}

bool verifyNodeVistited(short currX, short currY)
{
    //printf("Verifying: (%d, %d)", currX,currY);
    // 20 is the size of arr
    for (short i = 0; i < visitedPoints; i++)
    {
        if (arr[i].x == currX && arr[i].y == currY)
        {
            //printf("true");
            return true;
        }
    }
    //printf("false");
    return false;
}

void AStar(short currX, short currY, struct Point endPoint){
    printf("\nStart point:(%d,%d)   End Point: (%d,%d)", currX, currY, endPoint.x, endPoint.y);
    short currQueue = 0;                        // The number of items in the queue
    short paths = 0;                            // Number of paths travelled
    struct QueueItem queuedNode = {0, 0, 0, 0}; // The node that needs to be checked


    queuedNode.nodeName.x = currX;
    queuedNode.nodeName.y = currY;
    struct Point pointToAdd = {0,0};
    struct Point neighbourNode[maxsize];                 //Array to hold the neighbours of the node
    short neighbours = 0;                         //Number of neighbours in the node
    struct QueueItem queue[maxarraysize];              //Initialise the queue array with the QueueItem struct, this will hold the queue for the paths to travel
    struct QueueItem pathTravelled[maxarraysize];      //Initialise the pathTravelled array with the QueueItem struct, this will hold the QueueItems of the paths that have been popped from the queue
    graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].checked = 3;  

    while (CheckIsPoint(queuedNode.nodeName, endPoint) == 0) // If the current node to check is not the goal node
    {
        if ((graphArray[queuedNode.nodeName.x][queuedNode.nodeName.y].directions | NORTHCHECK) == ACCESSIBLE) // Checks if north node can be travelled to
        {
            
            pointToAdd.x = queuedNode.nodeName.x;
            pointToAdd.y = queuedNode.nodeName.y - 1;
            if (graphArray[pointToAdd.x][pointToAdd.y].checked != 3) // Checks if north node has been checked before
            {
                neighbourNode[neighbours] = pointToAdd;             // Add the north node of the current point to the neighbours array
                neighbours++;                                       // Increment the number of neighbours
                graphArray[pointToAdd.x][pointToAdd.y].checked = 3; // Set the current point as checked as it will be checked for its weight, prevents point from being checked again
              //  printf("\nAdd neighbour north: (%i, %i)", pointToAdd.x, pointToAdd.y);
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
                //printf("\nAdd neighbour east: (%i, %i)", pointToAdd.x, pointToAdd.y);
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
                //printf("\nAdd neighbour south: (%i, %i)", pointToAdd.x, pointToAdd.y);
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
                //printf("\nAdd neighbour west: (%i, %i)\n", pointToAdd.x, pointToAdd.y);
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
        // for (short i = paths - 1; i >= 0; i--)
        // {
        //     printf("\nPath %i, travelled: (%i, %i) from: (%i, %i)", i, pathTravelled[i].nodeName.x, pathTravelled[i].nodeName.y, pathTravelled[i].startNode.x, pathTravelled[i].startNode.y);
        // }
    }
    
    // After the goal has been reached
    struct Point finalPath[paths];                // Initialise an array for the final path to take
    paths -= 1;                                   // Subtract 1 from the number of paths
    short route = 1;                              // Initialise the number of routes
    finalPath[0] = pathTravelled[paths].nodeName; // Set the first index of the finalPath array to the last item in the pathTravelled array
    
    // Appends the routes to take to the finalPath array
   // while (CheckIsPoint(finalPath[route - 1], startpoint) == 0) // While the last index in the finalPath array is not the starting node
    //{
        for (short i = paths; i >= 0; i--) // Iterates through the pathTravelled array backwards
        {
            if (CheckIsPoint(pathTravelled[i].nodeName, finalPath[route - 1]) == 1) // If the name of the current iteration of the pathTravelled array matches the name of the last index of the finalPath array
            {
                finalPath[route] = pathTravelled[i].startNode; // Appends the current pathTravelled value to the finalPath array
                route++;                                       // Increments the number of routes
               // break;
            }
        }
   // }


    // To print out the route to take
    printf("\n\nFinal Path: ");
    for (short i = route - 1; i > 0; i--)
    {
        printf("(%i,%i), ", finalPath[i].x, finalPath[i].y);
 
        if(finalPath[i].x > finalPath[i-1].x)
        {
            //go west
            HowToTurn(compass,'W');
        }
        else if (finalPath[i-1].x > finalPath[i].x) 
        {
            //go east
            HowToTurn(compass,'E');
        }   
        else if (finalPath[i-1].y > finalPath[i].y) 
        {
            //go south
            HowToTurn(compass,'S');
        }
        else if (finalPath[i].y > finalPath[i-1].y) 
        {
            //go north
            HowToTurn(compass,'N');
        }

    }

    printf("(%i,%i), ", finalPath[0].x, finalPath[0].y);


    //resets the checked variable of graphArray
    for (short i = 0; i < yPoints; i++)
    {
        printf("\n");
        for (short j = 0; j < xPoints; j++)
        {
            graphArray[j][i].checked = 1;

        }
    }
}

short CheckIsPoint(struct Point currPoint, struct Point endPoint)   //Check if the current point is equal to the endpoint
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
    struct QueueItem temp = queue[currIndex]; // Saves the value of the unsorted item into temp
    queue[currIndex] = queue[currIndex - 1];
    queue[currIndex - 1] = temp;
}

short CheckIfBlocked()
{
    //take in ultrasonic

}

int GetXPoint()
{
    return currX;
}

int GetYPoint()
{
    return currY;
}