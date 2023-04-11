#define Process struct Process 
Process
{
    int Id, Arrive_Time, Run_Time, Priority,Remaining_Time;
};
typedef struct node {
    Process data;
    int priority;
    struct node* next;
} Node;

Node* createNode(Process data, int priority) {
    Node* newNode = (Node*) malloc(sizeof(Node));
    newNode->data = data;
    newNode->priority = priority;
    newNode->next = NULL;
    return newNode;
}

void enqueue(Node** head, Process data, int priority) {
    Node* newNode = createNode(data, priority);
    if (*head == NULL) {
        *head = newNode;
        return;
    }
    if ((*head)->priority > priority) {
        newNode->next = *head;
        *head = newNode;
        return;
    }
    Node* temp = *head;
    while (temp->next != NULL && temp->next->priority <= priority) {
        temp = temp->next;
    }
    newNode->next = temp->next;
    temp->next = newNode;
}

Process dequeue(Node** head) {
    if (*head == NULL) {
        Process x;
        printf("Priority Queue is empty\n");
        return x;
    }
    Node* temp = *head;
    Process data = temp->data;
    *head = (*head)->next;
    //free(temp);
    return data;
}
int isEmpty(Node** head) {
   return (*head) == NULL;
}