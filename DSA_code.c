#include <stdio.h>
#include <stdlib.h>

#define Queue_size 10
#define Routers 20
#define Packets 100
#define History_Length 5

/* Command used to launch the Python AI module.
   Windows normally uses "python" (or "py"); Linux/macOS use "python3". */
#ifdef _WIN32
    #define PYTHON_CMD "python predict_congestion.py"
#else
    #define PYTHON_CMD "python3 predict_congestion.py"
#endif

struct Packet {
    int id;
    int source;
    int destination;
    int size;
};

struct Router {
    int id;
    char name;
    struct Packet queue[Queue_size];
    int front;
    int rear;
};

struct Router routers[Routers];
struct Packet packetList[Packets];

int Network[Routers][Routers];

int Router_Count = 0;
int Packet_Count = 0;

int Congestion_History[Routers][History_Length];
int History_Count[Routers];
int History_Next[Routers];

void Initialising_Network() {
    int i, j;
    for(i = 0; i < Routers; i++) {
        for(j = 0; j < Routers; j++) {
            if(i == j)
                Network[i][j] = 0;
            else
                Network[i][j] = -1;
        }
        History_Count[i] = 0;
        History_Next[i] = 0;
    }
}

void Creating_router() {
    if(Router_Count >= Routers) {
        printf("\nRouter limit reached.\n");
        return;
    }
    routers[Router_Count].id = Router_Count;
    routers[Router_Count].name = 'A' + Router_Count;
    routers[Router_Count].front = 0;
    routers[Router_Count].rear = -1;
    printf("\nRouter %c created.\n",routers[Router_Count].name);
    Router_Count++;
}

void Display_Routers() {
    int i;
    if(Router_Count == 0) {
        printf("\nNo routers available.\n");
        return;
    }
    printf("\n----- ROUTERS -----\n");
    for(i = 0; i < Router_Count; i++) {
        printf("ID: %d   Name: %c   Queue: %d\n",routers[i].id,routers[i].name,routers[i].rear - routers[i].front + 1);
    }
}

void Add_Connection() {
    int source;
    int destination;
    int cost;

    if(Router_Count < 2) {
        printf("\nCreate at least two routers first.\n");
        return;
    }
    printf("\nAvailable routers:\n");
    for(int i = 0; i < Router_Count; i++) {
        printf("%d -> %c\n",routers[i].id,routers[i].name);
    }
    printf("\nEnter source router ID: ");
    scanf("%d", &source);
    printf("Enter destination router ID: ");
    scanf("%d", &destination);

    if(source < 0 || source >= Router_Count || destination < 0 || destination >= Router_Count) {
        printf("Invalid router ID.\n");
        return;
    }

    if(source == destination) {
        printf("A router cannot connect to itself.\n");
        return;
    }
    printf("Enter connection cost: ");
    scanf("%d", &cost);
    if(cost <= 0) {
        printf("Cost must be positive.\n");
        return;
    }

    Network[source][destination] = cost;
    Network[destination][source] = cost;

    printf("\nConnection added: %c <-> %c\n",routers[source].name,routers[destination].name);
}

void Display_Network() {
    int i, j;

    if(Router_Count == 0) {
        printf("\nNo routers exist.\n");
        return;
    }
    printf("\n--------- NETWORK ---------\n\n");
    printf("     ");
    for(i = 0; i < Router_Count; i++) {
        printf("%5c", routers[i].name);
    }
    printf("\n");

    for(i = 0; i < Router_Count; i++) {
        printf("%3c  ", routers[i].name);
        for(j = 0; j < Router_Count; j++){
            if(Network[i][j] == -1)
                printf("%5s", "-");
            else
                printf("%5d", Network[i][j]);
        }
        printf("\n");
    }
}

int Enqueue_Packet(int routerID, struct Packet p) {
    if(routers[routerID].rear >= Queue_size - 1) {
        printf("\nQueue of Router %c is full.\n", routers[routerID].name);
        return 0;
    }
    routers[routerID].rear++;
    routers[routerID].queue[
        routers[routerID].rear
    ] = p;
    return 1;
}

/* Resets front/rear once the queue drains completely, so a router's
   buffer can be reused instead of permanently "filling up". */
struct Packet Dequeue_Packet(int routerID) {
    struct Packet emptyPacket = {-1, -1, -1, 0};
    if(routers[routerID].front > routers[routerID].rear) {
        return emptyPacket;
    }
    struct Packet p =
        routers[routerID].queue[
            routers[routerID].front
        ];

    routers[routerID].front++;

    if(routers[routerID].front > routers[routerID].rear) {
        routers[routerID].front = 0;
        routers[routerID].rear = -1;
    }

    return p;
}

void Creating_Packet() {
    struct Packet p;
    if(Packet_Count >= Packets) {
        printf("\nPacket limit reached.\n");
        return;
    }
    if(Router_Count < 2) {
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
    if(p.source < 0 || p.source >= Router_Count || p.destination < 0 || p.destination >= Router_Count) {
        printf("Invalid router ID.\n");
        return;
    }
    if(p.source == p.destination) {
        printf("Source and destination cannot be same.\n");
        return;
    }
    if(p.size <= 0) {
        printf("Packet size must be positive.\n");
        return;
    }
    packetList[Packet_Count] = p;
    if(Enqueue_Packet(p.source, p)) {
        Packet_Count++;
        printf("\nPacket %d created successfully.\n", p.id);
    }
}

int Find_Minimum(int distance[], int visited[]) {
    int minimum = 1000000;
    int position = -1;
    for(int i = 0; i < Router_Count; i++) {
        if(visited[i] == 0 && distance[i] < minimum) {
            minimum = distance[i];
            position = i;
        }
    }
    return position;
}

void Dijkstra(int source, int parent[], int distance[]) {
    int visited[Routers];
    for(int i = 0; i < Router_Count; i++) {
        distance[i] = 1000000;
        visited[i] = 0;
        parent[i] = -1;
    }
    distance[source] = 0;
    for(int step = 0; step < Router_Count; step++) {
        int current = Find_Minimum(distance, visited);
        if(current == -1) {
            break;
        }
        visited[current] = 1;
        for(int next = 0; next < Router_Count; next++) {
            if(Network[current][next] != -1 && visited[next] == 0) {
                int newDistance = distance[current] + Network[current][next];
                if(newDistance < distance[next]) {
                    distance[next] = newDistance;
                    parent[next] = current;
                }
            }
        }
    }
}

/* Builds the path as a forward array (source -> ... -> destination). */
int Build_Path(int source, int destination, int parent[], int path[]) {
    int reverse[Routers];
    int count = 0;
    int current = destination;

    while(current != -1) {
        reverse[count] = current;
        count++;
        if(current == source) {
            break;
        }
        current = parent[current];
    }

    if(reverse[count - 1] != source) {
        return 0;
    }

    for(int i = 0; i < count; i++) {
        path[i] = reverse[count - 1 - i];
    }

    return count;
}

void Route_Packet() {
    int packetID;
    int parent[Routers];
    int distance[Routers];
    int path[Routers];

    printf("\nEnter packet ID: ");
    scanf("%d", &packetID);
    if(packetID <= 0 || packetID > Packet_Count) {
        printf("Invalid packet ID.\n");
        return;
    }

    struct Packet p = packetList[packetID - 1];
    Dijkstra(p.source, parent, distance);

    if(distance[p.destination] == 1000000) {
        printf("\nPacket cannot be delivered.");
        printf("\nNo path between %c and %c.\n", routers[p.source].name, routers[p.destination].name);
        return;
    }

    int pathLength = Build_Path(p.source, p.destination, parent, path);

    printf("\nShortest path: ");
    for(int i = 0; i < pathLength; i++) {
        printf("%c", routers[path[i]].name);
        if(i != pathLength - 1) {
            printf(" -> ");
        }
    }
    printf("\n");

    Dequeue_Packet(p.source);

    for(int hop = 1; hop < pathLength; hop++) {
        if(!Enqueue_Packet(path[hop], p)) {
            printf("Packet dropped at Router %c (queue full).\n", routers[path[hop]].name);
            return;
        }
        printf("Packet forwarded to Router %c.\n", routers[path[hop]].name);
        Dequeue_Packet(path[hop]);
    }

    printf("\nTotal route cost: %d\n", distance[p.destination]);
    printf("Packet %d delivered.\n", p.id);
}

void Display_Queues() {
    int i, j;
    printf("\n--------- ROUTER QUEUES ---------\n");
    for(i = 0; i < Router_Count; i++) {
        printf("\nRouter %c: ", routers[i].name);
        if(routers[i].front > routers[i].rear) {
            printf("Empty");
        }
        else {
            for(j = routers[i].front;j <= routers[i].rear;j++) {
                printf("[P%d] ", routers[i].queue[j].id);
            }
        }
        printf("\n");
    }
}

/* Records this reading into the router's rolling history. */
void Record_History(int routerID, int count) {
    int slot = History_Next[routerID];
    Congestion_History[routerID][slot] = count;
    History_Next[routerID] = (slot + 1) % History_Length;
    if(History_Count[routerID] < History_Length) {
        History_Count[routerID]++;
    }
}

void Check_Congestion() {
    int count;
    printf("\n------- CONGESTION STATUS -------\n");
    for(int i = 0; i < Router_Count; i++) {
        count = routers[i].rear - routers[i].front + 1;
        if(count < 0)
            count = 0;

        Record_History(i, count);

        printf("Router %c : %d/%d packets -> ",routers[i].name,count,Queue_size);
        if(count >= Queue_size * 0.8)
            printf("CONGESTED\n");
        else if(count >= Queue_size * 0.5)
            printf("MODERATE\n");
        else
            printf("NORMAL\n");
    }
}

/* AI traffic prediction: sends every router's congestion history
   (oldest -> newest) to predict_congestion.py through a pipe.
   The Python script prints the predictions itself.
   Line format sent:  <name> <queue_size> <reading1> ... <readingN>  */
void Predict_Congestion() {
    if(Router_Count == 0) {
        printf("\nNo routers available.\n");
        return;
    }

    /* Flush our own output first so the Python output appears in order. */
    fflush(stdout);

    FILE *pipe = popen(PYTHON_CMD, "w");
    if(pipe == NULL) {
        printf("\nCould not start the Python AI module.\n");
        printf("Make sure Python is installed and predict_congestion.py\n");
        printf("is in the same folder as this program.\n");
        return;
    }

    for(int i = 0; i < Router_Count; i++) {
        int readings = History_Count[i];
        int oldestSlot = (History_Next[i] - readings + History_Length) % History_Length;

        fprintf(pipe, "%c %d", routers[i].name, Queue_size);
        for(int step = 0; step < readings; step++) {
            int slot = (oldestSlot + step) % History_Length;
            fprintf(pipe, " %d", Congestion_History[i][slot]);
        }
        fprintf(pipe, "\n");
    }

    int status = pclose(pipe);
    if(status != 0) {
        printf("\nPython AI module failed (exit code %d).\n", status);
        printf("Check that '%s' works from your terminal.\n", PYTHON_CMD);
    }
}

void Display_Packets() {
    if(Packet_Count == 0) {
        printf("\nNo packets created.\n");
        return;
    }
    printf("\n--------- PACKETS ---------\n");

    for(int i = 0; i < Packet_Count; i++) {
        printf("\nPacket ID    : %d", packetList[i].id);
        printf("\nSource       : %c", routers[packetList[i].source].name);
        printf("\nDestination  : %c", routers[packetList[i].destination].name);
        printf("\nSize         : %d bytes\n", packetList[i].size);
    }
}

int main() {
    int choice;
    Initialising_Network();
    while(1) {
        printf("\n\n====================================");
        printf("\n   NETWORK PACKET ROUTING SYSTEM");
        printf("\n====================================");
        printf("\n1. Create Router");
        printf("\n2. Add Network Connection");
        printf("\n3. Display Network");
        printf("\n4. Create Packet");
        printf("\n5. Display Packets");
        printf("\n6. Display Router Queues");
        printf("\n7. Route Packet");
        printf("\n8. Check Congestion");
        printf("\n9. Predict Congestion (Python AI)");
        printf("\n10. Exit");

        printf("\n\nEnter your choice: ");
        if(scanf("%d", &choice) != 1) {
            /* Non-numeric input: clear the buffer to avoid an infinite loop. */
            int c;
            while((c = getchar()) != '\n' && c != EOF);
            if(c == EOF) return 0;
            printf("\nInvalid choice.\n");
            continue;
        }

        switch(choice) {
            case 1:
                Creating_router();
                break;

            case 2:
                Add_Connection();
                break;

            case 3:
                Display_Network();
                break;

            case 4:
                Creating_Packet();
                break;

            case 5:
                Display_Packets();
                break;

            case 6:
                Display_Queues();
                break;

            case 7:
                Route_Packet();
                break;

            case 8:
                Check_Congestion();
                break;

            case 9:
                Predict_Congestion();
                break;

            case 10:
                printf("\nProgram ended.\n");
                return 0;

            default:
                printf("\nInvalid choice.\n");
        }
    }
    return 0;
}
