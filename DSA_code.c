#include <stdio.h>
#include <stdlib.h>

#define QUEUE_SIZE 10
#define MAX_ROUTERS 20
#define MAX_PACKETS 100
#define INF 1000000

struct Packet {
    int id;
    int source;
    int destination;
    int size;
};

struct Router {
    int id;
    char name;

    struct Packet queue[QUEUE_SIZE];

    int front;
    int rear;
};

struct Router routers[MAX_ROUTERS];
struct Packet packetList[MAX_PACKETS];

int Network[MAX_ROUTERS][MAX_ROUTERS];

int Router_Count = 0;
int Packet_Count = 0;


/* =========================================
   INITIALIZE NETWORK
   ========================================= */

void Initialising_Network() {

    int i, j;

    for (i = 0; i < MAX_ROUTERS; i++) {

        for (j = 0; j < MAX_ROUTERS; j++) {

            if (i == j)
                Network[i][j] = 0;
            else
                Network[i][j] = -1;
        }
    }
}


/* =========================================
   CREATE ROUTER
   ========================================= */

void Creating_Router() {

    if (Router_Count >= MAX_ROUTERS) {

        printf("\nRouter limit reached.\n");
        return;
    }

    routers[Router_Count].id = Router_Count;

    routers[Router_Count].name = 'A' + Router_Count;

    routers[Router_Count].front = 0;

    routers[Router_Count].rear = -1;

    printf("\nRouter %c created.\n",
           routers[Router_Count].name);

    Router_Count++;
}


/* =========================================
   DISPLAY ROUTERS
   ========================================= */

void Display_Routers() {

    int i;
    int count;

    if (Router_Count == 0) {

        printf("\nNo routers available.\n");
        return;
    }

    printf("\n--------- ROUTERS ---------\n");

    for (i = 0; i < Router_Count; i++) {

        count = routers[i].rear -
                routers[i].front + 1;

        if (count < 0)
            count = 0;

        printf("ID: %d   Name: %c   Queue: %d/%d\n",
               routers[i].id,
               routers[i].name,
               count,
               QUEUE_SIZE);
    }
}


/* =========================================
   ADD CONNECTION
   ========================================= */

void Add_Connection() {

    int source;
    int destination;
    int cost;

    if (Router_Count < 2) {

        printf("\nCreate at least two routers first.\n");
        return;
    }

    printf("\nAvailable routers:\n");

    for (int i = 0; i < Router_Count; i++) {

        printf("%d -> %c\n",
               routers[i].id,
               routers[i].name);
    }

    printf("\nEnter source router ID: ");
    scanf("%d", &source);

    printf("Enter destination router ID: ");
    scanf("%d", &destination);

    if (source < 0 ||
        source >= Router_Count ||
        destination < 0 ||
        destination >= Router_Count) {

        printf("Invalid router ID.\n");
        return;
    }

    if (source == destination) {

        printf("A router cannot connect to itself.\n");
        return;
    }

    printf("Enter connection cost: ");
    scanf("%d", &cost);

    if (cost <= 0) {

        printf("Cost must be positive.\n");
        return;
    }

    Network[source][destination] = cost;

    Network[destination][source] = cost;

    printf("\nConnection added: %c <-> %c\n",
           routers[source].name,
           routers[destination].name);
}


/* =========================================
   DISPLAY NETWORK
   ========================================= */

void Display_Network() {

    int i, j;

    if (Router_Count == 0) {

        printf("\nNo routers exist.\n");
        return;
    }

    printf("\n--------- NETWORK ---------\n\n");

    printf("     ");

    for (i = 0; i < Router_Count; i++)
        printf("%5c", routers[i].name);

    printf("\n");

    for (i = 0; i < Router_Count; i++) {

        printf("%3c  ", routers[i].name);

        for (j = 0; j < Router_Count; j++) {

            if (Network[i][j] == -1)
                printf("%5s", "-");
            else
                printf("%5d", Network[i][j]);
        }

        printf("\n");
    }
}


/* =========================================
   ENQUEUE PACKET
   ========================================= */

int Enqueue_Packet(int routerID, struct Packet p) {

    if (routers[routerID].rear >= QUEUE_SIZE - 1) {

        printf("\nQueue of Router %c is full.\n",
               routers[routerID].name);

        return 0;
    }

    routers[routerID].rear++;

    routers[routerID].queue[
        routers[routerID].rear
    ] = p;

    return 1;
}


/* =========================================
   DEQUEUE PACKET
   ========================================= */

struct Packet Dequeue_Packet(int routerID) {

    struct Packet emptyPacket = {-1, -1, -1, 0};

    if (routers[routerID].front >
        routers[routerID].rear) {

        return emptyPacket;
    }

    struct Packet p =
        routers[routerID].queue[
            routers[routerID].front
        ];

    routers[routerID].front++;

    return p;
}


/* =========================================
   CREATE PACKET
   ========================================= */

void Creating_Packet() {

    struct Packet p;

    if (Packet_Count >= MAX_PACKETS) {

        printf("\nPacket limit reached.\n");
        return;
    }

    if (Router_Count < 2) {

        printf("\nCreate routers first.\n");
        return;
    }

    p.id = Packet_Count + 1;

    printf("\nEnter source router ID: ");
    scanf("%d", &p.source);

    printf("Enter destination router ID: ");
    scanf("%d", &p.destination);

    printf("Enter packet size: ");
    scanf("%d", &p.size);

    if (p.source < 0 ||
        p.source >= Router_Count ||
        p.destination < 0 ||
        p.destination >= Router_Count) {

        printf("Invalid router ID.\n");
        return;
    }

    if (p.source == p.destination) {

        printf("Source and destination cannot be same.\n");
        return;
    }

    if (p.size <= 0) {

        printf("Packet size must be positive.\n");
        return;
    }

    if (Enqueue_Packet(p.source,p )) {

        packetList[Packet_Count] = p;

        Packet_Count++;

        printf("\nPacket %d created successfully.\n",
               p.id);
    }
}


/* =========================================
   FIND MINIMUM FOR DIJKSTRA
   ========================================= */

int Find_Minimum(int distance[],
                 int visited[]) {

    int minimum = INF;

    int position = -1;

    for (int i = 0; i < Router_Count; i++) {

        if (visited[i] == 0 &&
            distance[i] < minimum) {

            minimum = distance[i];

            position = i;
        }
    }

    return position;
}


/* =========================================
   DIJKSTRA ALGORITHM
   ========================================= */

void Dijkstra(int source,
              int destination,
              int parent[],
              int distance[]) {

    int visited[MAX_ROUTERS];

    for (int i = 0; i < Router_Count; i++) {

        distance[i] = INF;

        visited[i] = 0;

        parent[i] = -1;
    }

    distance[source] = 0;

    for (int step = 0;
         step < Router_Count;
         step++) {

        int current =
            Find_Minimum(distance, visited);

        if (current == -1)
            break;

        visited[current] = 1;

        for (int next = 0;
             next < Router_Count;
             next++) {

            if (Network[current][next] != -1 &&
                visited[next] == 0) {

                int newDistance =
                    distance[current] +
                    Network[current][next];

                if (newDistance < distance[next]) {

                    distance[next] = newDistance;

                    parent[next] = current;
                }
            }
        }
    }
}


/* =========================================
   DISPLAY PATH
   ========================================= */

void Display_Path(int source,
                  int destination,
                  int parent[]) {

    int path[MAX_ROUTERS];

    int count = 0;

    int current = destination;

    while (current != -1) {

        path[count] = current;

        count++;

        if (current == source)
            break;

        current = parent[current];
    }

    if (path[count - 1] != source) {

        printf("\nNo route exists.\n");
        return;
    }

    printf("\nShortest path: ");

    for (int i = count - 1;
         i >= 0;
         i--) {

        printf("%c",
               routers[path[i]].name);

        if (i != 0)
            printf(" -> ");
    }
}


/* =========================================
   ROUTE PACKET
   ========================================= */

void Route_Packet() {

    int packetID;

    int parent[MAX_ROUTERS];

    int distance[MAX_ROUTERS];

    printf("\nEnter packet ID: ");
    scanf("%d", &packetID);

    if (packetID <= 0 ||
        packetID > Packet_Count) {

        printf("Invalid packet ID.\n");
        return;
    }

    struct Packet p =
        packetList[packetID - 1];

    Dijkstra(
        p.source,
        p.destination,
        parent,
        distance
    );

    if (distance[p.destination] == INF) {

        printf("\nPacket cannot be delivered.");

        printf("\nNo path between %c and %c.\n",
               routers[p.source].name,
               routers[p.destination].name);

        return;
    }

    Display_Path(
        p.source,
        p.destination,
        parent
    );

    printf("\nTotal route cost: %d\n",
           distance[p.destination]);

    printf("Packet %d delivered.\n",
           p.id);

    Dequeue_Packet(p.source);
}


/* =========================================
   DISPLAY QUEUES
   ========================================= */

void Display_Queues() {

    int i, j;

    printf("\n--------- ROUTER QUEUES ---------\n");

    for (i = 0; i < Router_Count; i++) {

        printf("\nRouter %c: ",
               routers[i].name);

        if (routers[i].front >
            routers[i].rear) {

            printf("Empty");
        }
        else {

            for (j = routers[i].front;
                 j <= routers[i].rear;
                 j++) {

                printf("[P%d] ",
                       routers[i].queue[j].id);
            }
        }

        printf("\n");
    }
}


/* =========================================
   BASIC CONGESTION CHECK
   ========================================= */

void Check_Congestion() {

    int count;

    printf("\n------- CONGESTION STATUS -------\n");

    for (int i = 0;
         i < Router_Count;
         i++) {

        count =
            routers[i].rear -
            routers[i].front + 1;

        if (count < 0)
            count = 0;

        printf("Router %c : %d/%d packets -> ",
               routers[i].name,
               count,
               QUEUE_SIZE);

        if (count >= QUEUE_SIZE * 0.8)

            printf("CONGESTED\n");

        else if (count >= QUEUE_SIZE * 0.5)

            printf("MODERATE\n");

        else

            printf("NORMAL\n");
    }
}


/* =========================================
   DISPLAY PACKETS
   ========================================= */

void Display_Packets() {

    if (Packet_Count == 0) {

        printf("\nNo packets created.\n");
        return;
    }

    printf("\n--------- PACKETS ---------\n");

    for (int i = 0;
         i < Packet_Count;
         i++) {

        printf("\nPacket ID    : %d",
               packetList[i].id);

        printf("\nSource       : %c",
               routers[
                   packetList[i].source
               ].name);

        printf("\nDestination  : %c",
               routers[
                   packetList[i].destination
               ].name);

        printf("\nSize         : %d bytes\n",
               packetList[i].size);
    }
}


/* =========================================
   GENERATE DATA FOR AI
   ========================================= */

void Generate_Traffic_Data() {

    FILE *fp;

    int count;

    fp = fopen("network_data.csv", "w");

    if (fp == NULL) {

        printf("\nUnable to create CSV file.\n");
        return;
    }

    fprintf(fp,
            "router,packets,capacity,traffic\n");

    for (int i = 0;
         i < Router_Count;
         i++) {

        count =
            routers[i].rear -
            routers[i].front + 1;

        if (count < 0)
            count = 0;

        int traffic =
            (count * 100) / QUEUE_SIZE;

        fprintf(fp,
                "%c,%d,%d,%d\n",
                routers[i].name,
                count,
                QUEUE_SIZE,
                traffic);
    }

    fclose(fp);

    printf("\nTraffic data saved to network_data.csv\n");
}


/* =========================================
   RUN PYTHON AI
   ========================================= */

void Run_AI_Prediction() {

    Generate_Traffic_Data();

    printf("\nStarting AI congestion prediction...\n");

    system("python prediction.py");
}


/* =========================================
   MAIN
   ========================================= */

int main() {

    int choice;

    Initialising_Network();

    while (1) {

        printf("\n\n====================================");

        printf("\n   INTELLIGENT NETWORK ROUTING");

        printf("\n====================================");

        printf("\n1. Create Router");

        printf("\n2. Display Routers");

        printf("\n3. Add Network Connection");

        printf("\n4. Display Network");

        printf("\n5. Create Packet");

        printf("\n6. Display Packets");

        printf("\n7. Display Router Queues");

        printf("\n8. Route Packet");

        printf("\n9. Check Current Congestion");

        printf("\n10. Run AI Congestion Prediction");

        printf("\n11. Exit");

        printf("\n\nEnter your choice: ");

        scanf("%d", &choice);


        switch (choice) {

            case 1:
                Creating_Router();
                break;

            case 2:
                Display_Routers();
                break;

            case 3:
                Add_Connection();
                break;

            case 4:
                Display_Network();
                break;

            case 5:
                Creating_Packet();
                break;

            case 6:
                Display_Packets();
                break;

            case 7:
                Display_Queues();
                break;

            case 8:
                Route_Packet();
                break;

            case 9:
                Check_Congestion();
                break;

            case 10:
                Run_AI_Prediction();
                break;

            case 11:
                printf("\nProgram ended.\n");
                return 0;

            default:
                printf("\nInvalid choice.\n");
        }
    }

    return 0;
}              
