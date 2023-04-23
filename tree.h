#define TreeNode struct TreeNode
TreeNode
{
     int size, start, takenId, cursize;
     TreeNode *left;
     TreeNode *right;
};
void CreateNode(TreeNode **root, int lastSize, int start)
{
     if (*root == NULL)
     {
          *root = (TreeNode *)malloc(sizeof(TreeNode));
          (*root)->size = (*root)->cursize = lastSize;
          (*root)->start = start;
          (*root)->left = NULL;
          (*root)->right = NULL;
          (*root)->takenId = -1;
     }
}
int insert(TreeNode **root, int value, int PId, int lastSize, int start)
{
     // if the root is empty, create a new node to hold the value and make it the root
     if (value > (*root)->cursize)
          return 0;                                                 // can't insert under this node
     if (value > (*root)->size / 2 && value <= (*root)->cursize && (*root)->takenId == -1) // check the place and check that it can be taken
     {
          (*root)->takenId = PId;
          (*root)->cursize = 0;
          return (*root)->size;
     }
     else
     {
          //(*root)->takenId = -2;
          if ((*root)->left == NULL)
          {
               CreateNode(&((*root)->left), lastSize / 2, start);
               CreateNode(&((*root)->right), lastSize / 2, start + lastSize / 2);
          }
          int ret;
          ret = insert(&((*root)->left), value, PId, lastSize / 2, start);
          if (ret > 0)
          {
               (*root)->cursize -= ret;
               return ret;
          }
          ret = insert(&((*root)->right), value, PId, lastSize / 2, start + (lastSize / 2));
          if (ret > 0)
          {
               (*root)->cursize -= ret;
          }
          return ret;
     }
}

int deleteNode(TreeNode **root,int PId){
     if((*root) == NULL) return 0;
     if((*root)->takenId == PId){
          (*root)->takenId = -1;
          (*root)->cursize = (*root)->size;
          return (*root)->size;
     }
     int ret = 0;
     ret = deleteNode(&((*root)->left), PId);
     if(ret != 0){
          // check that the right are empty or not to delete them both
          if((*root)->right->cursize == (*root)->right->size){
               free((*root)->left);
               free((*root)->right);
               (*root)->left = (*root)->right = NULL;
          }
          (*root)->cursize += ret;
          return ret;
     }
     ret = deleteNode(&((*root)->right), PId);
     if(ret != 0){
          // check that the left are empty or not
          if((*root)->left->cursize == (*root)->left->size){
               free((*root)->left);
               free((*root)->right);
               (*root)->left = (*root)->right = NULL;
          }
          (*root)->cursize += ret;
          return ret;
     }
     return ret;
}

TreeNode* findNode(TreeNode **root, int PId){
     if((*root) == NULL) return 0;
     if((*root)->takenId == PId){
          return (*root);
     }
     TreeNode *left_pointer = findNode(&((*root)->left), PId);
     if(left_pointer != NULL) return left_pointer;
     TreeNode *right_pointer = findNode(&((*root)->right), PId);
     return right_pointer;
}



/*
         (-2 , 1024)
         /           \
     (5 , 512)         (-2 , 512)
        /   \         /        \
(5 , 256) (5 , 256) (-2, 265) (-1 , 256)
*/