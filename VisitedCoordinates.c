#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define NORTHCHECK 7
#define EASTCHECK 11
#define SOUTHCHECK 13
#define WESTCHECK 14
#define ACCESSIBLE 15

// Function prototypes
void push();         // Push element to the top of the stack
struct Point pop();  // Remove and return the top most element of the stack
struct Point peek(); // Return the top most element of the stack
bool isEmpty();      // Check if the stack is in Underflow state or not
char checkDirection(char turn);
int nextTravelDir(short currX, short currY);

char compass = 'N'; // default North
short directionCount = 0;

// Scan and then U-Turn

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
    // change to int eg 0000
    short directions; // directions in binary in NESW eg 1111 1 = open, 0 = closed off
    short routes;
};

struct node *head;

struct GraphNode graphArray[5][4];

struct Point arr[20]; // 20 nodes

short visitedPoints = 0;

int main()
{
    short leftSensorDistance = 0;    // test
    short rightSensorDistance = 100; // test
    short forwardSensorDistance;

    char northInput;
    char eastInput;
    char southInput;
    char westInput;

    short direction = 0; // default 0 means closed off routes.

    short currX;
    short currY;

    // if y means flip to 1 (open route), else n is 0 (close route)
    printf("Enter North,East,South,West (y/n): ");
    scanf("%c%c%c%c", &northInput, &eastInput, &southInput, &westInput);
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

    if (northInput == 'y')
    {
        direction += 8;
    }

    printf("%d", direction);

    if (leftSensorDistance > rightSensorDistance)
    {
        // initialization
        struct Point point;
        point.x, point.y, currX, currY = 0;
        push(point);

        // U turn

        struct GraphNode a = {1, direction, direction};
        graphArray[point.x][point.y] = a;

        arr[visitedPoints] = point;
        visitedPoints++;
    }

    else if (leftSensorDistance > rightSensorDistance)
    {
        // initialization
        struct Point point;
        point.x, currX = 4;
        point.y, currY = 0;
        push(point);

        // U turn

        struct GraphNode a = {1, direction, direction};
        graphArray[point.x][point.y] = a;

        arr[visitedPoints] = point;
        visitedPoints++;
    }
}

void push(struct Point point)
{
    struct node *ptr = (struct node *)malloc(sizeof(struct node));

    if (head == NULL)
    {
        ptr->point = point;
        ptr->next = NULL;
        head = ptr;
    }
    else
    {
        ptr->point = point;
        ptr->next = head;
        head = ptr;
    }
}

struct Point pop()
{
    struct Point item;
    struct node *ptr;
    if (head == NULL)
        printf("Underflow State: can't remove any item");
    else
    {
        item = head->point;
        ptr = head;
        head = head->next;
        free(ptr);
        printf("%d,%d is popped out of the stack", item.x, item.y);
        return item;
    }
    struct Point point;
    point.x, point.y = -1;
    return point;
}

struct Point peek()
{
    struct Point x = head->point;
    printf("%d, %d is the top most element of the stack\n", x.x, x.y);
    return x;
}

bool isEmpty()
{
    if (head == NULL)
    {
        printf("Stack is empty: Underflow State\n");
        return true;
    }
    printf("Stack is not empty\n");
    return false;
}

int nextTravelDir(short currX, short currY)
{
    if ((graphArray[currX][currY].routes | NORTHCHECK) == ACCESSIBLE) // Checks if north node can be travelled to
    {
        // go north

        // how to turn

        // move forward 27cm

        // current y - 1

        // toggleNorth

        return 1;
    }

    if ((graphArray[currX][currY].routes | EASTCHECK) == ACCESSIBLE) // Checks if east node can be travelled to
    {
        // go east

        // how to turn

        // move forward 27cm

        // current x + 1

        return 2;
    }

    if ((graphArray[currX][currY].routes | SOUTHCHECK) == ACCESSIBLE) // Checks if south node can be travelled to
    {
        // go south

        // how to turn

        // move forward 27cm

        // current y + 1

        return 3;
    }

    if ((graphArray[currX][currY].routes | WESTCHECK) == ACCESSIBLE) // Checks if west node can be travelled to
    {
        // go west

        // how to turn

        // move forward 27cm

        // current x - 1

        return 4;

        //if routes are all zero
        //change direction(r)
        //change direction(r)
    }
}

char checkDirection(char turn)
{

    if (turn == 'L')
    {
        directionCount -= 1;
        if (directionCount == -1)
        {
            directionCount = 3;
        }
    }

    else if (turn == 'R')
    {
        directionCount += 1;
        if (directionCount == 4)
        {
            directionCount = 0;
        }
    }

    // Calculate the initial orientation of the car

    switch (directionCount)
    {
    case 0:
        compass = 'N';
        break;

    case 1:
        compass = 'E';
        break;
    case 2:
        compass = 'S';
        break;
    case 3:
        compass = 'W';

    default:
        break;
    }
}
// get the index of the direction

short getIndex(char direction)
{
    // get the index of the arr
    char directionArr[4] = {'N', 'E', 'S', 'W'};
    short index = -1;

    for (int i = 0; i < 4; i++)
    {
        if (directionArr[i] == direction)
        {
            index = i;
            break;
        }
    }
    return index;
}

// current direction takes in from checkDirection
bool changeDirection(char currentDirection, char intendedDirection)
{
    if (currentDirection == intendedDirection)
    {
        printf("STAY");
    }

    else if (getIndex(currentDirection) == getIndex(intendedDirection) - 1)
    {
        // turn right;
        checkDirection('R');
    }

    else if (getIndex(currentDirection) == getIndex(intendedDirection) + 1)
    {
        // turn left;
        checkDirection('L');
    }

    else if (getIndex(currentDirection) == getIndex(intendedDirection) + 2)
    {
        // u turn
        checkDirection('R');
        checkDirection('R');
        //getDirection
        //check through
    }
}

struct Point getNodeWithUnvisitedRoute(struct Point arr[20], struct GraphNode graphArray)
{

    for (int i = 0; i < 20; i++)
    {
        if (graphArray[arr[i].x][arr[i].y].routes != 0)
        {
            return arr[i].x, arr[i].y;
        }
    }
}

bool verifyNodeVistited(short currX, short currY)
{
    // 20 is the size of arr
    for (short i = 0; i < 20; i++)
    {
        if (arr[i].x == currX && arr[i].y == currY)
        {
            return true;
        }

        else
        {
            return false;
        }
    }
}

void updateGraphNode(short currX, short currY)
{
}