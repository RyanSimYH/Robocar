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
// int nextTravelDir(struct GraphNode** graphArray, short currX, short currY);

char direction = 'N'; // default North

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

struct node *head;

struct GraphNode
{
    short checked; // if -1,not checked, if 1, checked, if 3, check for weight to point in A star
    // change to int eg 0000
    short directions; // directions in binary in NESW eg 1111 1 = open, 0 = closed off
    short routes;     // these routes will keep changing
};

int main()
{

    short xPoints;
    short yPoints;

    xPoints = 5; // x points = (ultrasonic sensor distance in front + car length) /27
    yPoints = 4; // y points = (ultrasonic sensor longer distance L/R + car width) /27

    struct Point arr[20]; // 20 nodes
    struct GraphNode graphArray[xPoints][yPoints];

    short leftSensorDistance = 0;    // test
    short rightSensorDistance = 100; // test
    short forwardSensorDistance;

    char northInput;
    char eastInput;
    char southInput;
    char westInput;

    short direction = 0; // default 0 means closed off routes.
    short vistedPoints;

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
        //initialization
        struct Point point;
        point.x, point.y, currX, currY = 0;
        push(point);

        // U turn

        struct GraphNode a = {1, direction, direction};
        graphArray[point.x][point.y] = a;

        arr[vistedPoints] = point;
        vistedPoints++;
    }

    else if (leftSensorDistance > rightSensorDistance)
    {
         //initialization
        struct Point point;
        point.x, currX = 4;
        point.y, currY = 0;
        push(point);

        // U turn

        struct GraphNode a = {1, direction, direction};
        graphArray[point.x][point.y] = a;

        arr[vistedPoints] = point;
        vistedPoints++;
    }

    // printf("%i", nextTravelDir(graphArray[xPoints][yPoints], currX, currY));
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

// int nextTravelDir(struct GraphNode graphArray, short currX, short currY)
// {
//     if ((graphArray.routes | NORTHCHECK) == ACCESSIBLE) // Checks if north node can be travelled to
//     {
//         // go north
//         return 1;
//     }

//     if ((graphArray.routes | EASTCHECK) == ACCESSIBLE) // Checks if east node can be travelled to
//     {
//         // go east
//         return 2;
//     }

//     if ((graphArray.routes | SOUTHCHECK) == ACCESSIBLE) // Checks if south node can be travelled to
//     {
//         // go south
//         return 3;
//     }

//     if ((graphArray.routes | WESTCHECK) == ACCESSIBLE) // Checks if west node can be travelled to
//     {
//         // go west
//         return 4;
//     }
// }

char checkDirection(char turn)
{
    short count = 0;
    char direction;

    if (turn == 'L')
    {
        count -= 1;
        if (count == -1)
        {
            count = 3;
        }
    }

    else if (turn == 'T')
    {
        count += 1;
        if (count == 4)
        {
            count = 0;
        }
    }

    switch (count)
    {
    case 0:
        direction = 'N';
        break;

    case 1:
        direction = 'E';
        break;
    case 2:
        direction = 'S';
        break;
    case 3:
        direction = 'W';

    default:
        break;
    }
}
