#define Process struct Process 
Process
{
    int Id, Arrive_Time,
     Run_Time, Priority,Remaining_Time,
     Mem_Size;
};
typedef struct node {   
    Process data;
    int priority;
    struct node* next;
} Node;
int queue_size=0;
Node* createNode(Process data, int priority) {
    Node* newNode = (Node*) malloc(sizeof(Node));
    newNode->data = data;
    newNode->priority = priority;
    newNode->next = NULL;
    return newNode;
}

void enqueue(Node** head, Process data, int priority) {
    Node* newNode = createNode(data, priority);
    queue_size++;
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
    free(temp);
    queue_size--;
    return data;
}
Process Peek(Node**head){
      if (*head == NULL) {
        Process x;
       x.Id=-5;
        return x;
    }
    Node* temp = *head;
    Process data = temp->data;
    return data;
}
void printQueue(Node** head) {
    Node* temp = *head;
    while (temp != NULL) {
        printf("Process Id: %d, Priority: %d Remaining time:%d\n", temp->data.Id, temp->priority,temp->data.Remaining_Time);
        temp = temp->next;
    }
    printf("\n");
}
int isEmpty(Node** head) {
   return (*head) == NULL;
}